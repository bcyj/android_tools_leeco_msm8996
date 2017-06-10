/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/
#define ATRACE_TAG ATRACE_TAG_CAMERA
#include <cutils/trace.h>
#include <linux/media.h>
#include "mct_module.h"
#include "module_wnr.h"
#include "mct_stream.h"
#include "pthread.h"
#include "chromatix.h"
#include "sw_wnr_chromatix.h"
#include "mct_stream.h"

#define MODULE_DENOISE_LIB_CHROMATIX_VERSION (0x301)

/** g_detection_chromatix:
 *
 *  Default chromatix for sw wnr specific parameters
 **/
static sw_wnr_chromatix_t g_sw_wnr_chromatix = {
  #include "sw_wnr_chromatix_specific.h"
};

/**
 * Function: module_wnr_client_event_handler
 *
 * Description: event handler for WNR client
 *
 * Arguments:
 *   p_appdata - WNR test object p_event - pointer to the event
 *   p_event - event from the WNR library
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int module_wnr_client_event_handler(void* p_appdata,
  img_event_t *p_event)
{
  wnr_client_t *p_client;
  img_component_ops_t *p_comp;
  int rc = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  p_client = (wnr_client_t *)p_appdata;
  p_comp = &p_client->comp;
  IDBG_LOW("%s:%d] type %d", __func__, __LINE__, p_event->type);

  switch (p_event->type) {
  case QIMG_EVT_BUF_DONE:
    //WNR processes one frame at a time currently. Nothing to be done.
    break;
  case QIMG_EVT_DONE:
    pthread_mutex_lock(&p_client->mutex);
    pthread_cond_signal(&p_client->cond);
    pthread_mutex_unlock(&p_client->mutex);
    break;
  case QIMG_EVT_EARLY_CB_DONE:
    IDBG_HIGH("%s:%d QIMG_EVT_EARLY_CB_DONE, post mct msg] ", __func__, __LINE__);
   // module_wnr_client_post_mct_msg(p_client);
    break;
  default:
    break;
  }
  return rc;

}

/**
 * Function: module_wnr_client_getbuf
 *
 * Description: This function is to open the imaging buffer mgr
 *              and queing and dequeing the buffer.
 * Arguments:
 *   @p_client: wnr client
 *   @pframe: frame pointer
 *   @native_buf: flag to indicate if its a native buffer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_wnr_client_getbuf(wnr_client_t *p_client,
  img_frame_t *pframe, int native_buf)
{
  int rc = IMG_SUCCESS;
  int i = 0;
  uint32_t buf_idx;
  int stride = 0, scanline = 0, padded_size = 0;
  uint32_t size;
  uint8_t *p_addr;
  mct_module_t *p_mct_mod;
  int fd = -1;
  mct_stream_info_t* info = p_client->stream_info;
  isp_buf_divert_t *buf_divert = p_client->p_buf_divert_data;
  mct_stream_map_buf_t *buf_holder;
  mct_stream_info_t* input_stream_info =  NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(p_client->p_sinkport))->data);

  if (!info || !buf_divert || !p_mct_mod) {
    IDBG_ERROR("%s:%d] Invalid inputs", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  pframe->frame_cnt = 1;
  pframe->idx = 0;
  pframe->frame_id = buf_divert ->buffer.sequence;

  input_stream_info = &p_client->input_stream_info;

  if (!input_stream_info) {
    CDBG_ERROR("%s:%d] stream_info NULL\n", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  IDBG_MED("%s:%d] Output dimensions width %d height %d stride %d scanline %d",
    __func__, __LINE__,
    info->dim.width,info->dim.height,
    info->buf_planes.plane_info.mp[0].stride,
    info->buf_planes.plane_info.mp[0].scanline);

  IDBG_MED("%s:%d] Input dimensions width %d height %d stride %d scanline %d",
    __func__, __LINE__,
    input_stream_info->dim.width,input_stream_info->dim.height,
    input_stream_info->buf_planes.plane_info.mp[0].stride,
    input_stream_info->buf_planes.plane_info.mp[0].scanline);

  /* get the frame dimensions from input stream info */
  pframe->info.width = input_stream_info->dim.width;
  pframe->info.height = input_stream_info->dim.height;
  stride = input_stream_info->buf_planes.plane_info.mp[0].stride;
  scanline = input_stream_info->buf_planes.plane_info.mp[0].scanline;
  size = pframe->info.width * pframe->info.height;
  padded_size = stride * scanline;

  /* check actually how many */
  pframe->frame[0].plane_cnt = info->buf_planes.plane_info.num_planes;
  pframe->idx = buf_idx = buf_divert->buffer.index;

  if (NULL == p_client->p_sinkport) {
    IDBG_ERROR("%s:%d] NULL Sink port", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(p_client->p_sinkport))->data);
  IDBG_MED("%s:%d] Dimension %dx%d buf_idx %d %x mod %p port %p pproc %p",
    __func__, __LINE__,
    pframe->info.width, pframe->info.height, buf_idx,
    p_client->identity,
    p_mct_mod,
    p_client->p_sinkport,
    p_client->parent_mod);

  if (!native_buf) {
    buf_holder = mct_module_get_buffer(buf_idx,
      p_client->parent_mod,
      IMGLIB_SESSIONID(p_client->identity),
      IMGLIB_STREAMID(p_client->identity));

    if (NULL == buf_holder) {
      IDBG_ERROR("%s:%d] NULL buff holder", __func__, __LINE__);
      return IMG_ERR_INVALID_OPERATION;
    }
    p_addr = buf_holder->buf_planes[0].buf;
    fd = buf_holder->buf_planes[0].fd;
  } else {
    p_addr = buf_divert->vaddr;
    fd = buf_divert->fd;
   }

  if (NULL == p_addr) {
    IDBG_ERROR("%s:%d] NULL address", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((CAM_FORMAT_YUV_420_NV21 != info->fmt) &&
      (CAM_FORMAT_YUV_422_NV16 != info->fmt) &&
      (CAM_FORMAT_YUV_422_NV61 != info->fmt)) {
    IDBG_ERROR("%s:%d] Wrong image format, fmt=%d", __func__, __LINE__, info->fmt);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < pframe->frame[0].plane_cnt; i++) {
    pframe->frame[0].plane[i].plane_type = i;
    pframe->frame[0].plane[i].fd = fd;
    /* check if the stride is set correctly */
    pframe->frame[0].plane[i].offset = info->buf_planes.plane_info.mp[i].offset;
    pframe->frame[0].plane[i].addr = (i == 0) ? p_addr
      : p_addr + padded_size;
    pframe->frame[0].plane[i].width = pframe->info.width;
    pframe->frame[0].plane[i].height = pframe->info.height;
    if (CAM_FORMAT_YUV_420_NV21 == input_stream_info->fmt)
      pframe->frame[0].plane[i].height /= (i + 1);
    pframe->frame[0].plane[i].stride =
      input_stream_info->buf_planes.plane_info.mp[i].stride;
    pframe->frame[0].plane[i].scanline =
      input_stream_info->buf_planes.plane_info.mp[i].scanline;
    pframe->frame[0].plane[i].length =
      pframe->frame[0].plane[i].height * pframe->frame[0].plane[i].stride;
  }

  if (CAM_FORMAT_YUV_420_NV21 == info->fmt) {
    pframe->info.ss = IMG_H2V2;
  }
  else {
    pframe->info.ss = IMG_H2V1;
  }
  pframe->info.analysis = 0;
  pframe->timestamp = buf_divert->buffer.timestamp.tv_sec * 1000000
                       + buf_divert->buffer.timestamp.tv_usec;

  return rc;
}

/**
 * Function: module_wnr_update_offline_params
 *
 * Description: This function is to update the WNR parameters
 * for offline usecase
 *
 * Arguments:
 *   @p_client: wnr client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_wnr_update_offline_params(wnr_client_t *p_client)
{
  cam_metadata_info_t *metadata_buff = NULL;
  mct_stream_session_metadata_info *session_meta = NULL;
  mct_stream_session_metadata_info *local_meta = NULL;
  wnr_metadata_info_t meta_info;
  int rc;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  rc = module_wnr_client_get_meta_info(p_client, &meta_info);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Metadata info is not available", __func__, __LINE__);
    return rc;
  }
  if (p_client->stream_info->reprocess_config.pp_type ==
      CAM_ONLINE_REPROCESS_TYPE) {
    metadata_buff = mct_module_get_buffer_ptr(meta_info.meta_buf_index,
                        p_client->parent_mod,
                        IMGLIB_SESSIONID(p_client->identity),
                        meta_info.meta_stream_handle);
  } else {
    metadata_buff = module_imglib_common_get_metadata(p_client->stream_info,
        meta_info.meta_buf_index);
  }

  if (NULL == metadata_buff) {
    IDBG_ERROR("%s:%d] Invalid metadata pointer", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  session_meta =
    (mct_stream_session_metadata_info *)&metadata_buff->private_metadata;
  local_meta = &p_client->session_meta;

  local_meta->sensor_data.chromatix_ptr =
                   session_meta->sensor_data.chromatix_ptr;
  local_meta->sensor_data.common_chromatix_ptr =
                   session_meta->sensor_data.common_chromatix_ptr;
  if (NULL == local_meta->sensor_data.chromatix_ptr ||
       NULL == local_meta->sensor_data.common_chromatix_ptr) {
    IDBG_ERROR("%s:%d] Invalid chromatix pointer", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  memcpy(&local_meta->isp_stats_awb_data.private_data[0],
          &session_meta->isp_stats_awb_data.private_data[0],
          sizeof(awb_update_t));

  memcpy(&local_meta->isp_gamma_data.private_data[0],
          &session_meta->isp_gamma_data.private_data[0],
          sizeof(img_gamma_t));

  /* AEC is taken from MODULE EVENT sent by PPROC */

  return IMG_SUCCESS;
}

