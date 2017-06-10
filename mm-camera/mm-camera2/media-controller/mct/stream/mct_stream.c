/* mct_stream.c
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mct_controller.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "mct_module.h"
#include "camera_dbg.h"
#include "cam_intf.h"
#include "server_debug.h"

#include <media/msmb_generic_buf_mgr.h>

#if 0
#undef CDBG
#define CDBG CDBG_ERROR
#endif

/* forward declare tuning APIs */
extern void mct_notify_metadata_frame(void *param);
extern void mct_notify_snapshot_triggered(void);

/** g_imglib_feature_mask:
 *
 *  Composite feature mask of the features supported by imglib
 *  Incase of SW WNR, HDR will be part of pproc instead of imglib
 **/
#ifndef CAMERA_FEATURE_WNR_SW
static uint32_t g_imglib_feature_mask =
  CAM_QCOM_FEATURE_REGISTER_FACE |
  CAM_QCOM_FEATURE_FACE_DETECTION |
  CAM_QCOM_FEATURE_HDR |
  CAM_QCOM_FEATURE_CHROMA_FLASH |
  CAM_QCOM_FEATURE_UBIFOCUS |
  CAM_QCOM_FEATURE_REFOCUS |
  CAM_QCOM_FEATURE_OPTIZOOM |
  CAM_QCOM_FEATURE_TRUEPORTRAIT |
  CAM_QCOM_FEATURE_FSSR |
  CAM_QCOM_FEATURE_MULTI_TOUCH_FOCUS;
#else
 static uint32_t g_imglib_feature_mask =
  CAM_QCOM_FEATURE_REGISTER_FACE |
  CAM_QCOM_FEATURE_FACE_DETECTION |
  CAM_QCOM_FEATURE_CHROMA_FLASH |
  CAM_QCOM_FEATURE_UBIFOCUS |
  CAM_QCOM_FEATURE_OPTIZOOM |
  CAM_QCOM_FEATURE_TRUEPORTRAIT |
  CAM_QCOM_FEATURE_FSSR |
  CAM_QCOM_FEATURE_MULTI_TOUCH_FOCUS;
#endif

/** mct_stream_check_module:
 *    @
 *    @
 *
 **/
static boolean mct_stream_check_module(void *d1, void *d2)
{
  return (!strcmp(MCT_OBJECT_NAME(d1), MCT_OBJECT_NAME(d2)) ? TRUE : FALSE);
}

/** mct_stream_check_name:
 *    @
 *    @
 *
 **/
static boolean mct_stream_check_name(void *mod, void *name)
{
  return ((!strcmp(MCT_OBJECT_NAME(mod), (char *)name)) ? TRUE : FALSE);
}

/** mct_stream_get_module:
 *    @mods: modules list
 *    @name: module name to retrive from modules list
 *
 *  Return corresponding module or NULL if not found in
 *  the list.
 **/
static mct_module_t *mct_stream_get_module(mct_list_t *mods, char *name)
{
  mct_list_t *module;

  module = mct_list_find_custom(mods, name, mct_stream_check_name);
  if (module)
    return (mct_module_t *)(module->data);

  return NULL;
}

/** mct_stream_operate_unlink:
 *    @d1: module1
 *    @d2: module2
 *    @user_data: MctStream_t stream
 *
 *  To unlink module1 and module2 on one stream
 **/
void mct_stream_operate_unlink(void *d1, void *d2,
  const void *user_data)
{
  mct_module_t *mod1   = (mct_module_t *)d1;
  mct_module_t *mod2   = (mct_module_t *)d2;
  mct_stream_t *stream = (mct_stream_t *)user_data;

  if (!strcmp(MCT_OBJECT_NAME(mod1), "stats")) {
    mod2 = mod1;
    mod1 = mct_stream_get_module(
      MCT_PIPELINE_MODULES(MCT_OBJECT_PARENT(stream)->data), "isp");
  } else if (!strcmp(MCT_OBJECT_NAME(mod2), "stats")) {
    mod1 = mct_stream_get_module(
      MCT_PIPELINE_MODULES(MCT_OBJECT_PARENT(stream)->data), "isp");
  }

  if (stream->unlink)
    stream->unlink(stream, mod1, mod2);
}

/** mct_stream_add_module:
 *    @
 *    @
 *
 **/
static boolean mct_stream_add_module(mct_stream_t *stream,
  mct_module_t *module)
{
  boolean is_sink, is_source;

  /* can't add ourself to ourself */
  if (&((MCT_PIPELINE_CAST(MCT_STREAM_PARENT(stream)))->module) == module)
    return FALSE;

  /* get the module name to make sure it is unique in this pipeline. */
  is_sink   = (mct_module_find_type(module, stream->streaminfo.identity))
                & MCT_MODULE_FLAG_SINK   ? 1 : 0;
  is_source = (mct_module_find_type(module, stream->streaminfo.identity))
                & MCT_MODULE_FLAG_SOURCE ? 1 : 0;

  /* check to see if the module's name is already existing in the stream */
  if (mct_object_check_uniqueness(MCT_OBJECT_CHILDREN(stream),
       (const char *)MCT_OBJECT_NAME(module)) != NULL)
    return TRUE;

  /* set the module's parent and add the module to the pipeline's list of children */
  if (mct_object_set_parent(MCT_OBJECT_CAST(module),
       MCT_OBJECT_CAST(stream)) == FALSE)
    return FALSE;

  return TRUE;
}

/** mct_stream_link_modules:
 *    @
 *    @
 *
 **/
boolean mct_stream_link_modules(mct_stream_t *stream,
  mct_module_t *mod1, mct_module_t *mod2, ...)
{
  va_list args;
  if (!mod1 || !mod2 || !stream)
    return FALSE;

  va_start(args, mod2);
  while(mod2) {
    if (mct_module_link((void *)(&(stream->streaminfo)), mod1, mod2) == TRUE) {
      if ((mct_stream_add_module(stream, mod1) == FALSE) ||
          (mct_stream_add_module(stream, mod2) == FALSE))
        goto error;
    } else {
      goto error;
    }

    mod1 = mod2;
    mod2 = va_arg(args, mct_module_t *);
  }
  va_end(args);

  return TRUE;

error:
  /*unlinking is done during stream destroy*/
  return FALSE;
}


static boolean mct_pproc_sink_port_caps_reserve(void *data1, void *data2)
{
  mct_port_t        *port = (mct_port_t *)data1;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data2;
  mct_port_caps_t    peer_caps;

  if (!port || !stream_info) {
    CDBG_ERROR("%s:%d] error port: %p stream_info: %p\n", __func__, __LINE__,
      port, stream_info);
    return FALSE;
  }

  if (port->peer != NULL) {
    CDBG_ERROR("%s:%d] The port %p is not free\n", __func__, __LINE__,port);
    return FALSE;
  }
  peer_caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  return port->check_caps_reserve(port, &peer_caps, stream_info);
}

/** mct_stream_start_link:
 *    @stream: stream object of mct_stream_t
 *
 * Analysis stream information and links corresponding
 * modules for this steam.
 *
 * Reture FASLE if the stream is not able to be linked.
 **/