/**
 * Function: module_wnr_configure_client
 *
 * Description: This function sets the params to the
 *            library coming from metadata
 *
 * Arguments:
 *   @p_client: wnr client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_wnr_configure_client(wnr_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  unsigned int i = 0;
  int sum = 0;

  aec_get_t* aec_get = NULL;
  awb_update_t* awb_update = NULL;
  chromatix_parms_type* chromatix = NULL;
  chromatix_gamma_type *chromatix_gamma = NULL;
  stats_get_data_t* stats_get_data = NULL;
  int16_t* isp_gamma_data = NULL;
  img_component_ops_t *p_comp = &p_client->comp;
  mct_stream_session_metadata_info *session_meta = &p_client->session_meta;
  cam_denoise_param_t *cam_denoise_param = &p_client->cam_denoise_param;
  wd_3a_info_t info_3a;
  wd_mode_t wd_mode;
  img_gamma_t gamma;
  img_gamma_t low_gamma;

  IDBG_MED("%s:%d] E ", __func__, __LINE__);

  if (NULL == p_client->stream_info) {
    IDBG_ERROR("%s:%d] Invalid inputs", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  if (CAM_STREAM_TYPE_OFFLINE_PROC == p_client->stream_info->stream_type) {
    rc = module_wnr_update_offline_params(p_client);
    if (IMG_SUCCESS != rc) return rc;

    if (NULL == session_meta->sensor_data.chromatix_ptr
        || NULL == session_meta->sensor_data.common_chromatix_ptr) {
      IDBG_ERROR("%s:%d] Invalid metadata buffer", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }

    chromatix = session_meta->sensor_data.chromatix_ptr;
    if (MODULE_DENOISE_LIB_CHROMATIX_VERSION != chromatix->chromatix_version) {
      IDBG_ERROR("%s:%d] Invalid hromatix version", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    stats_get_data = (stats_get_data_t*)&session_meta->stats_aec_data.private_data;
    isp_gamma_data = (int16_t*)&session_meta->isp_gamma_data.private_data;
    awb_update = (awb_update_t*)&session_meta->isp_stats_awb_data.private_data;
    info_3a.wb_g_gain = awb_update->gain.g_gain;
    isp_gamma_data = (int16_t*)&session_meta->isp_gamma_data.private_data;
    memcpy(&gamma.table, isp_gamma_data,sizeof(gamma.table));
    chromatix_gamma = &(chromatix->chromatix_VFE.chromatix_gamma);
  }
  else {
    chromatix = p_client->chromatix_param.chromatixPtr;
    ALOGE("%s chromatix %p", __func__,chromatix);
    stats_get_data = &p_client->stats_get;
    if (NULL == chromatix) {
      IDBG_ERROR("%s %d: Chromatix Ptr is NULL", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    chromatix_gamma = &(chromatix->chromatix_VFE.chromatix_gamma);
    for (i=0; i<IMGLIB_ARRAY_SIZE(low_gamma.table)-1; i++) {
      gamma.table[i] = chromatix_gamma->default_gamma_table.gamma[i] +
        ((chromatix_gamma->default_gamma_table.gamma[i + 1] -
        chromatix_gamma->default_gamma_table.gamma[i]) << 8);
    }
    gamma.table[i] = chromatix_gamma->default_gamma_table.gamma[i] + 0xff00;
    info_3a.wb_g_gain = p_client->stats_get.awb_get.g_gain;
  }

    aec_get = &stats_get_data->aec_get;
    info_3a.aec_real_gain = aec_get->real_gain[0];
    info_3a.lux_idx = aec_get->lux_idx;

   for (i=0; i<IMGLIB_ARRAY_SIZE(low_gamma.table)-1; i++) {
        low_gamma.table[i] = chromatix_gamma->lowlight_gamma_table.gamma[i] +
          ((chromatix_gamma->lowlight_gamma_table.gamma[i + 1] -
          chromatix_gamma->lowlight_gamma_table.gamma[i]) << 8);
    }
    low_gamma.table[i] = chromatix_gamma->lowlight_gamma_table.gamma[i] + 0xff00;

    switch (cam_denoise_param->process_plates) {
    case CAM_WAVELET_DENOISE_CBCR_ONLY:
      wd_mode = WD_MODE_CBCR_ONLY;
      break;
    case CAM_WAVELET_DENOISE_STREAMLINE_YCBCR:
      wd_mode = WD_MODE_STREAMLINE_YCBCR;
      break;
    case CAM_WAVELET_DENOISE_STREAMLINED_CBCR:
      wd_mode = WD_MODE_STREAMLINED_CBCR;
      break;
    case CAM_WAVELET_DENOISE_YCBCR_PLANE:
    default:
      wd_mode = WD_MODE_YCBCR_PLANE;
    }

    rc = IMG_COMP_SET_PARAM(p_comp, QWD_3A_INFO, &info_3a);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s : Error: IMG_COMP_SET_PARAM (QWD_3A_INFO)", __func__);
      return IMG_ERR_GENERAL;
    }
    rc = IMG_COMP_SET_PARAM(p_comp, QWD_CHROMATIX, chromatix);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s : Error: IMG_COMP_SET_PARAM (QWD_CHROMATIX)", __func__);
      return IMG_ERR_GENERAL;
    }
    rc = IMG_COMP_SET_PARAM(p_comp, QWD_SW_WNR_SPEC_CHROMATIX,
      &g_sw_wnr_chromatix);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s : Error: IMG_COMP_SET_PARAM (QWD_SW_WNR_CHROMATIX)"
        , __func__);
      return IMG_ERR_GENERAL;
    }

    sum = 0;
    for (i=0; i<IMGLIB_ARRAY_SIZE(low_gamma.table); i++)
      sum += low_gamma.table[i];
    if (sum) {
      rc = IMG_COMP_SET_PARAM(p_comp, QWD_GAMMA_TABLE,&gamma);
      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("%s : Error: IMG_COMP_SET_PARAM (QWD_GAMMA_TABLE)", __func__);
        return IMG_ERR_GENERAL;
      }
    }
    sum = 0;
    for (i=0; i<IMGLIB_ARRAY_SIZE(low_gamma.table); i++)
      sum += low_gamma.table[i];
    if (sum) {
      rc = IMG_COMP_SET_PARAM(p_comp, QWD_LOW_GAMMA_TABLE,&low_gamma);
      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("%s : Error: IMG_COMP_SET_PARAM (QWD_LOW_GAMMA_TABLE)", __func__);
        return IMG_ERR_GENERAL;
      }
    }
    rc = IMG_COMP_SET_PARAM(p_comp, QWD_MODE, &wd_mode);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s : Error: IMG_COMP_SET_PARAM (QWD_MODE)", __func__);
      return IMG_ERR_GENERAL;
    }

    rc = IMG_COMP_SET_PARAM(p_comp, QIMG_CAMERA_DUMP, &p_client->debug_info);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s : Error: IMG_COMP_SET_PARAM (QIMG_CAMERA_DUMP)", __func__);
      return IMG_ERR_GENERAL;
    }

    rc = IMG_COMP_SET_PARAM(p_comp, QWD_EARLY_CB, &p_client->early_cb_enabled);
    IDBG_ERROR("%s : IMG_COMP_SET_PARAM (QWD_EARLY_CB) rc %d", __func__, rc);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s : Error: IMG_COMP_SET_PARAM (QWD_EARLY_CB)", __func__);
      return IMG_ERR_GENERAL;
    }
  IDBG_MED("%s:%d] X ", __func__, __LINE__);
  return rc;
}

/**
 * Function: module_wnr_configure_chroma_buffer
 *
 * Description: When UV downsample is enabled, the
 *   chroma is half the size. This function pads
 *   the buffer since WNR lib cannot handle chroma
 *   stride.
 *
 * Arguments:
 *   @p_frame: Image frame
 *
 * Return values:
 *     imaging error values
 *
 * Notes: Should be called only when CDS is applied
 **/

int module_wnr_configure_chroma_buffer(img_frame_t *p_frame)
{
  int rc = IMG_SUCCESS;
  uint8_t *p_cbcr_addr = NULL, *p_src_addr = NULL, *p_dest_addr = NULL;
  int chroma_width =0, chroma_height = 0, stride = 0;
  int i = 0, j = 0;
  uint8_t *dst_ptr = NULL, *src_ptr = NULL;

  if (NULL == p_frame) {
    IDBG_ERROR("%s %d: Frame is NULL",  __func__,  __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  chroma_width = p_frame[0].frame[0].plane[IY].width/2;
  chroma_height = p_frame[0].frame[0].plane[IC].height;
  stride = p_frame[0].frame[0].plane[IY].stride;

  //Mirror the last 128 columns to the right of the chroma buffer
  p_cbcr_addr = p_frame[0].frame[0].plane[IC].addr;
  p_dest_addr = p_cbcr_addr + chroma_width + 127;
  p_src_addr = p_cbcr_addr + (chroma_width - 129);

  IDBG_LOW("%s %d: Cbcr Addr %p, wxh %dx%d", __func__, __LINE__,
    p_cbcr_addr, chroma_width, chroma_height);
  IDBG_LOW("%s %d: src_addr %p dest addr %p", __func__, __LINE__,
    p_src_addr, p_dest_addr);
  for(i = 0; i < chroma_height; i++) {
    src_ptr = p_src_addr;
    dst_ptr = p_dest_addr;
    for(j = 0; j < 128; j++) {
     *dst_ptr-- = *src_ptr++;
    }
    p_src_addr += stride;
    p_dest_addr += stride;
  }

  if((chroma_height % 2) != 0) {
    //Update height by 1 to make it even
    p_frame[0].frame[0].plane[IC].height = chroma_height + 1;

    //If height is odd, pad the height (Mirror the last but one line)
    p_dest_addr = p_cbcr_addr + (stride * (chroma_height));
    p_src_addr = p_cbcr_addr + (stride * (chroma_height - 2));
    IDBG_LOW("%s %d: src_addr %p dest addr %p base addr %p", __func__, __LINE__,
      p_src_addr, p_dest_addr, p_cbcr_addr);

    for(i = 0; i < chroma_width + 128 ; i++) {
      *p_dest_addr ++ = *p_src_addr++;
    }
  }

  return rc;
}

/**
 * Function: module_wnr_client_exec
 *
 * Description: This function handles the WNR lib configuration,
 *            and frame processing
 *
 * Arguments:
 *   @p_client: wnr client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_wnr_client_exec(wnr_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  img_frame_t *p_frame;
  module_wnr_t *p_mod;

  IDBG_MED("%s:%d] E ", __func__, __LINE__);

  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] WNR client NULL", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  p_frame = &p_client->frame[0];
  p_mod = (module_wnr_t *)p_client->p_mod;

  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] WNR module NULL", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  // Get Frame
  rc = module_wnr_client_getbuf(p_client, p_frame,
    p_client->p_buf_divert_data->native_buf);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s : Error: Cannot get frame", __func__);
    return IMG_ERR_GENERAL;
  }

  ///Set width to the same as stride since WNR does not handle stride
  p_frame[0].info.width = p_frame[0].frame[0].plane[IC].stride =
    p_frame[0].frame[0].plane[IY].stride;

  IDBG_HIGH("%s:%d]CDS enabled %d", __func__, __LINE__,
    p_client->p_buf_divert_data->is_uv_subsampled);

  /*Derive plane[IC] dimensions from plane[IY] if CDS is on*/
  if (p_client->p_buf_divert_data->is_uv_subsampled) {
    p_frame[0].frame[0].plane[IC].stride /= 2;
    p_frame[0].frame[0].plane[IC].width = p_frame[0].frame[0].plane[IC].stride;

    if((p_client->stream_info->fmt == CAM_FORMAT_YUV_422_NV16) ||
      (p_client->stream_info->fmt == CAM_FORMAT_YUV_422_NV61)) {
      p_frame[0].info.ss = IMG_H4V2;
      p_frame[0].frame[0].plane[IC].height =
        (p_frame[0].frame[0].plane[IY].height >> 1);
    } else {
      p_frame[0].info.ss = IMG_H4V4;
      p_frame[0].frame[0].plane[IC].height =
        (p_frame[0].frame[0].plane[IY].height >> 1)/2;
    }
  }

  IDBG_MED("%s:%d] dim %dx%d frame %p Y-stride %d scanline %d", __func__,
    __LINE__, p_frame[0].info.width, p_frame[0].info.height, &p_frame[0],
    p_frame[0].frame[0].plane[IY].stride,
    p_frame[0].frame[0].plane[IY].scanline);
  IDBG_MED("%s:%d] Y widthxheight %dx%d chroma widthxheight %dx%d, c-stride %d"
    "scanline %d",__func__, __LINE__,p_frame[0].frame[0].plane[IY].width,
    p_frame[0].frame[0].plane[IY].height, p_frame[0].frame[0].plane[IC].width,
    p_frame[0].frame[0].plane[IC].height, p_frame[0].frame[0].plane[IC].stride,
    p_frame[0].frame[0].plane[IC].scanline);

  //Dump the frame if dumping is enabled
  if (p_client->debug_info.camera_dump_enabled) {
    char filename[40];
    char *s = "Pre_WNR";
    mod_imglib_get_timestamp_string(p_client->debug_info.timestamp);
    snprintf(filename, sizeof (filename), "%s_%s",
      p_client->debug_info.timestamp, s);
    mod_imglib_dump_frame(p_client->frame, filename, p_frame[0].frame_id);
  }

  pthread_mutex_lock(&p_mod->lib_singleton_mutex);
  rc = module_wnr_configure_client(p_client);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error: Cannot configure lib", __func__, __LINE__);
    pthread_mutex_unlock(&p_mod->lib_singleton_mutex);
    return IMG_ERR_GENERAL;
  }

  // Queuing only 1 buffer at a time.
  rc = IMG_COMP_Q_BUF(p_comp, &p_frame[0], IMG_IN);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_mod->lib_singleton_mutex);
    return rc;
  }

  rc = IMG_COMP_START(p_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_mod->lib_singleton_mutex);
    return rc;
  }
  pthread_mutex_unlock(&p_mod->lib_singleton_mutex);

  IDBG_MED("%s:%d] X ", __func__, __LINE__);
  return rc;
}