static boolean mct_stream_start_link(mct_stream_t *stream)
{
  uint32_t sessionid;
  cam_stream_info_t    *stream_info;
  boolean              ret = FALSE;
  mct_stream_map_buf_t *info    = MCT_STREAM_STREAMINFO(stream);
  mct_list_t           *modules =
    MCT_PIPELINE_MODULES(MCT_OBJECT_PARENT(stream)->data);
  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);

  mct_module_t *sensor = NULL;
  mct_module_t *iface  = NULL;
  mct_module_t *isp    = NULL;
  mct_module_t *stats  = NULL;
  mct_module_t *pproc  = NULL;
  mct_module_t *imglib = NULL;
  mct_module_t *hdr = NULL;

  if (info == NULL)
    return FALSE;

  stream_info = (cam_stream_info_t *)info;

  sessionid = MCT_PIPELINE_SESSION(
    MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));

  stream->streaminfo.identity = pack_identity(sessionid, stream->streamid);
  stream->streaminfo.stream_type = stream_info->stream_type;
  stream->streaminfo.fmt = stream_info->fmt;
  stream->streaminfo.dim = stream_info->dim;
  stream->streaminfo.streaming_mode = stream_info->streaming_mode;
  stream->streaminfo.num_burst = stream_info->num_of_burst;
  stream->streaminfo.buf_planes = stream_info->buf_planes;
  stream->streaminfo.pp_config = stream_info->pp_config;
  stream->streaminfo.reprocess_config = stream_info->reprocess_config;
  stream->streaminfo.num_bufs = stream_info->num_bufs;
  stream->streaminfo.is_type = stream_info->is_type;
  /* TODO: temporary solution for now */
  stream->streaminfo.stream = stream;

  mct_pipeline_add_stream_to_linked_streams(pipeline, stream);

  switch (stream->streaminfo.stream_type) {
  case CAM_STREAM_TYPE_POSTVIEW: {

    CDBG_HIGH("%s: Starting postview stream linking \n", __func__);

    sensor = mct_stream_get_module(modules, "sensor");
    iface  = mct_stream_get_module(modules, "iface");
    isp    = mct_stream_get_module(modules, "isp");
    stats  = mct_stream_get_module(modules, "stats");
    pproc  = mct_stream_get_module(modules, "pproc");

    if (!sensor || !iface || !isp || !stats || !pproc) {
      CDBG_ERROR("%s:%d] Null: %p %p %p %p %p", __func__, __LINE__,
        sensor, iface, isp, stats, pproc);
      return FALSE;
    }
    sensor->set_mod(sensor, MCT_MODULE_FLAG_SOURCE,
      stream->streaminfo.identity);
    iface->set_mod(iface, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    isp->set_mod(isp, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    stats->set_mod(stats, MCT_MODULE_FLAG_SINK,
      stream->streaminfo.identity);
    pproc->set_mod(pproc, MCT_MODULE_FLAG_SINK,
      stream->streaminfo.identity);

    ret = mct_stream_link_modules(stream, sensor, iface, isp, pproc,
      NULL);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
      return FALSE;
    }
    if(pipeline->query_data.sensor_cap.sensor_format != FORMAT_YCBCR){
      ret = mct_stream_link_modules(stream, isp, stats, NULL);
      if (ret == FALSE) {
        CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
        return FALSE;
      }
    }
    ret = mct_pipeline_send_ctrl_events(pipeline, stream->streamid,
      MCT_EVENT_CONTROL_SET_PARM);
  }
    break;

  case CAM_STREAM_TYPE_PREVIEW: {

    CDBG_HIGH("%s: Starting preview/postview stream linking \n", __func__);

    sensor = mct_stream_get_module(modules, "sensor");
    iface  = mct_stream_get_module(modules, "iface");
    isp    = mct_stream_get_module(modules, "isp");
    stats  = mct_stream_get_module(modules, "stats");
    pproc  = mct_stream_get_module(modules, "pproc");
    imglib = mct_stream_get_module(modules, "imglib");

    if (!sensor || !iface || !isp || !stats || !pproc || !imglib) {
      CDBG_ERROR("%s:%d] Null: %p %p %p %p %p %p", __func__, __LINE__,
        sensor, iface, isp, stats, pproc, imglib);
      return FALSE;
    }
    sensor->set_mod(sensor, MCT_MODULE_FLAG_SOURCE,
      stream->streaminfo.identity);
    iface->set_mod(iface, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    isp->set_mod(isp, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    stats->set_mod(stats, MCT_MODULE_FLAG_SINK,
      stream->streaminfo.identity);
    pproc->set_mod(pproc, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    imglib->set_mod(imglib, MCT_MODULE_FLAG_SINK,
      stream->streaminfo.identity);

    ret = mct_stream_link_modules(stream, sensor, iface, isp, pproc, imglib,
    NULL);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
      return FALSE;
    }
    if(pipeline->query_data.sensor_cap.sensor_format != FORMAT_YCBCR){
      ret = mct_stream_link_modules(stream, isp, stats, NULL);
      if (ret == FALSE) {
      CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
      return FALSE;
      }
    }

    if (ret == FALSE) {
      CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
      return FALSE;
    }
    ret = mct_pipeline_send_ctrl_events(pipeline, stream->streamid,
          MCT_EVENT_CONTROL_SET_PARM);
  }
    break;

  case CAM_STREAM_TYPE_SNAPSHOT: {

    CDBG_HIGH("%s: Starting snapshot stream linking \n", __func__);

    mct_pipeline_t *pipeline =
      MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data);

    /* if snapshot has continuous mode it's ZSL */
    if (stream->streaminfo.streaming_mode ==
        CAM_STREAMING_MODE_CONTINUOUS) {
      /* this means this stream(list->data) has the same bundle_id
       * and it is ZSL snapshot stream, which doesn't need to link
       * pproc module */
      CDBG_HIGH("%s: ZSL snapshot", __func__);
      sensor = mct_stream_get_module(modules, "sensor");
      iface  = mct_stream_get_module(modules, "iface");
      isp    = mct_stream_get_module(modules, "isp");
      stats  = mct_stream_get_module(modules, "stats");
      if (!sensor || !iface || !isp || !stats) {
        CDBG_ERROR("%s:%d] Null: %p %p %p %p", __func__, __LINE__,
        sensor, iface, isp, stats);
        return FALSE;
      }
    } else {
      /* regular snapshot stream which needs to link pproc module */

      CDBG_HIGH("%s: Regular snapshot", __func__);
      sensor = mct_stream_get_module(modules, "sensor");
      iface  = mct_stream_get_module(modules, "iface");
      isp    = mct_stream_get_module(modules, "isp");
      stats  = mct_stream_get_module(modules, "stats");
      if (!sensor || !iface || !isp || !stats) {
        CDBG_ERROR("%s:%d] Null: %p %p %p %p", __func__, __LINE__,
          sensor, iface, isp, stats);
          return FALSE;
      }
      mct_notify_snapshot_triggered();
    }
    sensor->set_mod(sensor, MCT_MODULE_FLAG_SOURCE,
      stream->streaminfo.identity);
    iface->set_mod(iface, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    isp->set_mod(isp, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    stats->set_mod(stats, MCT_MODULE_FLAG_SINK,
      stream->streaminfo.identity);

    pproc  = NULL;
    /* If the sensor outputs YUV422i use C2D to convert it to NV21 */
    if((pipeline->query_data.sensor_cap.sensor_format == FORMAT_YCBCR) &&
       !pipeline->query_data.isp_cap.use_pix_for_SOC){
      pproc  = mct_stream_get_module(modules, "pproc");
    }
    if (pproc) {
      pproc->set_mod(pproc, MCT_MODULE_FLAG_SINK, stream->streaminfo.identity);
      ret = mct_stream_link_modules(stream, sensor, iface, isp, pproc, NULL);
      if (ret == FALSE)
        return FALSE;
      if(pipeline->query_data.sensor_cap.sensor_format != FORMAT_YCBCR){
      ret = mct_stream_link_modules(stream, isp, stats, NULL);
      if (ret == FALSE) {
      CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
      return FALSE;
      }
    }
    } else {
      if(pipeline->query_data.sensor_cap.sensor_format != FORMAT_YCBCR)
        ret = mct_stream_link_modules(stream, sensor, iface, isp, stats, NULL);
      else
        ret = mct_stream_link_modules(stream, sensor, iface, isp, NULL);
    }
  }
    break;

  case CAM_STREAM_TYPE_VIDEO: {
    sensor = mct_stream_get_module(modules, "sensor");
    iface  = mct_stream_get_module(modules, "iface");
    isp    = mct_stream_get_module(modules, "isp");
    stats  = mct_stream_get_module(modules, "stats");
    pproc  = mct_stream_get_module(modules, "pproc");

    if (!sensor || !iface || !isp || !stats || !pproc ) {
      CDBG_ERROR("%s:%d] Null: %p %p %p %p %p", __func__, __LINE__,
        sensor, iface, isp, stats, pproc);
      return FALSE;
    }
    sensor->set_mod(sensor, MCT_MODULE_FLAG_SOURCE,
      stream->streaminfo.identity);
    iface->set_mod(iface, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    isp->set_mod(isp, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    stats->set_mod(stats, MCT_MODULE_FLAG_SINK,
      stream->streaminfo.identity);

    pproc->set_mod(pproc, MCT_MODULE_FLAG_SINK,
      stream->streaminfo.identity);

    ret = mct_stream_link_modules(stream, sensor, iface, isp, pproc,
      NULL);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
      return FALSE;
    }
    if(pipeline->query_data.sensor_cap.sensor_format != FORMAT_YCBCR){
    ret = mct_stream_link_modules(stream, isp, stats, NULL);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
      return FALSE;
    }
    }

    if (ret == FALSE) {
      CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
      return FALSE;
    }
  }
    break;

  case CAM_STREAM_TYPE_RAW: {

    CDBG_HIGH("%s: Starting RAW stream linking \n", __func__);

    sensor = mct_stream_get_module(modules, "sensor");
    iface  = mct_stream_get_module(modules, "iface");
    isp    = mct_stream_get_module(modules, "isp");
    stats    = mct_stream_get_module(modules, "stats");

    if (!sensor || !iface || !isp || !stats) {
      CDBG_ERROR("%s:Null: %p %p %p %p", __func__, sensor, iface, isp, stats);
      return FALSE;
    }

    sensor->set_mod(sensor, MCT_MODULE_FLAG_SOURCE,
      stream->streaminfo.identity);
    iface->set_mod(iface, MCT_MODULE_FLAG_INDEXABLE,
      stream->streaminfo.identity);
    isp->set_mod(isp, MCT_MODULE_FLAG_SINK,
      stream->streaminfo.identity);

    ret = mct_stream_link_modules(stream, sensor, iface, isp, stats,  NULL);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d] link failed", __func__, __LINE__);
      return FALSE;
    }
    ret = mct_pipeline_send_ctrl_events(pipeline, stream->streamid,
          MCT_EVENT_CONTROL_SET_PARM);
  }
    break;

  case CAM_STREAM_TYPE_METADATA:
    CDBG_HIGH("%s: Starting meta stream linking: do nothing \n", __func__);

    CDBG("%s:CAM_STREAM_TYPE_METADATA", __func__);
    ret = TRUE;
    break;

  case CAM_STREAM_TYPE_OFFLINE_PROC: {
    mct_module_t *single_module = NULL;

    CDBG_HIGH("%s: Starting offline stream linking, 0x%x", __func__,
      stream->streaminfo.reprocess_config.pp_feature_config.feature_mask);

    /*Check if imglib module is needed*/
    if (stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & g_imglib_feature_mask) {
      imglib = mct_stream_get_module(modules, "imglib");

      if (!imglib) {
        CDBG_ERROR("%s:Null: imglib Module", __func__);
        return FALSE;
      }
      CDBG_HIGH("Imglib obtained in offline stream");
    }

    if (stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
      & CAM_QCOM_FEATURE_DENOISE2D
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
        & CAM_QCOM_FEATURE_SHARPNESS
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
        & CAM_QCOM_FEATURE_EFFECT
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
        & CAM_QCOM_FEATURE_ROTATION
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
        & CAM_QCOM_FEATURE_CROP
#ifdef CAMERA_FEATURE_WNR_SW
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
        & CAM_QCOM_FEATURE_HDR
#endif
      || stream->streaminfo.reprocess_config.pp_feature_config.feature_mask
        & CAM_QCOM_FEATURE_SCALE) {
      pproc = mct_stream_get_module(modules, "pproc");

      if (!pproc) {
        CDBG_ERROR("%s:Null: postproc Module", __func__);
        return FALSE;
      }
    }

    if(pproc) {
      mct_list_t  *lport = NULL;
      mct_port_t  *port = NULL;
      int32_t rc;

      lport = mct_list_find_custom(MCT_MODULE_SINKPORTS(pproc),
        &stream->streaminfo, mct_pproc_sink_port_caps_reserve);

      if (!lport) {
        CDBG_ERROR("%s:%d failed: to caps reserve\n", __func__, __LINE__);
        return FALSE;
      }

      /* Extract pproc port from mct list */
      port = (mct_port_t *)lport->data;
      if (!port) {
        CDBG_ERROR("%s:%d failed: reserved port NULL", __func__, __LINE__);
        return  FALSE;
      }

      /* Add identity in port's children */
      rc = mct_port_add_child(stream->streaminfo.identity, port);
      if (rc == FALSE) {
        CDBG_ERROR("%s:%d failed: to add child\n", __func__, __LINE__);
        return FALSE;
      }

    }

    if (imglib == NULL && pproc == NULL)
      return FALSE;

    /* Start linking modules
     * pproc (if present) will always be the source
     * If all three present connect 'pproc to imglib'  and 'prpoc to hdr'
     * only imglib and hdr is not a valid combination
     */

    if (pproc && imglib) {

      CDBG("%s: Linking pproc and hdr\n", __func__);

       pproc->set_mod(pproc, MCT_MODULE_FLAG_SOURCE,
         stream->streaminfo.identity);

       imglib->set_mod(imglib, MCT_MODULE_FLAG_SINK,
         stream->streaminfo.identity);

       ret = mct_stream_link_modules(stream, pproc, imglib, NULL);

    } else {
      /* We have only one module to the stream
       and we can not establish the link
       */
      CDBG("%s: Linking single module\n", __func__);
      if (imglib)
        single_module = imglib;
      else if (pproc)
        single_module = pproc;

      single_module->set_mod(single_module, MCT_MODULE_FLAG_SOURCE,
        stream->streaminfo.identity);

      /*add module to the stream*/
      MCT_STREAM_CHILDREN(stream) = mct_list_append(
        MCT_STREAM_CHILDREN(stream), single_module, NULL, NULL);
      if (!MCT_OBJECT_CHILDREN(stream)) {
        CDBG_ERROR("%s: Error configuring single module \n", __func__);
        return FALSE;
      }
      /* If module is added to the stream set his parent */MCT_MODULE_PARENT(single_module) =
        mct_list_append(MCT_MODULE_PARENT(single_module), stream, NULL, NULL);
      if (!MCT_MODULE_PARENT(single_module)) {
        CDBG_ERROR("%s: Couldn't append\n", __func__);
        return FALSE;
      }
    }
    break;

  }
  default:
    break;
  } /* switch (stream->streaminfo.stream_type) */

  return TRUE;
}

/** mct_stream_start_unlink:
 *    @stream:
 *    @module1:
 *    @module2:
 *
 **/
static void mct_stream_start_unlink(mct_stream_t *stream,
  mct_module_t *module1, mct_module_t *module2)
{
  CDBG("%s: Un-linking %s and %s\n", __func__,
    MCT_MODULE_NAME(module1), MCT_MODULE_NAME(module2));

  mct_module_unlink(stream->streaminfo.identity, module1, module2);
  mct_module_remove_type(module1, stream->streaminfo.identity);
  mct_module_remove_type(module2, stream->streaminfo.identity);

  return;
}

/** mct_stream_find_metadata_buf:
 *    @stream: mct_stream_t object which to receive the event
 *    @event:  mct_event_t object to send to this stream
 *
 *  Used for matching metadata buffer in to stream list.
 *
 *  Return TRUE on success
 **/
static boolean mct_stream_find_metadata_buf(void *data, void *user_data)
{
  boolean check_index;

  check_index = (((mct_stream_map_buf_t *)data)->buf_index ==
      *((uint32_t *)user_data));

  return ((check_index) ? TRUE : FALSE);
}

/** mct_stream_find_stream_buf:
 *    @stream: mct_stream_t object which to receive the event
 *    @event:  mct_event_t object to send to this stream
 *
 *  Used for matching image buffer in to stream list.
 *
 *  Return TRUE on success
 **/
static boolean mct_stream_find_stream_buf(void *data, void *user_data)
{
  boolean check_index;

  check_index = (((mct_stream_map_buf_t *)data)->buf_index ==
    ((mct_serv_ds_msg_t *)user_data)->index) &&
    (((mct_stream_map_buf_t *)data)->buf_type ==
    ((mct_serv_ds_msg_t *)user_data)->buf_type);

  return ((check_index) ? TRUE : FALSE);
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  This implementation is incomplete. Need to redo module linking
 **/
static boolean mct_stream_remove_module(mct_stream_t *stream,
  mct_module_t *module)
{
  if (MCT_STREAM_CAST((MCT_MODULE_PARENT(module))->data) != stream)
    return FALSE;

  MCT_STREAM_CHILDREN(stream) =
    mct_list_remove(MCT_STREAM_CHILDREN(stream), module);
  (MCT_STREAM_NUM_CHILDREN(stream))--;

  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_stream_metabuf_find_bfr_mngr_subdev(int *buf_mgr_fd)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  char subdev_name[32];
  int32_t dev_fd = 0, ioctl_ret;
  boolean ret = FALSE;
  uint32_t i = 0;

  while (1) {
    int32_t num_entities = 1;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd >= MAX_FD_PER_PROCESS) {
      dump_list_of_daemon_fd();
      dev_fd = -1;
      break;
    }
    if (dev_fd < 0) {
      CDBG_ERROR("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      break;
    }
    num_media_devices++;
    ioctl_ret = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (ioctl_ret < 0) {
      CDBG_ERROR("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      close(dev_fd);
      break;
    }
    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model) != 0)) {
      close(dev_fd);
      continue;
    }
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      CDBG("%s:%d entity id %d", __func__, __LINE__, entity.id);
      ioctl_ret = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (ioctl_ret < 0) {
        CDBG_ERROR("%s:%d Done enumerating media entities\n", __func__, __LINE__);
        ret = FALSE;
        close(dev_fd);
        break;
      }
      CDBG("%s:%d entity name %s type %d group id %d\n", __func__, __LINE__,
        entity.name, entity.type, entity.group_id);

      CDBG("%s:group_id=%d", __func__, entity.group_id);

      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_BUF_MNGR) {
        snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);

        CDBG("%s:subdev_name=%s", __func__, subdev_name);

        *buf_mgr_fd = open(subdev_name, O_RDWR);
        CDBG("%s: *buf_mgr_fd=%d\n", __func__, *buf_mgr_fd);
      if ((*buf_mgr_fd) >= MAX_FD_PER_PROCESS) {
        dump_list_of_daemon_fd();
        *buf_mgr_fd = -1;
        continue;
    }
        if (*buf_mgr_fd < 0) {
          CDBG_ERROR("%s: Open subdev failed\n", __func__);
          continue;
        }
        ret = TRUE;
        CDBG("%s:%d:ret=%d\n", __func__, __LINE__, ret);
        close(dev_fd);
        return ret;
      }
    }
    close(dev_fd);
  }
  CDBG("%s:%d:ret=%d\n", __func__, __LINE__, ret);
  return ret;
}