/**
 * Function: module_wnr_client_do_buf_done
 *
 * Description: his function handles buffer divert
 *            events from pproc
 *
 * Arguments:
 *   @p_client: WNR client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_wnr_client_do_buf_done(wnr_client_t *p_client,
  mod_img_msg_buf_divert_t *p_divert)
{
  int out_idx;
  int subdev_fd = -1;
  int status;
  int frame_id;
  void *p_addr;
  img_frame_t *p_frame = &p_client->frame[0];

  int stride = p_client->stream_info->buf_planes.plane_info.mp[0].stride;
  int scanline = p_client->stream_info->buf_planes.plane_info.mp[0].scanline;
  int padded_size = stride * scanline;

  status = module_imglib_common_get_bfr_mngr_subdev(&subdev_fd);
  if (subdev_fd < 0) {
    IDBG_ERROR("%s:%d] Error getting subdev", __func__, __LINE__);
    return status;
  }

  out_idx = module_imglib_common_get_buffer(subdev_fd, p_divert->identity);
  if (out_idx < 0) {
    IDBG_ERROR("%s:%d] Error getting output buffer %d", __func__, __LINE__,
      out_idx);
    close(subdev_fd);
    return status;
  }

  /* get output buffer address */
  p_addr = mct_module_get_buffer_ptr(out_idx,
    p_client->parent_mod,
    IMGLIB_SESSIONID(p_divert->identity),
    IMGLIB_STREAMID(p_divert->identity));

  /* copy output buffer */
  if (p_addr) {
    ATRACE_BEGIN("Camera:WNR:memcpy");
    memcpy(p_addr, p_frame->frame[0].plane[0].addr, padded_size * 3/2);
    ATRACE_END();
    frame_id = p_divert->buf_divert.buffer.sequence;
  }
  frame_id = p_divert->buf_divert.buffer.sequence;
  IDBG_MED("%s:%d] ", __func__, __LINE__);

  status = module_imglib_common_release_buffer(subdev_fd, p_divert->identity,
    out_idx, frame_id, TRUE);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] Error getting output buffer %d", __func__, __LINE__,
      out_idx);
  }
  close(subdev_fd);
  return status;
}

/**
 * Function: module_wnr_get_uv_subsampling
 *
 * Description: Get img uv subsampling factor based on stream format
 *
 * Arguments:
 *   @fmt: Stream info format
 *
 * Return values:
 *     img_subsampling_t factor
 *
 * Notes: none
 **/
static img_subsampling_t module_wnr_get_uv_subsampling(cam_format_t fmt)
{
  switch (fmt) {
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21:
    return IMG_H2V2;
  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61:
    return IMG_H2V1;
  default:
    return IMG_HV_MAX;
  }
}

/**
 * Function: module_wnr_client_buffers_allocate
 *
 * Description: this function handles buffer allocate event
 *
 * Arguments:
 *   @userdata: wnr client
 *   @data: not used
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_wnr_client_buffers_allocate(void *userdata, void *data)
{
  int rc = IMG_SUCCESS;
  wnr_client_t *p_client = userdata;
  img_component_ops_t *p_comp;
  wd_buffers_realloc_info_t info;
  int denoise_enable;

  if (!p_client) {
    IDBG_ERROR("%s:%d] Invalid inputs", __func__, __LINE__);
    return;
  }

  p_comp = &p_client->comp;

  if (CAM_STREAM_TYPE_OFFLINE_PROC == p_client->stream_info->stream_type) {

    info.mode =
      p_client->stream_info->reprocess_config.pp_feature_config.denoise2d.process_plates;
    denoise_enable =
      p_client->stream_info->reprocess_config.pp_feature_config.denoise2d.denoise_enable
        && (CAM_QCOM_FEATURE_DENOISE2D &
          p_client->stream_info->reprocess_config.pp_feature_config.feature_mask);

  } else {

    info.mode =
      p_client->stream_info->pp_config.denoise2d.process_plates;
    denoise_enable =
      p_client->stream_info->pp_config.denoise2d.denoise_enable;

  }

  if (denoise_enable) {
    if( (p_client->stream_info->reprocess_config.pp_feature_config.rotation
        == ROTATE_270) ||
      (p_client->stream_info->reprocess_config.pp_feature_config.rotation
        == ROTATE_90)) {
          info.width =
            p_client->stream_info->buf_planes.plane_info.mp[0].width;
          info.height =
            p_client->stream_info->buf_planes.plane_info.mp[0].scanline;
    } else {
      info.width =
        p_client->stream_info->buf_planes.plane_info.mp[0].stride;
      info.height =
        p_client->stream_info->buf_planes.plane_info.mp[0].height;
    }

    info.uv_subsampling =
      module_wnr_get_uv_subsampling(p_client->stream_info->fmt);

    rc = IMG_COMP_SET_PARAM(p_comp, QWD_BUFFERS_REALLOC, &info);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s : Error: IMG_COMP_SET_PARAM (QWD_BUFFERS_REALLOC)",
        __func__);
    }
  }
}

/**
 * Function: module_wnr_client_divert_exec
 *
 * Description: his function handles buffer divert
 *            events from pproc
 *
 * Arguments:
 *   @userdata: wnr client
 *   @data: diverted buffer to be processed
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_wnr_client_divert_exec(void *userdata, void *data)
{
  int rc = IMG_SUCCESS;
  wnr_client_t *p_client = (wnr_client_t *)userdata;
  mod_img_msg_buf_divert_t *p_divert = (mod_img_msg_buf_divert_t *)data;
  mct_event_t buff_divert_event;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!p_client || !p_divert) {
    IDBG_ERROR("%s:%d] Invalid inputs", __func__, __LINE__);
    return;
  }

  p_client->p_buf_divert_data = &p_divert->buf_divert;
  pthread_mutex_lock(&p_client->mutex);

  p_client->state = IMGLIB_STATE_PROCESSING;

  if (p_client->stream_off) {
    p_client->stream_off = FALSE;
    IDBG_HIGH("%s:%d] streamoff called return", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->mutex);
    return;
  }

  ATRACE_BEGIN("Camera:WNR");
  rc = module_wnr_client_exec(p_client);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] WNR Not Successful, rc = %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_client->mutex);
    return;
  }

  /* Reprocessing stage is done. Notify Bus*/
  module_imglib_common_post_bus_msg(p_client->parent_mod, p_client->identity,
    MCT_BUS_MSG_REPROCESS_STAGE_DONE, NULL);

  p_client->state = IMGLIB_STATE_STARTED;

  if (p_client->stream_off) {
    p_client->stream_off = FALSE;
    IDBG_HIGH("%s:%d] streamoff called return", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->mutex);
    return;
  }
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_client->mutex);
    return;
  }
  ATRACE_END();
  IDBG_HIGH("%s:%d] after wait rc %d", __func__, __LINE__, rc);

  if (p_client->debug_info.camera_dump_enabled) {
    char filename[40];
    char * s = "Post_WNR";
    snprintf(filename, sizeof(filename), "%s_%s",
      p_client->debug_info.timestamp, s);
    mod_imglib_dump_frame(p_client->frame, filename,
      p_client->frame[0].frame_id);
  }
  /* Send event to CPP */
  if (!p_client->stream_off) {
    //if we already posted the mct msg in early CB then we don't need to post again
    IDBG_HIGH("gjia %s:%d] p_client->early_cb_enabled%d", __func__, __LINE__, p_client->early_cb_enabled);
    if (!p_client->early_cb_enabled){
    //  module_wnr_client_post_mct_msg(p_client);
    }
    if (p_client->p_srcport) {
      memset(&buff_divert_event, 0x0, sizeof(mct_event_t));
      buff_divert_event.type = MCT_EVENT_MODULE_EVENT;
      buff_divert_event.identity = p_client->identity;
      buff_divert_event.direction = MCT_EVENT_DOWNSTREAM;
      buff_divert_event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
      buff_divert_event.u.module_event.module_event_data =
        p_client->p_buf_divert_data;
      rc =  mct_port_send_event_to_peer(p_client->p_srcport,
        &buff_divert_event);
    } else {
      /* do bufdone for the image */
      module_wnr_client_do_buf_done(p_client, p_divert);
    }
  }

  pthread_mutex_unlock(&p_client->mutex);

  return;
}