/** add_metadata_entry:
 *    @
 *    @
 *
 **/
static void add_metadata_entry(int meta_type, uint32_t meta_length,
  void *meta_value, metadata_buffer_t *m_table)
{
  int position = meta_type;
  int current, next;

  if (position >= CAM_INTF_PARM_MAX) {
    CDBG_ERROR("%s: position %d out of bound [0, %d]",
      __func__, position, CAM_INTF_PARM_MAX-1);
    return;
  }

  current = GET_FIRST_PARAM_ID(m_table);
  if (position == current){
    //DO NOTHING
  } else if (position < current){
    SET_NEXT_PARAM_ID(position, m_table, current);
    SET_FIRST_PARAM_ID(m_table, position);
  } else {
    /* Search for the position in the linked list where we need to slot in*/
    while (position > GET_NEXT_PARAM_ID(current, m_table))
      current = GET_NEXT_PARAM_ID(current, m_table);

    /*If node already exists no need to alter linking*/
    if (position != GET_NEXT_PARAM_ID(current, m_table)) {
      next = GET_NEXT_PARAM_ID(current, m_table);
      SET_NEXT_PARAM_ID(current, m_table, position);
      SET_NEXT_PARAM_ID(position, m_table, next);
    }
  }

  if (meta_length > sizeof(metadata_type_t)) {
    CDBG_ERROR("%s:Size of input larger than max entry size",__func__);
  }
  memcpy(POINTER_OF(meta_type, m_table), meta_value, meta_length);

  return;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_stream_send_event_stream_on(void *data, void *user_data)
{
  boolean ret = TRUE;
  mct_stream_t *stream = (mct_stream_t *)data;
  mct_event_control_t event_data;
  mct_event_t cmd_event;

  event_data.type = MCT_EVENT_CONTROL_STREAMON;
  event_data.control_event_data = (void *)&stream->streaminfo;
  CDBG_HIGH("%s: stream_type = %d stream state = %d\n",
            __func__, stream->streaminfo.stream_type, stream->state);

  if (MCT_ST_STATE_PENDING_RESTART == stream->state) {
    event_data.type = MCT_EVENT_CONTROL_STREAMON;
    event_data.control_event_data = (void *)&stream->streaminfo;

    CDBG("SSS %s buff list %p\n", __func__, stream->streaminfo.img_buffer_list);

    cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(
        MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data)), stream->streamid)),
        MCT_EVENT_DOWNSTREAM, &event_data);

    ret = stream->send_event(stream, &cmd_event);
    stream->state = MCT_ST_STATE_RUNNING;
  } else {
    CDBG_ERROR("%s: Skip\n", __func__);
  }

  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_stream_send_event_stream_off(void *data, void *user_data)
{
  boolean ret = TRUE;
  mct_stream_t *stream = (mct_stream_t *)data;
  mct_event_control_t event_data;
  mct_event_t cmd_event;

  event_data.type = MCT_EVENT_CONTROL_STREAMOFF;
  event_data.control_event_data = (void *)&stream->streaminfo;
  CDBG_HIGH("%s: stream_type = %d stream state = %d\n",
          __func__, stream->streaminfo.stream_type, stream->state);

  if (MCT_ST_STATE_RUNNING == stream->state) {
    event_data.type = MCT_EVENT_CONTROL_STREAMOFF;
    event_data.control_event_data = (void *)&stream->streaminfo;

    cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(
        MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data)), stream->streamid)),
        MCT_EVENT_DOWNSTREAM, &event_data);

    CDBG_HIGH("%s: stream_type = %d\n", __func__, stream->streaminfo.stream_type);

    ret = stream->send_event(stream, &cmd_event);
    stream->state = MCT_ST_STATE_PENDING_RESTART;
  } else {
    CDBG_ERROR("%s: Skip\n", __func__);
  }

  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_handle_no_vfe_resources_msg(mct_stream_t *metadata_stream)
{
  boolean ret = TRUE;
  mct_event_t cmd_event;
  mct_event_control_t event_data;
  mct_event_control_parm_t event_parm;
  mct_pipeline_t *pipeline =
    MCT_PIPELINE_CAST((MCT_STREAM_PARENT(metadata_stream))->data);

  /* send stream Off event */
  ret = mct_list_traverse(MCT_PIPELINE_CHILDREN(pipeline),
    mct_stream_send_event_stream_off, NULL);

  if (!ret) {
    CDBG_ERROR("%s:stream off event failed.", __func__);
    return ret;
  }

  /* send stream On event */
  ret = mct_list_traverse(MCT_PIPELINE_CHILDREN(pipeline),
    mct_stream_send_event_stream_on, NULL);

  if (!ret) {
    CDBG_ERROR("%s:stream off event failed.", __func__);
    return ret;
  }

  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_handle_error_msg(mct_stream_t *stream, void *msg)
{
    boolean ret = TRUE;
    mct_bus_msg_error_message_t *err_msg =
      (mct_bus_msg_error_message_t *)(msg);
    switch (*err_msg) {
      case MCT_ERROR_MSG_RSTART_VFE_STREAMING:
      case MCT_ERROR_MSG_RESUME_VFE_STREAMING: {
        ret = mct_handle_no_vfe_resources_msg(stream);
      }
      break;

      default:
        CDBG_ERROR("%s:Unsupported message type", __func__);
        ret = FALSE;
        break;
    }
    return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static void mct_stream_fill_metadata_v1(cam_metadata_info_t *pdst,
  void *psrc, mct_bus_msg_type_t type, unsigned int size,
  mct_stream_session_metadata_info *local)
{
  if (!pdst || !psrc) {
    CDBG_ERROR("%s:%d buf is null", __func__, __LINE__);
    return;
  }

  switch (type) {
  case MCT_BUS_MSG_Q3A_AF_STATUS:
  case MCT_BUS_MSG_SENSOR_AF_STATUS: {
    mct_bus_msg_af_status_t *af_msg = (mct_bus_msg_af_status_t *)psrc;

    pdst->focus_data.focus_dist = af_msg->f_distance;
    pdst->focus_data.focus_state = af_msg->focus_state;
    pdst->focus_data.focus_pos = af_msg->focus_pos;
    pdst->is_focus_valid = TRUE;
    }
    break;

  case MCT_BUS_MSG_UPDATE_AF_FOCUS_POS:{
    cam_focus_pos_info_t *af_msg = (cam_focus_pos_info_t *)psrc;
    pdst->cur_pos_info.diopter = af_msg->diopter;
    pdst->cur_pos_info.scale = af_msg->scale;
    pdst->is_focus_pos_info_valid = TRUE;
    }
    break;

  case MCT_BUS_MSG_ASD_HDR_SCENE_STATUS: {
    mct_bus_msg_asd_hdr_status_t *asd_msg = (mct_bus_msg_asd_hdr_status_t *)psrc;
    pdst->hdr_scene_data.is_hdr_scene = asd_msg->is_hdr_scene;
    pdst->hdr_scene_data.hdr_confidence = asd_msg->hdr_confidence;
    pdst->is_hdr_scene_data_valid = TRUE;
    }
    break;

  case MCT_BUS_MSG_FACE_INFO:
    pdst->is_faces_valid = TRUE;
    memcpy(&pdst->faces_data, psrc,
      sizeof(cam_face_detection_data_t));
    break;

  case MCT_BUS_MSG_HIST_STATS_INFO:
    memcpy(&pdst->stats_data, psrc, sizeof(cam_hist_stats_t));
    pdst->is_stats_valid = TRUE;
    break;

  case MCT_BUS_MSG_PREPARE_HW_DONE:
    memcpy(&pdst->prep_snapshot_done_state, psrc,
      sizeof(cam_prep_snapshot_state_t));
    pdst->is_prep_snapshot_done_valid = TRUE;
    break;

  case MCT_BUS_MSG_PREPARE_HDR_ZSL_DONE:
    memcpy(&pdst->prep_snapshot_done_state, psrc,
      sizeof(cam_prep_snapshot_state_t));
    pdst->is_prep_snapshot_done_valid = TRUE;
    break;

  case MCT_BUS_MSG_ZSL_TAKE_PICT_DONE:
    memcpy(&pdst->good_frame_idx_range, psrc,
      sizeof(cam_frame_idx_range_t));
    pdst->is_good_frame_idx_range_valid = TRUE;
    break;

  case MCT_BUS_MSG_ISP_STREAM_CROP: {
    mct_bus_msg_stream_crop_t *crop_msg = (mct_bus_msg_stream_crop_t *)psrc;
    pdst->is_crop_valid = TRUE;
    pdst->crop_data.crop_info[pdst->crop_data.num_of_streams]
      .stream_id = crop_msg->stream_id;
    pdst->crop_data.crop_info[pdst->crop_data.num_of_streams]
      .crop.left = (int32_t)crop_msg->x;
    pdst->crop_data.crop_info[pdst->crop_data.num_of_streams]
      .crop.top =  (int32_t)crop_msg->y;
    pdst->crop_data.crop_info[pdst->crop_data.num_of_streams]
      .crop.width = (int32_t)crop_msg->crop_out_x;
    pdst->crop_data.crop_info[pdst->crop_data.num_of_streams]
      .crop.height = (int32_t)crop_msg->crop_out_y;
    pdst->crop_data.num_of_streams++;
    }
    break;

  case MCT_BUS_MSG_SET_SENSOR_INFO:
    memcpy(&(local->sensor_data), psrc, sizeof(mct_bus_msg_sensor_metadata_t));
    break;

  case MCT_BUS_MSG_SET_STATS_AEC_INFO:
    if (sizeof(local->stats_aec_data) >= size) {
      memcpy(&(local->stats_aec_data), psrc, size);
    }
    break;

  case MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO:
    if (sizeof(local->isp_stats_awb_data) >= size) {
      memcpy(&(local->isp_stats_awb_data), psrc, size);
    }
    break;

  case MCT_BUS_MSG_SET_ISP_GAMMA_INFO:
    if (sizeof(local->isp_gamma_data) >= size) {
      memcpy(&(local->isp_gamma_data), psrc, size);
    }
    break;

  case MCT_BUS_MSG_ISP_SOF:
    break;

  case MCT_BUS_MSG_META_VALID: {
    mct_bus_msg_meta_valid *ptr = psrc;
    }
    break;
  case MCT_BUS_MSG_AE_INFO:
    pdst->is_ae_params_valid = TRUE;
    memcpy(&pdst->ae_params, psrc, sizeof(cam_ae_params_t));
    break;
  case MCT_BUS_MSG_AWB_INFO:
    pdst->is_awb_params_valid = TRUE;
    memcpy(&pdst->awb_params, psrc, sizeof(cam_awb_params_t));
    break;
  case MCT_BUS_MSG_AE_EXIF_DEBUG_INFO:
    pdst->is_ae_exif_debug_valid = TRUE;
    memcpy(&pdst->ae_exif_debug_params, psrc, sizeof(cam_ae_exif_debug_t));
    break;
  case MCT_BUS_MSG_AWB_EXIF_DEBUG_INFO:
    pdst->is_awb_exif_debug_valid = TRUE;
    memcpy(&pdst->awb_exif_debug_params, psrc, sizeof(cam_awb_exif_debug_t));
    break;
  case MCT_BUS_MSG_AF_EXIF_DEBUG_INFO:
    pdst->is_af_exif_debug_valid = TRUE;
    memcpy(&pdst->af_exif_debug_params, psrc, sizeof(cam_af_exif_debug_t));
    break;
  case MCT_BUS_MSG_ASD_EXIF_DEBUG_INFO:
    pdst->is_asd_exif_debug_valid = TRUE;
    memcpy(&pdst->asd_exif_debug_params, psrc, sizeof(cam_asd_exif_debug_t));
    break;
  case MCT_BUS_MSG_STATS_EXIF_DEBUG_INFO:
    pdst->is_stats_buffer_exif_debug_valid = TRUE;
    memcpy(&pdst->stats_buffer_exif_debug_params, psrc,
      sizeof(cam_stats_buffer_exif_debug_t));
    break;
  case MCT_BUS_MSG_SENSOR_INFO:
    pdst->is_sensor_params_valid = TRUE;
    memcpy(&pdst->sensor_params,  psrc,  sizeof(cam_sensor_params_t));
    break;
  case MCT_BUS_MSG_AUTO_SCENE_DECISION: {
    mct_bus_msg_asd_decision_t * asd_msg = (mct_bus_msg_asd_decision_t *)psrc;
    pdst->is_asd_decision_valid = TRUE;
    pdst->scene = asd_msg->scene;
    }
    break;
  case MCT_BUS_MSG_FRAME_INVALID:
    pdst->is_preview_frame_skip_valid = 1;
    memcpy(&pdst->preview_frame_skip_idx_range, psrc,
      sizeof(cam_frame_idx_range_t));
    CDBG("mct_stream: %d %d",
      pdst->preview_frame_skip_idx_range.min_frame_idx,
      pdst->preview_frame_skip_idx_range.max_frame_idx);
    break;
  case MCT_BUS_MSG_AE_EZTUNING_INFO:
    pdst->is_chromatix_lite_ae_stats_valid = TRUE;
    memcpy(&pdst->chromatix_lite_ae_stats_data, psrc, size);
    break;

  case MCT_BUS_MSG_AWB_EZTUNING_INFO:
    pdst->is_chromatix_lite_awb_stats_valid= TRUE;
    memcpy(&pdst->chromatix_lite_awb_stats_data, psrc, size);
    break;

  case MCT_BUS_MSG_AF_EZTUNING_INFO:
    pdst->is_chromatix_lite_af_stats_valid= TRUE;
    memcpy(&pdst->chromatix_lite_af_stats_data, psrc, size);
    break;


  case MCT_BUS_MSG_AF_MOBICAT_INFO:
    pdst->is_chromatix_mobicat_af_valid= TRUE;
    memcpy(&pdst->chromatix_mobicat_af_data, psrc, size);
    break;

  case MCT_BUS_MSG_ISP_CHROMATIX_LITE:
    pdst->is_chromatix_lite_isp_valid = TRUE;
    memcpy(&pdst->chromatix_lite_isp_data, psrc, size);
    break;

  case MCT_BUS_MSG_PP_CHROMATIX_LITE:
    pdst->is_chromatix_lite_pp_valid = TRUE;
    memcpy(&pdst->chromatix_lite_pp_data, psrc, size);
    break;

  case MCT_BUS_MSG_ISP_META: {
     isp_meta_t *isp_meta = (isp_meta_t *)psrc;
     int i;
     pdst->tuning_params.tuning_vfe_data_size = 0;
     memcpy(&pdst->tuning_params.data[TUNING_VFE_DATA_OFFSET +
       pdst->tuning_params.tuning_vfe_data_size],
       isp_meta,
       (sizeof(isp_meta_t) - (sizeof(isp_meta_entry_t) * ISP_META_MAX)));
     pdst->tuning_params.tuning_vfe_data_size +=
       (sizeof(isp_meta_t) - (sizeof(isp_meta_entry_t) * ISP_META_MAX));

     for (i = 0; i < isp_meta->num_entry; i++) {
       memcpy(&pdst->tuning_params.data[TUNING_VFE_DATA_OFFSET
         + pdst->tuning_params.tuning_vfe_data_size],
         &isp_meta->meta_entry[i], (sizeof(isp_meta->meta_entry[i].dump_type) +
         sizeof(isp_meta->meta_entry[i].len)
         + sizeof(isp_meta->meta_entry[i].start_addr)));
       pdst->tuning_params.tuning_vfe_data_size +=
         (sizeof(isp_meta->meta_entry[i].dump_type)
         + sizeof(isp_meta->meta_entry[i].len)
         + sizeof(isp_meta->meta_entry[i].start_addr));

       memcpy(&pdst->tuning_params.data[TUNING_VFE_DATA_OFFSET
         + pdst->tuning_params.tuning_vfe_data_size],
         isp_meta->meta_entry[i].isp_meta_dump, isp_meta->meta_entry[i].len);
       pdst->tuning_params.tuning_vfe_data_size += isp_meta->meta_entry[i].len;
     }
     break;
  }

  case MCT_BUS_MSG_SENSOR_META:
    pdst->tuning_params.tuning_sensor_data_size = size;
    memcpy(&pdst->tuning_params.data[TUNING_SENSOR_DATA_OFFSET],
      psrc, size);
    break;

  case MCT_BUS_MSG_SET_AF_MODE:
  case MCT_BUS_MSG_SET_AF_STATE:
  case MCT_BUS_MSG_SET_AEC_STATE:
  case MCT_BUS_MSG_SET_AEC_PRECAPTURE_ID:
  case MCT_BUS_MSG_SET_AF_TRIGGER_ID:
  case MCT_BUS_MSG_SET_AF_ROI:
      /* Handle this messages for compatibility with HAL3. */
      break;
  case MCT_BUS_MSG_PP_SET_META: {
    mct_bus_msg_meta_valid *meta_valid = (mct_bus_msg_meta_valid *)psrc;
    pdst->is_meta_invalid = 1;
    pdst->meta_invalid_params.meta_frame_id = meta_valid->frame_id;
    break;
  }
  case MCT_BUS_MSG_WM_BUS_OVERFLOW_RECOVERY:
    pdst->is_frame_id_reset = 1;
    break;
  default:
     CDBG_ERROR("%s:%d Unsupported message type msg_type %d\n",
       __func__, __LINE__,type);
     /* fall through */

  }
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static void mct_stream_fill_metadata_v3(metadata_buffer_t *pdst,
  void *psrc, mct_bus_msg_type_t type, unsigned int size,
  mct_stream_session_metadata_info *local)
{

  if (!pdst || !psrc) {
    CDBG_ERROR("%s:%d buf is null", __func__, __LINE__);
    return;
  }
  uint8_t valid_flag = 0;
  switch (type) {
  case MCT_BUS_MSG_Q3A_AF_STATUS: {
    mct_bus_msg_af_status_t *af_msg = (mct_bus_msg_af_status_t *)psrc;
    cam_auto_focus_data_t focus_data;
    focus_data.focus_dist = af_msg->f_distance;
    focus_data.focus_state = af_msg->focus_state;
    focus_data.focus_pos = af_msg->focus_pos;
    valid_flag = 1;
    add_metadata_entry(CAM_INTF_META_AUTOFOCUS_DATA,
      sizeof(cam_auto_focus_data_t), &focus_data, pdst);
    }
    break;

  case MCT_BUS_MSG_ASD_HDR_SCENE_STATUS: {
    mct_bus_msg_asd_hdr_status_t *asd_msg = (mct_bus_msg_asd_hdr_status_t *)psrc;
    cam_asd_hdr_scene_data_t asd_hdr_scene_data;
    asd_hdr_scene_data.is_hdr_scene = asd_msg->is_hdr_scene;
    asd_hdr_scene_data.hdr_confidence = asd_msg->hdr_confidence;
    add_metadata_entry(CAM_INTF_META_ASD_HDR_SCENE_DATA,
      sizeof(cam_asd_hdr_scene_data_t), &asd_hdr_scene_data, pdst);
    }
    break;

  case MCT_BUS_MSG_FACE_INFO:
    add_metadata_entry(CAM_INTF_META_FACE_DETECTION,
      sizeof(cam_face_detection_data_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_HIST_STATS_INFO:
    add_metadata_entry(CAM_INTF_META_HISTOGRAM,
      sizeof(cam_hist_stats_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_PREPARE_HW_DONE:
    add_metadata_entry(CAM_INTF_META_PREP_SNAPSHOT_DONE,
      sizeof(uint32_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_ZSL_TAKE_PICT_DONE:
    add_metadata_entry(CAM_INTF_META_GOOD_FRAME_IDX_RANGE,
      sizeof(cam_frame_idx_range_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_ISP_STREAM_CROP: {
    mct_bus_msg_stream_crop_t *crop_msg = (mct_bus_msg_stream_crop_t *)psrc;
    cam_crop_data_t crop_data, *temp_crop_data;
    uint8_t num_of_streams = 0;
    uint8_t current = GET_FIRST_PARAM_ID(pdst);
    while (current < CAM_INTF_PARM_MAX) {
      if (current == CAM_INTF_META_CROP_DATA) {
        temp_crop_data = (cam_crop_data_t *)(POINTER_OF(current, pdst));
        num_of_streams = temp_crop_data->num_of_streams;
        break;
      }
      current = GET_NEXT_PARAM_ID(current, pdst);
    }
    crop_data.crop_info[num_of_streams].stream_id = crop_msg->stream_id;
    crop_data.crop_info[num_of_streams].crop.left = (int32_t)crop_msg->x;
    crop_data.crop_info[num_of_streams].crop.top = (int32_t)crop_msg->y;
    crop_data.crop_info[num_of_streams].crop.width = (int32_t)crop_msg->crop_out_x;
    crop_data.crop_info[num_of_streams].crop.height = (int32_t)crop_msg->crop_out_y;
    num_of_streams++;
    crop_data.num_of_streams = num_of_streams;
    add_metadata_entry(CAM_INTF_META_CROP_DATA,
      sizeof(cam_crop_data_t), &crop_data, pdst);
    }
    break;

  case MCT_BUS_MSG_SET_SENSOR_INFO:
    memcpy(&(local->sensor_data), psrc, sizeof(mct_bus_msg_sensor_metadata_t));
    break;

  case MCT_BUS_MSG_SET_STATS_AEC_INFO:
    if (sizeof(local->stats_aec_data) >= size) {
      memcpy(&(local->stats_aec_data), psrc, size);
    }
    break;

  case MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO:
    if (sizeof(local->isp_stats_awb_data) >= size) {
      memcpy(&(local->isp_stats_awb_data), psrc, size);
    }
    break;

  case MCT_BUS_MSG_SET_AEC_STATE:
    add_metadata_entry(CAM_INTF_META_AEC_STATE,
      sizeof(int32_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_AEC_PRECAPTURE_ID:
    add_metadata_entry(CAM_INTF_META_AEC_STATE,
      sizeof(int32_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_SET_AF_STATE:
    add_metadata_entry(CAM_INTF_META_AF_STATE,
      sizeof(int32_t), psrc, pdst);
    break;
  case MCT_BUS_MSG_SET_AF_TRIGGER_ID:
    add_metadata_entry(CAM_INTF_META_AF_TRIGGER_ID,
      sizeof(int32_t), psrc, pdst);
    break;

  case MCT_BUS_MSG_ISP_SOF:
  case MCT_BUS_MSG_ERROR_MESSAGE:
    break;

  default:
     CDBG_ERROR("%s:%d Unsupported message type msg_type %d\n",
       __func__, __LINE__,type);
     /* fall through */

  }
}

/** mct_stream_find_frame_num_id_combo:
 *
 **/
static boolean mct_stream_find_frame_num_id_combo(void *data, void *user_data)
{
  boolean check_index;

  check_index = (((mct_stream_frame_num_idx_map_t *)data)->frame_index ==
      *((unsigned int *)user_data));

  return ((check_index) ? TRUE : FALSE);
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
unsigned int mct_stream_get_frame_number(mct_stream_t *stream,
  unsigned int frame_id, unsigned int *frame_number)
{
  mct_list_t *holder;
  mct_stream_frame_num_idx_map_t *data;
  int frame_number_found = 0;
  unsigned int frame_skip_count;
  unsigned int checking_frame_id;
  mct_pipeline_t *pipeline;
  pipeline = (MCT_PIPELINE_CAST(MCT_STREAM_PARENT(stream)->data));
  frame_skip_count = pipeline->max_pipeline_frame_delay;

  checking_frame_id = frame_id - frame_skip_count;
  do {
    holder = mct_list_find_custom(stream->frame_num_idx_list,
      &checking_frame_id, mct_stream_find_frame_num_id_combo);
    if (!holder || !holder->data) {
      CDBG("%s: No corresponding frame num found\n", __func__);
      break;
    }
    data = (mct_stream_frame_num_idx_map_t *)holder->data;
    if (!frame_number_found) {
      *frame_number = data->frame_number;
      frame_number_found = 1;
    } else {
      if (data->frame_number > *frame_number)
        *frame_number = data->frame_number;
    }
    stream->frame_num_idx_list =
      mct_list_remove(stream->frame_num_idx_list, data);
    free(data);
  } while (1);
  return frame_number_found;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
boolean mct_stream_metadata_bus_msg(mct_stream_t *stream,
  mct_bus_msg_t *bus_msg)
{
  mct_list_t *current_buf_holder = NULL, *new_buf_holder = NULL;
  mct_stream_map_buf_t *buf;
  cam_hal_version_t hal_version;
  struct msm_buf_mngr_info buf_info, get_buf;
  int ret = TRUE;
  unsigned int frame_number;
  int flag = 0;
  cam_metadata_info_t *current_buf_v1 = NULL;
  metadata_buffer_t *current_buf_v3 = NULL;
  mct_pipeline_t *pipeline;
  mct_stream_session_metadata_info *session_meta;
  mct_bus_msg_isp_sof_t *isp_sof_bus_msg = bus_msg->msg;
  mct_bus_msg_meta_valid *meta_valid_bus_msg = NULL;

  pipeline = (MCT_PIPELINE_CAST(MCT_STREAM_PARENT(stream)->data));
  session_meta = &stream->metadata_stream.session_meta;
  hal_version = pipeline->hal_version;

  if (MCT_BUS_MSG_ISP_SOF == bus_msg->type) {
    if (MCT_BUS_MSG_ISP_SOF == pipeline->bus->msg_to_send_metadata) {
      session_meta->frame_idx = isp_sof_bus_msg->frame_id - 1;
    } else {
      session_meta->frame_idx = isp_sof_bus_msg->frame_id;
    }
    session_meta->timestamp = isp_sof_bus_msg->timestamp;
  }

  if (MCT_BUS_MSG_META_VALID == bus_msg->type) {
    meta_valid_bus_msg = (mct_bus_msg_meta_valid *)bus_msg->msg;
    session_meta->frame_idx = meta_valid_bus_msg->frame_id;
  }

  if (!stream->metadata_stream.get_buf_err) {
    current_buf_holder = mct_list_find_custom(
      stream->buffers.img_buf,
      &(stream->metadata_stream.current_buf_idx),
      mct_stream_find_metadata_buf);
    if (!current_buf_holder) {
      CDBG_ERROR("%s:current_buf_holder is null", __func__);
      return FALSE;
    }

    buf = current_buf_holder->data;
    if (!buf) {
      CDBG_ERROR("%s:buf is null", __func__);
      return FALSE;
    }
    current_buf_v1 = (cam_metadata_info_t *)(buf->buf_planes[0].buf);
    current_buf_v3 = (metadata_buffer_t *)(buf->buf_planes[0].buf);
    if ((current_buf_v1 == NULL) || (current_buf_v3 == NULL)) {
      return FALSE;
    }
  }

  if (!stream->metadata_stream.get_buf_err) {
    /* fill correspondig part of metadata in output buffer */
    if (hal_version == CAM_HAL_V1) {
      mct_stream_fill_metadata_v1(current_buf_v1, bus_msg->msg, bus_msg->type,
        bus_msg->size, &stream->metadata_stream.session_meta);
    } else {
      mct_stream_fill_metadata_v3(current_buf_v3, bus_msg->msg, bus_msg->type,
        bus_msg->size, &stream->metadata_stream.session_meta);
    }
  }

  if (MCT_BUS_MSG_ERROR_MESSAGE == bus_msg->type) {
    mct_handle_error_msg(stream, bus_msg->msg);
  }
  if ((pipeline->bus->msg_to_send_metadata == bus_msg->type)
    && (session_meta->frame_idx)) {

    if (!stream->metadata_stream.get_buf_err) {
      mct_bus_msg_isp_sof_t *isp_sof_bus_msg = bus_msg->msg;
      buf_info.index = stream->metadata_stream.current_buf_idx;
      buf_info.frame_id = session_meta->frame_idx;
      buf_info.stream_id = stream->streamid;
      buf_info.timestamp = session_meta->timestamp;
      buf_info.session_id = MCT_PIPELINE_SESSION(
              MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));

      if (hal_version == CAM_HAL_V1) {
        if (sizeof(current_buf_v1->private_metadata) >=
        sizeof(mct_stream_session_metadata_info))
          memcpy(current_buf_v1->private_metadata,
            &(stream->metadata_stream.session_meta),
            sizeof(mct_stream_session_metadata_info));
        else
          CDBG("%s: Private metadata not of sufficient size\n", __func__);
        if ((current_buf_v1->tuning_params.tuning_sensor_data_size > 0) &&
            (current_buf_v1->tuning_params.tuning_vfe_data_size > 0)) {
          current_buf_v1->is_tuning_params_valid = 1;
        }
      } else if (hal_version == CAM_HAL_V3) {
        add_metadata_entry(CAM_INTF_META_PRIVATE_DATA,
          sizeof(mct_stream_session_metadata_info),
          &(stream->metadata_stream.session_meta), current_buf_v3);
        flag = mct_stream_get_frame_number(stream,
          isp_sof_bus_msg->frame_id, &frame_number);
        add_metadata_entry(CAM_INTF_META_FRAME_NUMBER_VALID, sizeof(int32_t),
          &flag, current_buf_v3);
        if (flag)
          add_metadata_entry(CAM_INTF_META_FRAME_NUMBER, sizeof(uint32_t),
           &frame_number, current_buf_v3);
        add_metadata_entry(CAM_INTF_META_SENSOR_TIMESTAMP,
         sizeof(struct timeval),
         &isp_sof_bus_msg->mono_timestamp, current_buf_v3);
      }

      stream->metadata_stream.current_frame_idx = session_meta->frame_idx;

      // avoid double send metadata during one frame -
      // if session_meta->frame_idx is 0 metadata is not sent
      session_meta->frame_idx = 0;

      ret = ioctl(stream->metadata_stream.buf_mgr_dev_fd,
        VIDIOC_MSM_BUF_MNGR_BUF_DONE, &buf_info);
      if (ret < 0) {
        CDBG_ERROR("%s:Failed to do buf_done", __func__);
        ret = FALSE;
      }
    }

    get_buf.stream_id = (uint32_t)stream->streamid;
    get_buf.session_id = MCT_PIPELINE_SESSION(
            MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));

    //notify tuning server of new metadata frame
    if (!stream->metadata_stream.get_buf_err)
      mct_notify_metadata_frame(current_buf_v1);

    ret = ioctl(stream->metadata_stream.buf_mgr_dev_fd,
      VIDIOC_MSM_BUF_MNGR_GET_BUF, &get_buf);
    if (ret < 0) {
      CDBG_ERROR("%s:Failed to get_buf", __func__);
      ret = FALSE;
      stream->metadata_stream.get_buf_err = TRUE;
      stream->metadata_stream.current_buf_idx = -1;
    } else {
      stream->metadata_stream.get_buf_err = FALSE;
      stream->metadata_stream.current_buf_idx = (int32_t)get_buf.index;
    }

    new_buf_holder = mct_list_find_custom(
                       stream->buffers.img_buf,
                       &(stream->metadata_stream.current_buf_idx),
                       mct_stream_find_metadata_buf);

    if ((new_buf_holder) && (new_buf_holder->data)) {
      mct_stream_map_buf_t *new_buf = new_buf_holder->data;
      if (hal_version == CAM_HAL_V1) {
        memset(new_buf->buf_planes[0].buf, 0x00, sizeof(cam_metadata_info_t));
      } else {
        memset(new_buf->buf_planes[0].buf, 0x00, sizeof(metadata_buffer_t));
        ((metadata_buffer_t *)new_buf->buf_planes[0].buf)
            ->first_flagged_entry = CAM_INTF_PARM_MAX;
      }
    } else {
      CDBG_ERROR("%s:%d: NULL ptr\n", __func__, __LINE__);
      ret = FALSE;
    }
  }

  return ret;
}

/** mct_stream_find_buf:
 *    @
 *    @
 *
 **/
boolean mct_stream_metadata_ctrl_event(mct_stream_t *stream,
  mct_event_t *event)
{
  boolean ret = TRUE;
  int ioctl_ret = 0;
  struct msm_buf_mngr_info buf_info;
  mct_list_t *new_buf_holder = NULL;
  cam_hal_version_t hal_version;
  mct_pipeline_t *pipeline;
  pipeline = (MCT_PIPELINE_CAST(MCT_STREAM_PARENT(stream)->data));
  hal_version = pipeline->hal_version;

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    switch (event->u.ctrl_event.type) {
    case MCT_EVENT_CONTROL_STREAMON: {

      if (pipeline->linked_streams & SEND_METADATA_AT_SOF_MASK) {
        pipeline->bus->msg_to_send_metadata = MCT_BUS_MSG_ISP_SOF;
      } else {
        pipeline->bus->msg_to_send_metadata = MCT_BUS_MSG_META_VALID;
      }

       ret = mct_stream_metabuf_find_bfr_mngr_subdev(
         &stream->metadata_stream.buf_mgr_dev_fd);
       if (ret == FALSE) {
         CDBG_ERROR("%s:failed to find buffer manager subdev \n", __func__);
         break;
       }
       buf_info.stream_id = (uint32_t)stream->streamid;
       buf_info.session_id = MCT_PIPELINE_SESSION(
                MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));

       ioctl_ret = ioctl(stream->metadata_stream.buf_mgr_dev_fd,
         VIDIOC_MSM_BUF_MNGR_GET_BUF, &buf_info);
       if (ioctl_ret < 0) {
         CDBG_ERROR("%s:Failed to get_buf", __func__);
         stream->metadata_stream.get_buf_err = TRUE;
         ret = FALSE;
         break;
       }
       stream->metadata_stream.current_buf_idx = buf_info.index;
       new_buf_holder = mct_list_find_custom(
                       stream->buffers.img_buf,
                       &(stream->metadata_stream.current_buf_idx),
                       mct_stream_find_metadata_buf);

       if ((new_buf_holder) && (new_buf_holder->data)) {
         mct_stream_map_buf_t *new_buf = new_buf_holder->data;
         if (hal_version == CAM_HAL_V1) {
           memset(new_buf->buf_planes[0].buf, 0x00, sizeof(cam_metadata_info_t));
         } else {
           memset(new_buf->buf_planes[0].buf, 0x00, sizeof(metadata_buffer_t));
           ((metadata_buffer_t *)new_buf->buf_planes[0].buf)
            ->first_flagged_entry = CAM_INTF_PARM_MAX;
         }
       } else {
         CDBG_ERROR("%s:%d: NULL ptr\n", __func__, __LINE__);
         ret = FALSE;
         break;
       }
    }
      break;

    case MCT_EVENT_CONTROL_STREAMOFF: {
      if (!stream->metadata_stream.get_buf_err) {
        buf_info.index = (uint32_t)stream->metadata_stream.current_buf_idx;
        buf_info.frame_id = 0;
        buf_info.stream_id = (uint32_t)stream->streamid;
        buf_info.session_id = MCT_PIPELINE_SESSION(
          MCT_PIPELINE_CAST(MCT_OBJECT_PARENT(stream)->data));

        ret = ioctl(stream->metadata_stream.buf_mgr_dev_fd,
          VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buf_info);
        if (ret < 0) {
          CDBG_ERROR("%s:Failed to do buf_done at stream off - errno: %s!!! "
            "for buffer info - index: %d, stream id: %d, session id: %d",
             __func__, strerror(errno),
              buf_info.index, buf_info.stream_id, buf_info.session_id);
        }
      }
      close(stream->metadata_stream.buf_mgr_dev_fd);

      /*Flush bus at this point*/
      mct_pipeline_t *pipeline =
        MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
      if (!pipeline)
        return FALSE;

      mct_bus_t *bus = pipeline->bus;
      if (!bus)
        return FALSE;

      pthread_mutex_lock(&bus->bus_msg_q_lock);
      mct_bus_queue_flush(bus);
      pthread_mutex_unlock(&bus->bus_msg_q_lock);
      ret = TRUE;
    }
      break;

    default: {
      ret = TRUE;
      CDBG_ERROR("%s:%d: ret=%d Unsupported cmd\n", __func__, __LINE__, ret);
    }
      break;
  } /* ctrl event type switch*/
  }
  break;

  default: {
    CDBG_ERROR("%s:%d: ret=%d Unsupported cmd\n", __func__, __LINE__, ret);
    break;
  }
  } /* event type switch*/
  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_stream_send_event(mct_stream_t *stream, mct_event_t *event)
{
  mct_module_t *src_module = NULL;
  boolean ret = FALSE;

  if (stream->streaminfo.stream_type == CAM_STREAM_TYPE_METADATA) {
    ret = mct_stream_metadata_ctrl_event(stream, event);
  }
  else {
    if (MCT_STREAM_CHILDREN(stream)) {
      src_module = (mct_module_t *)(MCT_STREAM_CHILDREN(stream)->data);
    }
    if (src_module) {
      if ((mct_module_find_type(src_module, event->identity)
           == MCT_MODULE_FLAG_SOURCE) &&
           src_module->process_event) {
        ret = src_module->process_event(src_module, event);
      }
    }
  }
  return ret;
}
/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static mct_stream_map_buf_t *mct_stream_create_buffers(
  cam_stream_info_t *stream_info, mct_list_t *img_list,
  mct_serv_ds_msg_t *msg)
{
  uint32_t i;
  mct_stream_map_buf_t *buf_holder;
  mct_list_t *buf_holder_list;
  boolean fd_mapped_flag;
  buf_holder_list = mct_list_find_custom(img_list, msg,
    mct_stream_find_stream_buf);
  cam_stream_buf_plane_info_t buf_planes;

  switch (msg->buf_type) {
  case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
    buf_planes.plane_info.num_planes = 1;
    buf_planes.plane_info.frame_len = msg->size;
    buf_planes.plane_info.sp.len = msg->size;
    break;
  case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
    buf_planes = stream_info->reprocess_config.offline.input_buf_planes;
    break;
  default:
    buf_planes = stream_info->buf_planes;
    break;
  }

  if (!buf_holder_list) {
    buf_holder = malloc(sizeof(mct_stream_map_buf_t));
    if (!buf_holder)
      goto error;
    buf_holder->buf_index = msg->index;
    buf_holder->buf_type = msg->buf_type;
    buf_holder->num_planes = buf_planes.plane_info.num_planes;
    buf_holder->common_fd = FALSE;
  } else {
    buf_holder = (mct_stream_map_buf_t *)(buf_holder_list->data);
    if ((buf_holder->num_planes == 1) ||
        (buf_holder->buf_planes[msg->plane_idx].buf != NULL))
      goto finish;
  }
  //Before mmap lets check the fd value
  if ((msg->fd) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    msg->fd = -1;
    goto error1;
  }
  if (buf_holder->num_planes == 1) {
    buf_holder->buf_planes[0].buf = mmap(NULL, msg->size,
      PROT_READ | PROT_WRITE, MAP_SHARED, msg->fd, 0);
    if (buf_holder->buf_planes[0].buf == MAP_FAILED) {
      CDBG_ERROR("%s: Mapping failed with error %s\n", __func__, strerror(errno));
      goto error1;
    }
    buf_holder->buf_size             = msg->size;
    buf_holder->buf_planes[0].size   = msg->size;
    buf_holder->buf_planes[0].offset = 0;
    buf_holder->buf_planes[0].fd     = msg->fd;
  } else if(buf_holder->num_planes > 1) { /*num_planes > 1*/
    if (msg->plane_idx == -1) { /*same fd is used for all buffers*/
      buf_holder->common_fd = TRUE;
      buf_holder->buf_planes[0].buf = mmap(NULL, msg->size,
        PROT_READ | PROT_WRITE, MAP_SHARED, msg->fd, 0);
      if (buf_holder->buf_planes[0].buf == MAP_FAILED) {
        CDBG_ERROR("%s: Mapping failed with error %s\n", __func__, strerror(errno));
        goto error1;
      }

      buf_holder->buf_size             = msg->size;
      buf_holder->buf_planes[0].size   =
        buf_planes.plane_info.mp[0].len;
      buf_holder->buf_planes[0].offset =
        0 + buf_planes.plane_info.mp[0].offset;
      buf_holder->buf_planes[0].fd     = msg->fd;
      buf_holder->buf_planes[0].stride =
        buf_planes.plane_info.mp[0].stride;
      buf_holder->buf_planes[0].scanline =
        buf_planes.plane_info.mp[0].scanline;

      for (i = 1; i < buf_holder->num_planes; i++) {
        buf_holder->buf_planes[i].size   =
          buf_planes.plane_info.mp[i].len;
        buf_holder->buf_planes[i].offset =
          buf_planes.plane_info.mp[i-1].len +
          buf_planes.plane_info.mp[i].offset;
        buf_holder->buf_planes[i].fd  = buf_holder->buf_planes[0].fd;
        buf_holder->buf_planes[i].buf = buf_holder->buf_planes[0].buf;
        buf_holder->buf_planes[i].stride   =
          buf_planes.plane_info.mp[i].stride;
        buf_holder->buf_planes[i].scanline =
          buf_planes.plane_info.mp[i].scanline;
      }
      for (i = 0; i < buf_holder->num_planes; i++) {
        CDBG("%s: plane idx = %d, offset %d, stride %d, scanline = %d",
              __func__, i, buf_planes.plane_info.mp[i].offset,
              buf_planes.plane_info.mp[i].stride,
              buf_planes.plane_info.mp[i].scanline);
      }
    } else { /*different fds for different planes; not supported atm*/
      buf_holder->common_fd = FALSE;
    }/*different fds for different planes*/
  } /*num_planes > 1*/

finish:
  return buf_holder;
error1:
  free(buf_holder);
error:
  return NULL;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_stream_free_list_data(void *data, void *user_data)
{
  mct_stream_frame_num_idx_map_t *entry =
    (mct_stream_frame_num_idx_map_t *)data;
  free(entry);
  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_stream_destroy_buffers(void *data, void *user_data)
{
  mct_stream_map_buf_t *mbuf = (mct_stream_map_buf_t *)data;
  uint32_t i,j;

  if (!mbuf)
    return FALSE;

  for (i = 0; i < mbuf->num_planes; i++) {
    if (mbuf->buf_planes[i].buf) {
      if (mbuf->common_fd == FALSE)
        munmap(mbuf->buf_planes[i].buf, mbuf->buf_planes[i].size);
      else {
        munmap(mbuf->buf_planes[i].buf, mbuf->buf_size);
      }
      mbuf->buf_planes[i].buf = NULL;
      for (j = 0; j< mbuf->num_planes; j++) {
        if (mbuf->buf_planes[j].buf != NULL &&
              mbuf->buf_planes[j].fd == mbuf->buf_planes[i].fd)
          mbuf->buf_planes[j].buf = NULL;
      }
      close(mbuf->buf_planes[i].fd);
    }
  }

  free(mbuf);
  mbuf = NULL;
  return TRUE;
}

/** mct_stream_map_buf
 *    @message: mct_serv_msg_t message sent from pipeline
 *    @stream: this stream object
 *
 *  Stream processes HAL buffer mappings. In case of SERV_BUF_STREAMINFO,
 *  as stream information is ready, the Stream will start module linking.
 *
 *  Return TRUE if mapped successfully.
 **/
static boolean mct_stream_map_buf(void *message, mct_stream_t *stream)
{
  boolean ret = FALSE;
  mct_serv_ds_msg_t *msg = (mct_serv_ds_msg_t *)message;
  mct_stream_map_buf_t *buf_holder;
  mct_pipeline_t *pipeline;

  if (!msg || !stream || msg->operation != CAM_MAPPING_TYPE_FD_MAPPING ||
      (uint32_t)msg->stream != stream->streamid) {
    ret = FALSE;
    goto finish;
  }
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);

  switch ((cam_mapping_buf_type)(msg->buf_type)) {
  /* Below message are per Stream */
  case CAM_MAPPING_BUF_TYPE_STREAM_INFO: {
  if ((msg->fd) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    msg->fd = -1;
    ret = FALSE;
  }
    stream->buffers.stream_info = mmap(NULL, msg->size, PROT_READ | PROT_WRITE,
      MAP_SHARED, msg->fd, 0);

    if (stream->buffers.stream_info == MAP_FAILED) {
      CDBG_ERROR("%s: Mapping failed with error %s\n", __func__, strerror(errno));
      ret = FALSE;
      break;
    }

    stream->buffers.stream_size = msg->size;
    stream->buffers.stream_fd   = msg->fd;
    ret = TRUE;
  }
    break;

  case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
  case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
  case CAM_MAPPING_BUF_TYPE_STREAM_BUF: {
    buf_holder = mct_stream_create_buffers(stream->buffers.stream_info,
      stream->buffers.img_buf, msg);
    if (!buf_holder) {
      goto error;
    }
    stream->buffers.img_buf = mct_list_append(stream->buffers.img_buf,
      buf_holder, NULL, NULL);

    if (!stream->buffers.img_buf)
      goto error1;
    stream->streaminfo.img_buffer_list = stream->buffers.img_buf;
    if (stream->state == MCT_ST_STATE_RUNNING) {
      mct_event_t         cmd_event;
      mct_event_control_t event_data;
      event_data.type = MCT_EVENT_CONTROL_UPDATE_BUF_INFO;
      event_data.control_event_data = (void *)buf_holder;
      cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
        (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
         MCT_EVENT_DOWNSTREAM, &event_data);
      stream->send_event(stream, &cmd_event);
    }
    ret = TRUE;
  }
    break;

  default:
    goto error;
    break;
  } /* switch */

  goto finish;

error1:
  mct_stream_destroy_buffers(buf_holder, NULL);
error:
  ret = FALSE;
finish:
  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
boolean mct_stream_unmap_buf(void *message, mct_stream_t *stream)
{
  boolean ret = FALSE;
  int rc;
  mct_serv_ds_msg_t *msg = (mct_serv_ds_msg_t *)message;
  mct_stream_map_buf_t *buf_holder;
  mct_list_t *buf_holder_list;
  mct_list_t *buf_list;

  if (!msg || !stream || msg->operation != CAM_MAPPING_TYPE_FD_UNMAPPING ||
      (uint32_t)msg->stream != stream->streamid) {
    ret = FALSE;
    goto finish;
  }

  switch ((cam_mapping_buf_type)(msg->buf_type)) {
  /* Below message are per Stream */
  case CAM_MAPPING_BUF_TYPE_STREAM_INFO: {
    rc = munmap(stream->buffers.stream_info, stream->buffers.stream_size);
    if (rc < 0) {
      ret = FALSE;
      break;
    }
    stream->buffers.stream_info = NULL;
    stream->buffers.stream_size = 0;
    close(stream->buffers.stream_fd);
    ret = TRUE;
  }
    break;

  case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
  case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
  case CAM_MAPPING_BUF_TYPE_STREAM_BUF: {
    buf_holder_list = mct_list_find_custom(stream->buffers.img_buf,
      msg, mct_stream_find_stream_buf);
    if (!buf_holder_list) {
      ret = FALSE;
      goto finish;
    }
    mct_stream_destroy_buffers(buf_holder_list->data, NULL);

    stream->buffers.img_buf = mct_list_remove(stream->buffers.img_buf,
                                buf_holder_list->data);
    stream->streaminfo.img_buffer_list = stream->buffers.img_buf;
    ret = TRUE;
  }
    break;

  default:
    ret = FALSE;
    goto finish;
  }

finish:
  return ret;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
mct_stream_t* mct_stream_new(uint32_t stream_id)
{
  mct_stream_t *stream;

  stream = malloc(sizeof(mct_stream_t));
  if (!stream)
    return NULL;

  memset(stream, 0, sizeof(mct_stream_t));
  stream->streamid  = stream_id;
  stream->state     = MCT_ST_STATE_NONE;

  pthread_mutex_init(MCT_OBJECT_GET_LOCK(stream), NULL);

  stream->add_module     = mct_stream_add_module;
  stream->remove_module  = mct_stream_remove_module;
  stream->send_event     = mct_stream_send_event;
  stream->map_buf        = mct_stream_map_buf;
  stream->unmap_buf      = mct_stream_unmap_buf;
  stream->link           = mct_stream_start_link;
  stream->unlink         = mct_stream_start_unlink;

  return stream;
}

/** mct_stream_remvove_stream_from_module:
 *    @data:
 *    @user_data:
 *
 *  Remove the stream from a module's parent's list
 *
 *  Return TRUE always.
 **/
boolean mct_stream_remvove_stream_from_module(void *data,
  void *user_data)
{
  mct_module_t *module = (mct_module_t *)data;
  mct_stream_t *stream = (mct_stream_t *)user_data;
  mct_list_t   *list = NULL;

  MCT_OBJECT_PARENT(module) = mct_list_remove(MCT_OBJECT_PARENT(module), MCT_OBJECT_CAST(stream));

  MCT_OBJECT_NUM_PARENTS(module) -= 1;

  if (MCT_OBJECT_NUM_PARENTS(module) == 0) {

    CDBG("b. %s: module is: %s, parents num is %d, list = %p\n", __func__, MCT_OBJECT_NAME(module),
      MCT_OBJECT_NUM_PARENTS(module), list);

    if (list != NULL)
      CDBG("c. %s: list->data = %p\n", __func__, list->data);
  }

  return TRUE;
}

static boolean mct_stream_streamoff(void *data, void *user_data)
{
  CDBG_HIGH("%s: Enter\n", __func__);
  mct_stream_t *stream   = (mct_stream_t *)data;
  mct_pipeline_t *pipeline = (mct_pipeline_t *)user_data;
  mct_event_t cmd_event;
  mct_event_control_t event_data;
  if (stream->state == MCT_ST_STATE_RUNNING) {
    event_data.type = MCT_EVENT_CONTROL_STREAMOFF;
    event_data.control_event_data = (void *)&stream->streaminfo;
    CDBG_HIGH("%s: STREAMING OFFstream_type = %d\n", __func__, stream->streaminfo.stream_type);
    cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
    (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
    MCT_EVENT_DOWNSTREAM, &event_data);
    stream->send_event(stream, &cmd_event);
    stream->state = MCT_ST_STATE_NONE;
  }
  return TRUE;
}

static boolean mct_del_offline_stream(void *data, void *user_data)
{
  CDBG_HIGH("%s: Enter\n", __func__);
  mct_stream_t *stream   = (mct_stream_t *)data;
  mct_pipeline_t *pipeline = (mct_pipeline_t *)user_data;
  mct_event_t cmd_event;
  mct_event_control_t event_data;

  event_data.type = MCT_EVENT_CONTROL_DEL_OFFLINE_STREAM;
  event_data.control_event_data = (void *)&stream->streaminfo;
  cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
    (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
    MCT_EVENT_DOWNSTREAM, &event_data);
    stream->send_event(stream, &cmd_event);
  stream->state = MCT_ST_STATE_NONE;
  return TRUE;
}

/** mct_stream_streamoff_no_offline:
 *    @
 *
 **/
static boolean mct_stream_streamoff_no_offline(void *data, void *user_data)
{
  mct_stream_t *stream   = (mct_stream_t *)data;
  if (stream->streaminfo.stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    return mct_stream_streamoff(data, user_data);
  }
  return TRUE;
}

/** mct_stream_destroy:
 *    @
 *
 **/
void mct_stream_destroy(mct_stream_t *stream)
{
  mct_event_t event;
  mct_list_t **buf_list;
  void *end;
  mct_pipeline_t *pipeline;
  if (!stream)
  {
    CDBG_ERROR("%s:%d] stream is NULL, return.", __func__, __LINE__);
    return;
  }
  if (!MCT_STREAM_PARENT(stream)) {
    CDBG_ERROR("%s:%d] stream parent is NULL, return.", __func__, __LINE__);
    return FALSE;
  }
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);

  if (MCT_OBJECT_CHILDREN(stream)) {
    if (stream->streaminfo.stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
      mct_list_traverse(MCT_PIPELINE_CHILDREN(pipeline),
        mct_stream_streamoff_no_offline,
        pipeline);
    } else if (stream->streaminfo.stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
      mct_stream_streamoff(stream, pipeline);
      mct_del_offline_stream(stream, pipeline);
    }
    if (MCT_OBJECT_NUM_CHILDREN(stream) > 1) {
      mct_list_operate_nodes(MCT_OBJECT_CHILDREN(stream),
        mct_stream_operate_unlink, stream);
    } else {
      /* Type is removed in unlink modules,since we have only one
       * module it is not linked, remove type here */
      mct_module_t *single_module = MCT_OBJECT_CHILDREN(stream)->data;
      mct_module_remove_type(single_module, stream->streaminfo.identity);
    }
    /* 1. free stream from module's parent list;
     * 2. free module object from stream's children list */
    mct_list_free_all_on_data(MCT_OBJECT_CHILDREN(stream),
      mct_stream_remvove_stream_from_module, stream);
    MCT_OBJECT_CHILDREN(stream) = NULL;
    MCT_STREAM_NUM_CHILDREN(stream) = 0;
  } else if (stream->streaminfo.stream_type == CAM_STREAM_TYPE_METADATA) {
    mct_stream_streamoff(stream, pipeline);
  }
  mct_pipeline_remove_stream_from_linked_streams(pipeline, stream);

  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(stream));

  if (stream->buffers.stream_info) {
      munmap(stream->buffers.stream_info,
        stream->buffers.stream_size);
    stream->buffers.stream_info = NULL;
  }

  buf_list = &(stream->buffers.img_buf);
  end  = &(stream->buffers) + sizeof(mct_stream_bufs_t);

  do {
    if (*buf_list)
      mct_list_free_all(*buf_list, mct_stream_destroy_buffers);
    buf_list++;
  } while ((void *)buf_list != &(stream->stream_private));

  mct_list_free_all(stream->frame_num_idx_list, mct_stream_free_list_data);
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  mct_object_unparent(MCT_OBJECT_CAST(stream), MCT_OBJECT_CAST(pipeline));

  free(stream);
}