/**
 * Function: module_wnr_client_stop
 *
 * Description: This function is used to stop the WNR
 *              client
 *
 * Arguments:
 *   @p_client: WNR client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_wnr_client_stop(wnr_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp;
  module_wnr_t *p_mod;
  int8_t wait;

  if(NULL == p_client) {
    IDBG_ERROR("[%s:%d] Invalid input param NULL.", __func__,__LINE__);
    return IMG_ERR_GENERAL;
  }

  IDBG_MED("%s:%d] state = %d ", __func__, __LINE__,p_client->state);

  p_comp = &p_client->comp;
  p_mod = (module_wnr_t *)p_client->p_mod;

  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] WNR module NULL", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] Invalid inputs", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  pthread_mutex_lock(&p_client->mutex);
  if (p_client->state == IMGLIB_STATE_INIT ||
        p_client->state == IMGLIB_STATE_IDLE) {
    IDBG_ERROR("%s:%d] wnr already stopped", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->mutex);
    return rc;
  }
  pthread_mutex_unlock(&p_client->mutex);

  pthread_mutex_lock(&p_mod->lib_singleton_mutex);
  rc = IMG_COMP_ABORT(p_comp, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] abort failed %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_mod->lib_singleton_mutex);
    return rc;
  }
  pthread_mutex_unlock(&p_mod->lib_singleton_mutex);

  pthread_mutex_lock(&p_client->mutex);
  p_client->state = IMGLIB_STATE_INIT;
  pthread_mutex_unlock(&p_client->mutex);

  return rc;
}

/**
 * Function: module_wnr_client_destroy
 *
 * Description: This function is used to destroy the wnr client
 *
 * Arguments:
 *   @p_client: wnr client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_wnr_client_destroy(wnr_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = NULL;
  module_wnr_t *p_mod = NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (NULL == p_client) {
    return;
  }

  p_mod = (module_wnr_t *)p_client->p_mod;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] WNR module NULL", __func__, __LINE__);
    return;
  }

  p_comp = &p_client->comp;
  IDBG_MED("%s:%d] state %d", __func__, __LINE__, p_client->state);

  if ((IMGLIB_STATE_STARTED == p_client->state)
    || (IMGLIB_STATE_PROCESSING == p_client->state)) {
    module_wnr_client_stop(p_client);
  }

  if (IMGLIB_STATE_INIT == p_client->state) {
    pthread_mutex_lock(&p_mod->lib_singleton_mutex);
    rc = IMG_COMP_DEINIT(p_comp);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] deinit failed %d", __func__, __LINE__, rc);
    }
    pthread_mutex_unlock(&p_mod->lib_singleton_mutex);
    p_client->state = IMGLIB_STATE_IDLE;
  }

  if (IMGLIB_STATE_IDLE == p_client->state) {
    /* Clear the metadata info queue */
    module_wnr_client_clear_meta_info(p_client);

    pthread_mutex_destroy(&p_client->mutex);
    pthread_cond_destroy(&p_client->cond);

    free(p_client);
    p_client = NULL;
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);

}

/** Function: module_wnr_client_create
 *
 * Description: This function is used to create the WNR client
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @p_port: mct port pointer
 *   @identity: identity of the stream
 *   @stream_info: stream info for the reprocess stream
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_wnr_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  uint32_t identity, mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  wnr_client_t *p_client = NULL;
  img_component_ops_t *p_comp = NULL;
  img_core_ops_t *p_core_ops = NULL;
  module_wnr_t *p_mod = (module_wnr_t *)p_mct_mod->module_private;
  mct_list_t *p_temp_list = NULL;
  wd_mode_t wd_mode;
#ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];
#endif

  p_core_ops = &p_mod->core_ops;

  IDBG_MED("%s:%d]", __func__, __LINE__);
  p_client = (wnr_client_t *)malloc(sizeof(wnr_client_t));
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] WNR client alloc failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  /* initialize the variables */
  memset(p_client, 0x0, sizeof(wnr_client_t));

  p_comp = &p_client->comp;
  pthread_mutex_init(&p_client->mutex, NULL);
  pthread_cond_init(&p_client->cond, NULL);
  p_client->state = IMGLIB_STATE_IDLE;
  p_client->stream_info = stream_info;
  wd_mode = WD_MODE_YCBCR_PLANE;

  pthread_mutex_lock(&p_mod->lib_singleton_mutex);

  rc = IMG_COMP_CREATE(p_core_ops, p_comp);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_mod->lib_singleton_mutex);
    goto error;
  }

  rc = IMG_COMP_INIT(p_comp, p_client, &wd_mode);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] init failed %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_mod->lib_singleton_mutex);
    goto error;
  }

  rc = IMG_COMP_SET_CB(p_comp, module_wnr_client_event_handler);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_mod->lib_singleton_mutex);
    goto error;
  }

  pthread_mutex_unlock(&p_mod->lib_singleton_mutex);

  p_client->state = IMGLIB_STATE_INIT;

  /* add the client to the list */
  p_temp_list = mct_list_append(p_mod->wnr_client, p_client, NULL, NULL);
  if (NULL == p_temp_list) {
    IDBG_ERROR("%s:%d] list append failed", __func__, __LINE__);
    rc = IMG_ERR_NO_MEMORY;
    goto error;
  }
  p_mod->wnr_client = p_temp_list;
  p_client->p_sinkport = p_port;
  p_client->identity = identity;
  p_client->parent_mod = p_mod->parent_mod ? p_mod->parent_mod : p_mct_mod;

  p_client->p_mod = p_mod;
#ifdef EARLY_CB
  p_client->early_cb_enabled = TRUE;
#else
  p_client->early_cb_enabled = FALSE;
#endif
  p_port->port_private = p_client;
  memset(p_client->frame, 0x0, sizeof(img_frame_t) * MAX_NUM_FRAMES);

  p_client->debug_info.camera_dump_enabled = FALSE;

#ifdef _ANDROID_
  property_get("persist.camera.dumpmetadata", value, 0);
  p_client->debug_info.camera_dump_enabled = atoi(value);
#endif

  IDBG_MED("%s:%d] Camera Dump enabled %d",
    __func__, __LINE__, p_client->debug_info.camera_dump_enabled );
  IDBG_MED("%s:%d] port %p client %p X", __func__, __LINE__, p_port, p_client);
  return rc;

error:
  if (p_client) {
    module_wnr_client_destroy(p_client);
  }
  return rc;
}

/**
 * Function: module_wnr_client_set_meta_info
 *
 * Description: Set metadata info, the info will be stored in the queue
 *  and used when the processing is set
 *
 * Arguments:
 *   @p_client: wnr client
 *   @p_metadata_info: Metadata info to be set
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This function is not protected it should be called
 *  when client mutex is held to avoid race condition
 **/
int module_wnr_client_set_meta_info(wnr_client_t *p_client,
    wnr_metadata_info_t *p_metadata_info)
{
  wnr_metadata_info_t *p_meta_info_holder;

  if (!p_client || !p_metadata_info) {
    IDBG_ERROR("%s:%d] Invalid input p_client %p p_metadata_info %p",
        __func__, __LINE__, p_client, p_metadata_info);
    return IMG_ERR_INVALID_INPUT;
  }

  if (p_client->meta_buf_q.length >= MODULE_WNR_META_QUEUE_DEPTH) {
    IDBG_ERROR("%s:%d] We reach queue limit discard configuration",
        __func__, __LINE__);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  p_meta_info_holder = malloc(sizeof(*p_meta_info_holder));
  if (NULL == p_meta_info_holder) {
    IDBG_ERROR("%s:%d] No memory", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  memcpy(p_meta_info_holder, p_metadata_info, sizeof(*p_meta_info_holder));
  mct_queue_push_tail(&p_client->meta_buf_q, p_meta_info_holder);

  return IMG_SUCCESS;
}

/**
 * Function: module_wnr_client_get_meta_info
 *
 * Description: Get metadata info. The info will be taken from metadata queue.
 *
 * Arguments:
 *   @p_client: wnr client
 *   @p_metadata_info: Metadata info to be set
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This function is not protected it should be called
 *  when client mutex is held to avoid race condition
 **/
int module_wnr_client_get_meta_info(wnr_client_t *p_client,
    wnr_metadata_info_t *p_metadata_info)
{
  wnr_metadata_info_t *p_meta_info_holder;
  int rc = IMG_SUCCESS;

  if (!p_client || !p_metadata_info) {
    IDBG_ERROR("%s:%d] Invalid input p_client %p p_metadata_info %p",
        __func__, __LINE__, p_client, p_metadata_info);
    return IMG_ERR_INVALID_INPUT;
  }

  p_meta_info_holder = mct_queue_pop_head(&p_client->meta_buf_q);
  if (NULL != p_meta_info_holder) {
    *p_metadata_info = *p_meta_info_holder;
    free(p_meta_info_holder);
  } else {
    IDBG_ERROR("%s:%d] Metadata info not available", __func__, __LINE__);
    rc = IMG_ERR_NOT_FOUND;
  }

  return rc;
}

/**
 * Function: module_wnr_client_clear_meta_info
 *
 * Description: Clear metadata info. It will clear metadata info queue
 *
 * Arguments:
 *   @p_client: wnr client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This function is not protected it should be called
 *  when client mutex is held to avoid race condition
 **/
int module_wnr_client_clear_meta_info(wnr_client_t *p_client)
{
  wnr_metadata_info_t *p_meta_info_holder;
  int i;

  if (!p_client) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < MODULE_WNR_META_QUEUE_DEPTH; i++) {
    p_meta_info_holder = mct_queue_pop_head(&p_client->meta_buf_q);
    if (NULL == p_meta_info_holder)
      break;
    free(p_meta_info_holder);
  }

  return IMG_SUCCESS;
}
