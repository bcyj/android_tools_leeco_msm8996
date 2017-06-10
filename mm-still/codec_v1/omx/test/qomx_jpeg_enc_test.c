/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include "qomx_jpeg_enc_test.h"
#include <sys/time.h>
#include "errno.h"

/**
 * MACROS and CONSTANTS
 **/
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))

#define CLAMP(x,min,max) MIN(MAX((x),(min)),(max))

#define TIME_IN_US(r) ((uint64_t)r.tv_sec * 1000000LL + r.tv_usec)

/** omx_test_color_format_t:
 *
 * test color format mapping
 **/
typedef struct {
  char *format_str;
  int eColorFormat;
  float chroma_wt;
} omx_test_color_format_t;

/** col_formats:
 *
 * Color format mapping from testapp to OMX
 **/
static const omx_test_color_format_t col_formats[17] =
{
  { "YCRCBLP_H2V2",      (int)OMX_QCOM_IMG_COLOR_FormatYVU420SemiPlanar, 1.5 },
  { "YCBCRLP_H2V2",               (int)OMX_COLOR_FormatYUV420SemiPlanar, 1.5 },
  { "YCRCBLP_H2V1",      (int)OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar, 2.0 },
  { "YCBCRLP_H2V1",               (int)OMX_COLOR_FormatYUV422SemiPlanar, 2.0 },
  { "YCRCBLP_H1V2", (int)OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar_h1v2, 2.0 },
  { "YCBCRLP_H1V2", (int)OMX_QCOM_IMG_COLOR_FormatYUV422SemiPlanar_h1v2, 2.0 },
  { "YCRCBLP_H1V1",      (int)OMX_QCOM_IMG_COLOR_FormatYVU444SemiPlanar, 3.0 },
  { "YCBCRLP_H1V1",      (int)OMX_QCOM_IMG_COLOR_FormatYUV444SemiPlanar, 3.0 },
  {    "IYUV_H2V2",          (int)OMX_QCOM_IMG_COLOR_FormatYVU420Planar, 1.5 },
  {    "YUV2_H2V2",                   (int)OMX_COLOR_FormatYUV420Planar, 1.5 },
  {    "IYUV_H2V1",          (int)OMX_QCOM_IMG_COLOR_FormatYVU422Planar, 2.0 },
  {    "YUV2_H2V1",                   (int)OMX_COLOR_FormatYUV422Planar, 2.0 },
  {    "IYUV_H1V2",     (int)OMX_QCOM_IMG_COLOR_FormatYVU422Planar_h1v2, 2.0 },
  {    "YUV2_H1V2",     (int)OMX_QCOM_IMG_COLOR_FormatYUV422Planar_h1v2, 2.0 },
  {    "IYUV_H1V1",          (int)OMX_QCOM_IMG_COLOR_FormatYVU444Planar, 3.0 },
  {    "YUV2_H1V1",          (int)OMX_QCOM_IMG_COLOR_FormatYUV444Planar, 3.0 },
  {   "MONOCHROME",                     (int)OMX_COLOR_FormatMonochrome, 1.0 }
};

/** preference_str:
 *
 * Encoding preference string
 **/
const char *preference_str[] =
{
  "Hardware accelerated preferred",
  "Hardware accelerated only",
  "Software based preferred",
  "Software based only",
};

/** omx_pending_func_t:
 *
 * Intermediate function for state change
 **/
typedef OMX_ERRORTYPE (*omx_pending_func_t)(void *);

/** DUMP_TO_FILE:
 *  @filename: file name
 *  @p_addr: address of the buffer
 *  @len: buffer length
 *
 *  dump the image to the file
 **/
#define DUMP_TO_FILE(filename, p_addr, len) ({ \
  int rc = 0; \
  FILE *fp = fopen(filename, "w+"); \
  if (fp) { \
    rc = fwrite(p_addr, 1, len, fp); \
    fclose(fp); \
  } \
})

/**
 * Macros for the STR_ADD_EXT function
 **/
#define EXT_LEN 4
#define FILE_EXT "_%d_%d.jpg"

/** STR_ADD_EXT:
 *  @str_in: input string
 *  @str_out: output string
 *  @ind1: instance index
 *  @ind2: buffer index
 *
 *  add addtional extension to the output string
 **/
#define STR_ADD_EXT(str_in, str_out, ind1, ind2) ({ \
  char temp[MAX_FN_LEN]; \
  int len = strlen(str_in); \
  temp[0] = '\0'; \
  memcpy(temp, str_in, len-EXT_LEN); \
  memcpy(temp + len - EXT_LEN, FILE_EXT, strlen(FILE_EXT)); \
  snprintf(str_out, MAX_FN_LEN-1, temp, ind1, ind2); \
})

/** omx_test_enc_get_buffer_offset:
 *
 *  Arguments:
 *     @width: image width
 *     @height: image height
 *     @p_y_offset: image y offset
 *     @p_cbcr_offset: image cbcr offset
 *     @p_buf_size: image width
 *     @usePadding: flag to indicate of padding can be used
 *     @rotation: image rotation
 *     @p_cbcrStartOffset: image width
 *     @chroma_wt: factor to be multiplied for the chroma buffer
 *
 *  Return:
 *     none
 *
 *  Description:
 *      get the buffer offsets
 *
 **/
void omx_test_enc_get_buffer_offset(OMX_U32 width, OMX_U32 height,
  OMX_U32* p_y_offset, OMX_U32* p_cbcr_offset, OMX_U32* p_buf_size,
  int usePadding,
  int rotation,
  OMX_U32 *p_cbcrStartOffset,
  float chroma_wt)
{
  if (usePadding) {
    int cbcr_offset = 0;
    int actual_size = width*height;
    int padded_size = width * CEILING16(height);
    *p_y_offset = 0;
    *p_cbcr_offset = padded_size;
    if ((rotation == 90) || (rotation == 180)) {
      *p_y_offset += padded_size - actual_size;
      *p_cbcr_offset += ((padded_size - actual_size) >> 1);
    }
    *p_buf_size = padded_size * chroma_wt;
  } else {
    *p_y_offset = 0;
    *p_cbcr_offset = 0;
    *p_buf_size = PAD_TO_WORD(width*height) * chroma_wt;
    *p_cbcrStartOffset = PAD_TO_WORD(width*height);
  }
}

/** omx_test_enc_deallocate_buffer:
 *
 *  Arguments:
 *    @p_client: encoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Deallocate buffers
 *
 **/
OMX_ERRORTYPE omx_test_enc_deallocate_buffer(buffer_test_t *p_buffer,
  int use_pmem)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  int rc = 0;
  if (use_pmem) {
    rc = buffer_deallocate(p_buffer);
    memset(p_buffer, 0x0, sizeof(buffer_test_t));
  } else {
    free(p_buffer->addr);
    p_buffer->addr = NULL;
  }
  return ret;
}

/** omx_test_enc_send_buffers:
 *
 *  Arguments:
 *    @data: encoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Sends the buffer to the OMX component
 *
 **/
OMX_ERRORTYPE omx_test_enc_send_buffers(void *data)
{
  omx_enc_test_t *p_client = (omx_enc_test_t *)data;
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  int i = 0;
  QOMX_BUFFER_INFO lbuffer_info;

  memset(&lbuffer_info, 0x0, sizeof(QOMX_BUFFER_INFO));
  for (i = 0; i < p_client->buf_count; i++) {
    lbuffer_info.fd = p_client->in_buffer[i].p_pmem_fd;
    QIDBG_MED("%s:%d] buffer %d fd - %d", __func__, __LINE__, i,
      (int)lbuffer_info.fd);
    ret = OMX_UseBuffer(p_client->p_handle, &(p_client->p_in_buffers[i]), 0,
      &lbuffer_info, p_client->total_size,
      p_client->in_buffer[i].addr);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      return ret;
    }

    ret = OMX_UseBuffer(p_client->p_handle, &(p_client->p_out_buffers[i]),
      1, NULL, p_client->total_size, p_client->out_buffer[i].addr);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      return ret;
    }

    ret = OMX_UseBuffer(p_client->p_handle, &(p_client->p_thumb_buf[i]), 2,
      &lbuffer_info, p_client->total_size,
      p_client->in_buffer[i].addr);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      return ret;
    }

  }
  QIDBG_MED("%s:%d]", __func__, __LINE__);
  return ret;
}

/** omx_test_enc_free_buffers:
 *
 *  Arguments:
 *    @data: encoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Free the buffers sent to the OMX component
 *
 **/
OMX_ERRORTYPE omx_test_enc_free_buffers(void *data)
{
  omx_enc_test_t *p_client = (omx_enc_test_t *)data;
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  int i = 0;

  for (i = 0; i < p_client->buf_count; i++) {
    QIDBG_MED("%s:%d] buffer %d", __func__, __LINE__, i);
    ret = OMX_FreeBuffer(p_client->p_handle, 0, p_client->p_in_buffers[i]);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      return ret;
    }
    ret = OMX_FreeBuffer(p_client->p_handle, 2 , p_client->p_thumb_buf[i]);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      return ret;
    }
    ret = OMX_FreeBuffer(p_client->p_handle, 1 , p_client->p_out_buffers[i]);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      return ret;
    }
  }
  QIDBG_MED("%s:%d]", __func__, __LINE__);
  return ret;
}

/** omx_test_enc_change_state:
 *
 *  Arguments:
 *    @p_client: encoder test client
 *    @new_state: state to be transitioned to
 *    @p_exec: function to be executed during transition
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Change the state of OMX component
 *
 **/
OMX_ERRORTYPE omx_test_enc_change_state(omx_enc_test_t *p_client,
  OMX_STATETYPE new_state, omx_pending_func_t p_exec)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  QIDBG_MED("%s:%d] new_state %d p_exec %p", __func__, __LINE__,
    new_state, p_exec);

  pthread_mutex_lock(&p_client->lock);


  if (p_client->aborted) {
    QIDBG_MED("%s:%d] Abort has been requested, quiting!!", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->lock);
    return OMX_ErrorNone;
  }

  if (p_client->omx_state != new_state) {
    p_client->state_change_pending = OMX_TRUE;
  } else {
    QIDBG_MED("%s:%d] new_state is the same: %d ", __func__, __LINE__,
    new_state);

    pthread_mutex_unlock(&p_client->lock);

    return OMX_ErrorNone;
  }
  QIDBG_MED("%s:%d] **** Before send command", __func__, __LINE__);
  ret = OMX_SendCommand(p_client->p_handle, OMX_CommandStateSet,
    new_state, NULL);
  QIDBG_MED("%s:%d] **** After send command", __func__, __LINE__);

  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->lock);
    return OMX_ErrorIncorrectStateTransition;
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  if (p_client->error_flag) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->lock);
    return OMX_ErrorIncorrectStateTransition;
  }
  if (p_exec) {
    ret = p_exec(p_client);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      pthread_mutex_unlock(&p_client->lock);
      return OMX_ErrorIncorrectStateTransition;
    }
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  if (p_client->state_change_pending) {
    QIDBG_MED("%s:%d] before wait", __func__, __LINE__);
    pthread_cond_wait(&p_client->cond, &p_client->lock);
    QIDBG_MED("%s:%d] after wait", __func__, __LINE__);
    p_client->omx_state = new_state;
  }
  pthread_mutex_unlock(&p_client->lock);
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return ret;
}

/** omx_test_enc_set_io_ports:
 *
 *  Arguments:
 *    @p_client: encoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Configure OMX buffer ports
 *
 **/
OMX_ERRORTYPE omx_test_enc_set_io_ports(omx_enc_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;

  QIDBG_MED("%s:%d: E", __func__, __LINE__);

  p_client->inputPort = malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  if (NULL == p_client->inputPort) {
    QIDBG_ERROR("%s: Error in malloc for inputPort",__func__);
    return OMX_ErrorInsufficientResources;
  }

  p_client->outputPort = malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  if (NULL == p_client->outputPort) {
    free(p_client->inputPort);
    QIDBG_ERROR("%s: Error in malloc for outputPort ",__func__);
    return OMX_ErrorInsufficientResources;
  }
  p_client->thumbPort = malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  if (NULL == p_client->thumbPort) {
    free(p_client->inputPort);
    free(p_client->outputPort);
    QIDBG_ERROR("%s: Error in malloc for thumbPort",__func__);
    return OMX_ErrorInsufficientResources;
  }

  p_client->inputPort->nPortIndex = 0;
  p_client->outputPort->nPortIndex = 1;
  p_client->thumbPort->nPortIndex = 2;

  ret = OMX_GetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
    p_client->inputPort);
  if (ret) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return ret;
  }

  ret = OMX_GetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
    p_client->thumbPort);
  if (ret) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return ret;
  }

  ret = OMX_GetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
    p_client->outputPort);
  if (ret) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return ret;
  }

  p_client->inputPort->format.image.nFrameWidth =
    p_client->main.width;
  p_client->inputPort->format.image.nFrameHeight =
    p_client->main.height;
  p_client->inputPort->format.image.nStride =
    p_client->main.width;
  p_client->inputPort->format.image.nSliceHeight =
    p_client->main.height;
  p_client->inputPort->format.image.eColorFormat =
    p_client->main.eColorFormat;
  p_client->inputPort->nBufferSize = p_client->total_size;
  p_client->inputPort->nBufferCountActual = p_client->buf_count;
  ret = OMX_SetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
    p_client->inputPort);
  if (ret) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return ret;
  }
  p_client->thumbPort->format.image.nFrameWidth =
    p_client->thumbnail.width;
  p_client->thumbPort->format.image.nFrameHeight =
    p_client->thumbnail.height;
  p_client->thumbPort->format.image.nStride =
    p_client->thumbnail.width;
  p_client->thumbPort->format.image.nSliceHeight =
    p_client->thumbnail.height;
  p_client->thumbPort->format.image.eColorFormat =
    p_client->thumbnail.eColorFormat;
  p_client->thumbPort->nBufferSize = p_client->total_size;
  p_client->thumbPort->nBufferCountActual = p_client->buf_count;
  ret = OMX_SetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
    p_client->thumbPort);
  if (ret) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return ret;
  }

  // Enable thumbnail port
  if (p_client->thumbPort) {
  ret = OMX_SendCommand(p_client->p_handle, OMX_CommandPortEnable,
      p_client->thumbPort->nPortIndex, NULL);
  }

  p_client->outputPort->nBufferSize = p_client->total_size;
  p_client->outputPort->nBufferCountActual = p_client->buf_count;
  ret = OMX_SetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
    p_client->outputPort);
  if (ret) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return ret;
  }

  return ret;
}

/** omx_test_enc_configure_buffer_ext:
 *
 *  Arguments:
 *    @p_client: encoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       configure OMX buffers extension
 *
 **/
OMX_ERRORTYPE omx_test_enc_configure_buffer_ext(omx_enc_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  OMX_INDEXTYPE buffer_index;

  QIDBG_MED("%s:%d: E", __func__, __LINE__);

  omx_test_enc_get_buffer_offset(p_client->main.width,
    p_client->main.height,
    &p_client->frame_info.yOffset,
    &p_client->frame_info.cbcrOffset[0],
    &p_client->total_size,
    p_client->usePadding,
    p_client->rotation,
    &p_client->frame_info.cbcrStartOffset[0],
    p_client->main.chroma_wt);

  ret = OMX_GetExtensionIndex(p_client->p_handle,
    "OMX.QCOM.image.exttype.bufferOffset", &buffer_index);
  if (ret != OMX_ErrorNone) {
    QIDBG_ERROR("%s: %d] Failed", __func__, __LINE__);
    return ret;
  }
  QIDBG_HIGH("%s:%d] yOffset = %lu, cbcrOffset = %lu, totalSize = %lu,"
    "cbcrStartOffset = %lu", __func__, __LINE__,
    p_client->frame_info.yOffset,
    p_client->frame_info.cbcrOffset[0],
    p_client->total_size,
    p_client->frame_info.cbcrStartOffset[0]);

  ret = OMX_SetParameter(p_client->p_handle, buffer_index,
    &p_client->frame_info);
  if (ret != OMX_ErrorNone) {
    QIDBG_ERROR("%s: %d] Failed", __func__, __LINE__);
    return ret;
  }
  return ret;
}

/** omx_test_enc_allocate_buffer:
 *
 *  Arguments:
 *    @p_client: encoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Allocate buffers
 *
 **/
OMX_ERRORTYPE omx_test_enc_allocate_buffer(buffer_test_t *p_buffer,
  int use_pmem)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  /*Allocate buffers*/
  if (use_pmem) {
    p_buffer->addr = (uint8_t *)buffer_allocate(p_buffer);
    if (NULL == p_buffer->addr) {
      QIDBG_ERROR("%s:%d] Error",__func__, __LINE__);
      return OMX_ErrorUndefined;
    }
  } else {
    /* Allocate heap memory */
    p_buffer->addr = (uint8_t *)malloc(p_buffer->size);
    if (NULL == p_buffer->addr) {
      QIDBG_ERROR("%s:%d] Error",__func__, __LINE__);
      return OMX_ErrorUndefined;
    }
  }
  return ret;
}

/** omx_test_enc_set_thumbnail_data:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      set exif info
 *
 **/
OMX_ERRORTYPE omx_test_enc_set_exif_info(omx_enc_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  OMX_INDEXTYPE exif_indextype;
  QOMX_EXIF_INFO exif_info;
  QEXIF_INFO_DATA exif_data[MAX_EXIF_ENTRIES];
  int num_exif_values = 0;

  ret = OMX_GetExtensionIndex(p_client->p_handle,
    "OMX.QCOM.image.exttype.exif", &exif_indextype);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }

  exif_data[num_exif_values].tag_id = EXIFTAGID_GPS_LONGITUDE_REF;
  exif_data[num_exif_values].tag_entry.type = EXIF_ASCII;
  exif_data[num_exif_values].tag_entry.count = 2;
  exif_data[num_exif_values].tag_entry.copy = 1;
  exif_data[num_exif_values].tag_entry.data._ascii = "se";
  num_exif_values++;

  exif_data[num_exif_values].tag_id = EXIFTAGID_GPS_LONGITUDE;
  exif_data[num_exif_values].tag_entry.type = EXIF_RATIONAL;
  exif_data[num_exif_values].tag_entry.count = 1;
  exif_data[num_exif_values].tag_entry.copy = 1;
  exif_data[num_exif_values].tag_entry.data._rat.num = 31;
  exif_data[num_exif_values].tag_entry.data._rat.denom = 1;
  num_exif_values++;

  exif_info.numOfEntries = num_exif_values;
  exif_info.exif_data = exif_data;

  ret = OMX_SetParameter(p_client->p_handle, exif_indextype,
    &exif_info);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }
  return ret;
}

/** omx_test_enc_set_thumbnail_data:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      set thumbnail image quality
 *
 **/
OMX_ERRORTYPE omx_test_enc_set_thumbnail_data(omx_enc_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  QOMX_THUMBNAIL_INFO thumbnail_info;
  OMX_INDEXTYPE thumb_indextype;

  if (p_client->encode_thumbnail) {
    ret = OMX_GetExtensionIndex(p_client->p_handle,
      "OMX.QCOM.image.exttype.thumbnail",
      &thumb_indextype);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      return ret;
    }

    /* fill thumbnail info*/
    thumbnail_info.scaling_enabled = p_client->tn_scale_cfg.enable;
    thumbnail_info.input_width = p_client->thumbnail.width;
    thumbnail_info.input_height = p_client->thumbnail.height;
    thumbnail_info.crop_info.nWidth = p_client->tn_scale_cfg.input_width;
    thumbnail_info.crop_info.nHeight = p_client->tn_scale_cfg.input_height;
    thumbnail_info.crop_info.nLeft = p_client->tn_scale_cfg.h_offset;
    thumbnail_info.crop_info.nTop = p_client->tn_scale_cfg.v_offset;
    thumbnail_info.output_width = p_client->tn_scale_cfg.output_width;
    thumbnail_info.output_height = p_client->tn_scale_cfg.output_height;
    thumbnail_info.tmbOffset = p_client->frame_info;

    ret = OMX_SetParameter(p_client->p_handle, thumb_indextype,
      &thumbnail_info);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      return ret;
    }
  }

  return ret;
}

/** omx_test_enc_set_quality:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      set main image quality
 *
 **/
OMX_ERRORTYPE omx_test_enc_set_quality(omx_enc_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  OMX_IMAGE_PARAM_QFACTORTYPE quality;

  quality.nPortIndex = 0;
  quality.nQFactor = p_client->main.quality;
  QIDBG_MED("%s:%d] Setting main image quality %d",
    __func__, __LINE__, p_client->main.quality);
  ret = OMX_SetParameter(p_client->p_handle, OMX_IndexParamQFactor,
    &quality);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }
  return ret;
}

/** omx_test_enc_set_rotation_angle:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      set rotation angle
 *
 **/
OMX_ERRORTYPE omx_test_enc_set_rotation_angle(omx_enc_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  OMX_CONFIG_ROTATIONTYPE rotType;
  rotType.nPortIndex = 0;
  rotType.nRotation = p_client->rotation;
  ret = OMX_SetConfig(p_client->p_handle, OMX_IndexConfigCommonRotate,
    &rotType);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }
  return ret;
}

/** omx_test_enc_set_scaling_params:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      set scaling parameters
 *
 **/
OMX_ERRORTYPE omx_test_enc_set_scaling_params(omx_enc_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  OMX_CONFIG_RECTTYPE recttype;
  OMX_CONFIG_RECTTYPE rect_type_in;

  memset(&rect_type_in, 0, sizeof(rect_type_in));
  rect_type_in.nPortIndex = 0;
  rect_type_in.nWidth = p_client->main_scale_cfg.input_width;
  rect_type_in.nHeight = p_client->main_scale_cfg.input_height;
  rect_type_in.nLeft = p_client->main_scale_cfg.h_offset;
  rect_type_in.nTop = p_client->main_scale_cfg.v_offset;

  QIDBG_HIGH("%s:%d] OMX_IndexConfigCommonInputCrop w = %d, h = %d, l = %d, t = %d,"
    " port_idx = %d", __func__, __LINE__,
    (int)p_client->main_scale_cfg.input_width, (int)p_client->main_scale_cfg.input_height,
    (int)p_client->main_scale_cfg.h_offset, (int)p_client->main_scale_cfg.v_offset,
    (int)rect_type_in.nPortIndex);

  ret = OMX_SetConfig(p_client->p_handle, OMX_IndexConfigCommonInputCrop,
    &rect_type_in);
  if (OMX_ErrorNone != ret) {
    QIDBG_ERROR("%s:%d] Error in setting input crop params", __func__, __LINE__);
    return ret;
  }

  recttype.nLeft = p_client->main_scale_cfg.h_offset;
  recttype.nTop = p_client->main_scale_cfg.v_offset;
  recttype.nWidth = p_client->main_scale_cfg.output_width;
  recttype.nHeight = p_client->main_scale_cfg.output_height;
  recttype.nPortIndex = 0;
  QIDBG_HIGH("%s:%d] OMX_IndexConfigCommonOutputCrop w = %d, h = %d, l = %d, t = %d,"
    " port_idx = %d", __func__, __LINE__,
    (int)p_client->main_scale_cfg.output_width, (int)p_client->main_scale_cfg.output_height,
    (int)p_client->main_scale_cfg.h_offset, (int)p_client->main_scale_cfg.v_offset,
    (int)rect_type_in.nPortIndex);

  ret = OMX_SetConfig(p_client->p_handle, OMX_IndexConfigCommonOutputCrop,
    &recttype);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error in setting output crop params", __func__, __LINE__);
    return ret;
  }
  return ret;
}

/** omx_test_enc_read_file:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      read data from file
 *
 **/
OMX_ERRORTYPE omx_test_enc_read_file(const char *filename,
  buffer_test_t *p_buffer)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  FILE *fp = NULL;
  int file_size = 0;
  fp = fopen(filename, "rb");
  if (!fp) {
    QIDBG_ERROR("%s:%d] error", __func__, __LINE__);
    return OMX_ErrorUndefined;
  }
  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  QIDBG_HIGH("%s:%d] input file size is %d buf_size %ld",
    __func__, __LINE__, file_size, p_buffer->size);

  if (p_buffer->size > file_size) {
    QIDBG_ERROR("%s:%d] error", __func__, __LINE__);
    fclose(fp);
    return ret;
  }
  fread(p_buffer->addr, 1, p_buffer->size, fp);
  fclose(fp);
  return ret;
}

/** omx_test_enc_check_for_completion:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *     true/false
 *
 *  Description:
 *      check if all the encoding is completed
 *
 **/
OMX_BOOL omx_test_enc_check_for_completion(omx_enc_test_t *p_client)
{
  if ((p_client->ebd_count == p_client->total_ebd_count) &&
    (p_client->fbd_count == p_client->buf_count)) {
    return OMX_TRUE;
  }
  return OMX_FALSE;
}

/** omx_test_enc_ebd:
 *
 *  Arguments:
 *     @hComponent: encoder component handle
 *     @pAppData: encoder client
 *     @pBuffer: OMX buffer
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      empty buffer done callback
 *
 **/
OMX_ERRORTYPE omx_test_enc_ebd(OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData, OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
  omx_enc_test_t *p_client = (omx_enc_test_t *) pAppData;
  OMX_ERRORTYPE ret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] ebd count %d ", __func__, __LINE__, p_client->ebd_count);
  pthread_mutex_lock(&p_client->lock);
  p_client->ebd_count++;
  if (omx_test_enc_check_for_completion(p_client) == OMX_TRUE) {
    pthread_cond_signal(&p_client->cond);
  } else if (p_client->ebd_count < p_client->buf_count) {
    ret = OMX_EmptyThisBuffer(p_client->p_handle,
      p_client->p_in_buffers[p_client->ebd_count]);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      pthread_mutex_unlock(&p_client->lock);
      return ret;
    }

    ret = OMX_EmptyThisBuffer(p_client->p_handle,
      p_client->p_thumb_buf[p_client->ebd_count]);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      pthread_mutex_unlock(&p_client->lock);
      return ret;
    }
  }
  pthread_mutex_unlock(&p_client->lock);
  return OMX_ErrorNone;
}

/** omx_test_enc_event_handler:
 *
 *  Arguments:
 *     @hComponent: encoder component handle
 *     @pAppData: encoder client
 *     @pBuffer: OMX buffer
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      fill buffer done callback
 *
 **/
OMX_ERRORTYPE omx_test_enc_fbd(OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData, OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
  omx_enc_test_t *p_client = (omx_enc_test_t *) pAppData;
  int rc = 0;
  OMX_ERRORTYPE ret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] length = %d", __func__, __LINE__,
    (int)pBuffer->nFilledLen);
  QIDBG_MED("%s:%d] file = %s", __func__, __LINE__,
    p_client->output_file[p_client->fbd_count]);
  DUMP_TO_FILE(p_client->output_file[p_client->fbd_count], pBuffer->pBuffer,
    (int)pBuffer->nFilledLen);

  QIDBG_MED("%s:%d] fbd count %d ", __func__, __LINE__, p_client->fbd_count);
  pthread_mutex_lock(&p_client->lock);
  p_client->fbd_count++;
  if (omx_test_enc_check_for_completion(p_client) == OMX_TRUE) {
    pthread_cond_signal(&p_client->cond);
  } else if (p_client->ebd_count < p_client->buf_count) {
    ret = OMX_FillThisBuffer(p_client->p_handle,
      p_client->p_out_buffers[p_client->fbd_count]);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      pthread_mutex_unlock(&p_client->lock);
      return ret;
    }
  }
  pthread_mutex_unlock(&p_client->lock);

  return OMX_ErrorNone;
}

/** omx_test_enc_event_handler:
 *
 *  Arguments:
 *     @hComponent: encoder component handle
 *     @pAppData: encoder client
 *     @eEvent: event type
 *     @nData1: event parameter
 *     @nData2: event parameter
 *     @pEventData: event data
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      uninitialize encoder test client
 *
 **/
OMX_ERRORTYPE omx_test_enc_event_handler(OMX_IN OMX_HANDLETYPE hComponent,
  OMX_IN OMX_PTR pAppData,
  OMX_IN OMX_EVENTTYPE eEvent,
  OMX_IN OMX_U32 nData1,
  OMX_IN OMX_U32 nData2,
  OMX_IN OMX_PTR pEventData)
{
  omx_enc_test_t *p_client = (omx_enc_test_t *)pAppData;

  QIDBG_HIGH("%s:%d] %d %d %d", __func__, __LINE__, eEvent, (int)nData1,
    (int)nData2);

  if (eEvent == OMX_EventError) {
    pthread_mutex_lock(&p_client->lock);
    p_client->error_flag = OMX_TRUE;
    pthread_cond_signal(&p_client->cond);
    pthread_mutex_unlock(&p_client->lock);
  } else if (eEvent == OMX_EventCmdComplete) {
    pthread_mutex_lock(&p_client->lock);
    if (p_client->state_change_pending == OMX_TRUE) {
      p_client->state_change_pending = OMX_FALSE;
      pthread_cond_signal(&p_client->cond);
    }
    pthread_mutex_unlock(&p_client->lock);
  }
  QIDBG_HIGH("%s:%d]", __func__, __LINE__);
  return OMX_ErrorNone;
}

/** omx_test_enc_deinit:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      uninitialize encoder test client
 *
 **/
OMX_ERRORTYPE omx_test_enc_deinit(omx_enc_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  int i = 0;
  QIDBG_HIGH("%s:%d] encoding %d", __func__, __LINE__, p_client->encoding);

  if (p_client->encoding) {
    p_client->encoding = 0;
    //Reseting error flag since we are deining the component
    p_client->error_flag = OMX_FALSE;

    ret = omx_test_enc_change_state(p_client, OMX_StateIdle, NULL);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      p_client->last_error = ret;
      goto error;
    }
    QIDBG_HIGH("%s:%d] ", __func__, __LINE__);
    ret = omx_test_enc_change_state(p_client, OMX_StateLoaded,
      omx_test_enc_free_buffers);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      p_client->last_error = ret;
      goto error;
    }
  }

error:

  for (i = 0; i < p_client->buf_count; i++) {
    p_client->in_buffer[i].size = p_client->total_size;
    ret = omx_test_enc_deallocate_buffer(&p_client->in_buffer[i],
      p_client->use_pmem);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    }

    p_client->out_buffer[i].size = p_client->total_size;
    ret = omx_test_enc_deallocate_buffer(&p_client->out_buffer[i],
      1);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    }
  }

  if (p_client->inputPort) {
    free(p_client->inputPort);
    p_client->inputPort = NULL;
  }

  if (p_client->outputPort) {
    free(p_client->outputPort);
    p_client->outputPort = NULL;
  }

  if (p_client->thumbPort) {
    free(p_client->thumbPort);
    p_client->thumbPort = NULL;
  }

  if (p_client->p_handle) {
    OMX_FreeHandle(p_client->p_handle);
    p_client->p_handle = NULL;
  }
  return ret;
}

/** omx_test_issue_abort:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      Sends abort to OpenMax layer (state change to idle)
 *
 **/
OMX_ERRORTYPE omx_test_issue_abort(omx_enc_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  pthread_mutex_lock(&p_client->lock);

  p_client->aborted = OMX_TRUE;

  if (p_client->omx_state == OMX_StateExecuting) {
    // Abort
    //
    QIDBG_MED("%s:%d] **** Abort requested & State is Executing -> ABORTING",
        __func__, __LINE__);
    QIDBG_MED("%s:%d] **** Before abort send command", __func__, __LINE__);
    p_client->state_change_pending = OMX_TRUE;
    ret = OMX_SendCommand(p_client->p_handle, OMX_CommandStateSet,
        OMX_StateIdle, NULL);
    QIDBG_MED("%s:%d] **** After abort send command", __func__, __LINE__);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      goto error;
    }
  } else {
    QIDBG_MED("%s:%d] **** Abort rquested but state is not Executing",
        __func__, __LINE__);

  }

  pthread_mutex_unlock(&p_client->lock);
  return OMX_ErrorNone;

error:
  pthread_mutex_unlock(&p_client->lock);
  return ret;
}

/** omx_test_omx_test_abort_is_pending:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *    OMX boolean
 *
 *  Description:
 *      Checks if abort has been requested
 *
 **/
OMX_BOOL omx_test_abort_is_pending(omx_enc_test_t *p_client)
{
  OMX_BOOL ret;

  pthread_mutex_lock(&p_client->lock);

  ret = p_client->aborted;

  pthread_mutex_unlock(&p_client->lock);

  return ret;
}

/** omx_test_abort_thread:
 *
 *  Arguments:
 *     @data: encoder test client
 *
 *  Return:
 *    none
 *
 *  Description:
 *      Checks if abort has been requested
 *
 **/
void *omx_test_abort_thread(void *data)
{
  omx_enc_test_t *p_client = (omx_enc_test_t *)data;
  struct timespec lTs;
  int aMs = p_client->abort_time;
  int ret = clock_gettime(CLOCK_REALTIME, &lTs);

  if (ret < 0)
    goto error;

  if (aMs >= 1000) {
    lTs.tv_sec += (aMs / 1000);
    lTs.tv_nsec += ((aMs % 1000) * 1000000);
  } else {
    lTs.tv_nsec += (aMs * 1000000);
  }

  if (lTs.tv_nsec > 1E9) {
    lTs.tv_sec++;
    lTs.tv_nsec -= 1E9;
  }

  QIDBG_MED("%s:%d] **** ABORT THREAD ****", __func__, __LINE__);

  pthread_mutex_lock(&p_client->abort_mutx);

  QIDBG_MED("%s:%d] before wait", __func__, __LINE__);
  ret = pthread_cond_timedwait(&p_client->abort_cond, &p_client->abort_mutx,
      &lTs);
  QIDBG_MED("%s:%d] after wait", __func__, __LINE__);

  pthread_mutex_unlock(&p_client->abort_mutx);

  if (ret == ETIMEDOUT) {

    QIDBG_MED("%s:%d] ****Issuing abort", __func__, __LINE__);
    ret = omx_test_issue_abort(p_client);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      goto error;
    }
  }

  return NULL;


error:
  p_client->last_error = ret;
  QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
  return NULL;

}

/** omx_test_enc_encode:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *
 *  Return:
 *       none
 *
 *  Description:
 *       main encode function for encoder test client
 *
 **/
void *omx_test_enc_encode(void *data)
{
  omx_enc_test_t *p_client = (omx_enc_test_t *)data;
  int ret = 0;
  int i = 0;
  struct timeval time[2];

  if (p_client->abort_time) {
    ret = pthread_create(&p_client->abort_thread_id, NULL,
        omx_test_abort_thread,
        p_client);

    if (ret != 0) {
      fprintf(stderr, "Error in thread creation\n");
      return 0;
    }
  }

  QIDBG_ERROR("%s:%d: E", __func__, __LINE__);
  ret = OMX_GetHandle(&p_client->p_handle,
    "OMX.qcom.image.jpeg.encoder", p_client,
    &p_client->callbacks);

  if ((ret != OMX_ErrorNone) || (p_client->p_handle == NULL)) {
    QIDBG_ERROR("%s:%d] ", __func__, __LINE__);
    goto error;
  }

  ret = omx_test_enc_configure_buffer_ext(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  ret = omx_test_enc_set_io_ports(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  ret = omx_test_enc_set_quality(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  /*Allocate input buffer*/
  for (i = 0; i < p_client->buf_count; i++) {
    p_client->in_buffer[i].size = p_client->total_size;
    ret = omx_test_enc_allocate_buffer(&p_client->in_buffer[i],
      p_client->use_pmem);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      goto error;
    }

    p_client->in_buffer[i].size = p_client->total_size;
    ret = omx_test_enc_read_file(p_client->main.file_name,
      &p_client->in_buffer[i]);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      goto error;
    }

    p_client->out_buffer[i].size = p_client->total_size;
    ret = omx_test_enc_allocate_buffer(&p_client->out_buffer[i],
      1);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      goto error;
    }
    if (omx_test_abort_is_pending(p_client))
      goto abort;
  }

  /*set exif info*/
  ret = omx_test_enc_set_exif_info(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  /*Set thumbnail data*/
  ret = omx_test_enc_set_thumbnail_data(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  if (omx_test_abort_is_pending(p_client))
      goto abort;

  /*Set rotation angle*/
  ret = omx_test_enc_set_rotation_angle(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  /*Set scaling parameters*/
  ret = omx_test_enc_set_scaling_params(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  if (omx_test_abort_is_pending(p_client))
      goto abort;

  ret = omx_test_enc_change_state(p_client, OMX_StateIdle,
    omx_test_enc_send_buffers);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  if (omx_test_abort_is_pending(p_client))
      goto abort;

  gettimeofday(&time[0], NULL);

  p_client->encoding = 1;
  ret = omx_test_enc_change_state(p_client, OMX_StateExecuting, NULL);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

#ifdef DUMP_INPUT
  DUMP_TO_FILE("/data/test.yuv",
    p_client->p_in_buffers[p_client->ebd_count]->pBuffer,
    (int)p_client->p_in_buffers[p_client->ebd_count]->nAllocLen);
#endif

#ifdef DUMP_THUMBNAIL
  DUMP_TO_FILE("/data/testThumbnail.yuv",
    p_client->p_thumb_buf[p_client->ebd_count]->pBuffer,
    (int)p_client->p_thumb_buf[p_client->ebd_count]->nAllocLen);
#endif

  gettimeofday(&time[0], NULL);
  ret = OMX_EmptyThisBuffer(p_client->p_handle,
    p_client->p_in_buffers[p_client->ebd_count]);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  ret = OMX_EmptyThisBuffer(p_client->p_handle,
    p_client->p_thumb_buf[p_client->ebd_count]);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  ret = OMX_FillThisBuffer(p_client->p_handle,
    p_client->p_out_buffers[p_client->fbd_count]);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  if (omx_test_abort_is_pending(p_client))
    goto abort;


  /* wait for the events*/

  pthread_mutex_lock(&p_client->lock);
  if (omx_test_enc_check_for_completion(p_client) == OMX_FALSE) {
    if (p_client->aborted) {
      goto abort;
    }
    QIDBG_MED("%s:%d] before wait", __func__, __LINE__);
    pthread_cond_wait(&p_client->cond, &p_client->lock);
    QIDBG_MED("%s:%d] after wait", __func__, __LINE__);
  }

  pthread_mutex_unlock(&p_client->lock);


abort:

  pthread_mutex_unlock(&p_client->lock);
  gettimeofday(&time[1], NULL);
  p_client->encode_time = TIME_IN_US(time[1]) - TIME_IN_US(time[0]);
  p_client->encode_time /= 1000LL;

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  /*invoke OMX deinit*/
  ret = omx_test_enc_deinit(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  // Signal and join abort thread
  if (p_client->abort_time) {
    pthread_mutex_lock(&p_client->abort_mutx);
    pthread_cond_signal(&p_client->abort_cond);
    pthread_mutex_unlock(&p_client->abort_mutx);
    pthread_join(p_client->abort_thread_id, NULL);
  }

  return NULL;

error:
  p_client->last_error = ret;
  QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
  return NULL;
}

/** omx_test_enc_init:
 *
 *  Arguments:
 *     @p_client: encoder test client
 *     @p_test: test arguments
 *
 *  Return:
 *       none
 *
 *  Description:
 *       initialize encoder test client
 *
 **/
void omx_test_enc_init(omx_enc_test_t *p_client, omx_enc_test_args_t *p_test,
  int id)
{
  int i = 0;

  QIDBG_MED("%s:%d: E", __func__, __LINE__);

  memset(p_client, 0x0, sizeof(omx_enc_test_t));
  p_client->abort_time = p_test->abort_time;
  p_client->main = p_test->main;
  p_client->thumbnail = p_test->thumbnail;
  p_client->rotation = p_test->rotation;
  p_client->encode_thumbnail = p_test->encode_thumbnail;
  p_client->use_pmem = p_test->use_pmem;
  p_client->main_scale_cfg = p_test->main_scale_cfg;
  p_client->tn_scale_cfg = p_test->tn_scale_cfg;
  p_client->buf_count = p_test->burst_count;

  if(p_test->encode_thumbnail) {
    p_client->total_ebd_count = p_client->buf_count * 2;
  } else {
    p_client->total_ebd_count = p_client->buf_count;
  }

  if ((p_client->buf_count == 1) && (p_test->instance_cnt == 1)) {
    strlcpy(p_client->output_file[0], p_test->output_file,
      strlen(p_test->output_file)+1);
  } else {
    for (i = 0; i < p_client->buf_count; i++)
      STR_ADD_EXT(p_test->output_file, p_client->output_file[i], id, i);
  }
  /*Set function callbacks*/
  p_client->callbacks.EmptyBufferDone = omx_test_enc_ebd;
  p_client->callbacks.FillBufferDone = omx_test_enc_fbd;
  p_client->callbacks.EventHandler = omx_test_enc_event_handler;

  pthread_mutex_init(&p_client->lock, NULL);
  pthread_cond_init(&p_client->cond, NULL);

  pthread_mutex_init(&p_client->abort_mutx, NULL);
  pthread_cond_init(&p_client->abort_cond, NULL);

  p_client->omx_state = OMX_StateInvalid;
  p_client->aborted = OMX_FALSE;
}

/** omx_test_enc_init_test_args:
 *
 *  Arguments:
 *     @p_test: test arguments
 *
 *  Return:
 *       none
 *
 *  Description:
 *       initialize testapp arguments
 *
 **/
void omx_test_enc_init_test_args(omx_enc_test_args_t *p_test)
{
  /*Initialize the test argument structure*/
  memset(p_test, 0, sizeof(sizeof(omx_enc_test_args_t)));
  p_test->main.eColorFormat = col_formats[0].eColorFormat;
  p_test->main.quality = 75;
  p_test->thumbnail.eColorFormat = col_formats[0].eColorFormat;
  p_test->thumbnail.quality = 75;
  p_test->use_pmem = 0;
  p_test->instance_cnt = 1;
  p_test->burst_count = 1;
  p_test->abort_time = 0;
}

/** omx_test_enc_print_usage:
 *
 *  Arguments:
 *
 *  Return:
 *       none
 *
 *  Description:
 *       print usage of test client
 *
 **/
void omx_test_enc_print_usage()
{
  fprintf(stderr, "Usage: program_name [options] [-I <input file>] "
    "[-O <output file] [-W <width>] [-H <height>]"
    "[-F<format>]\n");
  fprintf(stderr, "Mandatory options:\n");
  fprintf(stderr, "  -I FILE\t\tPath to the input file.\n");
  fprintf(stderr, "  -O FILE\t\tPath for the output file.\n");
  fprintf(stderr, "  -W WIDTH\t\tInput image width.\n");
  fprintf(stderr, "  -H HEIGHT\t\tInput image height.\n");
  fprintf(stderr, "  -F FORMAT\t\tInput image format:\n");
  fprintf(stderr, "\t\t\t\t%s (0), %s (1), %s (2) %s (3)\n"
    "\t\t\t\t%s (4), %s (5), %s (6) %s (7)\n "
    "\t\t\t\t%s (8), %s (9), %s (10) %s (11),\n"
    "\t\t\t\t%s (12), %s (13), %s (14), %s (15), %s (16).\n",
    col_formats[0].format_str, col_formats[1].format_str,
    col_formats[2].format_str, col_formats[3].format_str,
    col_formats[4].format_str, col_formats[5].format_str,
    col_formats[6].format_str, col_formats[7].format_str,
    col_formats[8].format_str, col_formats[9].format_str,
    col_formats[10].format_str, col_formats[11].format_str,
    col_formats[12].format_str, col_formats[13].format_str,
    col_formats[14].format_str, col_formats[15].format_str,
    col_formats[16].format_str);
  fprintf(stderr, "Optional:\n");
  fprintf(stderr, "  -r ROTATION\t\tRotation (clockwise) in degrees\n");
  fprintf(stderr, "  -t\t\t\tEncode thumbnail image as well. If turned on, "
    "the four arguments below \n\t\t\t\t should be supplied as well as "
    "thumbnail scale params.\n");
  fprintf(stderr, "  -i FILE\t\tPath to the input file for thumbnail.\n");
  fprintf(stderr, "  -w WIDTH\t\tThumbnail image width.\n");
  fprintf(stderr, "  -h HEIGHT\t\tThumbnail image height.\n");
  fprintf(stderr, "  -f FORMAT\t\tThumbnail image format: (same as main)\n");
  fprintf(stderr, "  -Q QUALITY\t\tQuality factor for main image (1-100).\n");
  fprintf(stderr, "  -p PREFERENCE\t\tPreference on which encoder to use"
    "(Software-ased or Hardware-accelerated).\n");
  fprintf(stderr, "               \t\t\tHW preferred (0-default), HW only(1), "
    "SW peferred (2), SW only (3)\n");
  fprintf(stderr, "  -P PMEM\t\tEnable use of PMEM.\n");
  fprintf(stderr, "  -u\t\t\tEnable scale for Main Image. Make sure the scale "
    "paramters are supplied.\n");
  fprintf(stderr, "  -m SCALE_I_WIDTH\tMain input width.\n");
  fprintf(stderr, "  -n SCALE_I_HEIGHT\tMain input height.\n");
  fprintf(stderr, "  -x SCALE_H_OFFSET\tMain horizontal offset.\n");
  fprintf(stderr, "  -y SCALE_V_OFFSET\tMain vertical offset.\n");
  fprintf(stderr, "  -M SCALE_O_WIDTH\tMain output width.\n");
  fprintf(stderr, "  -N SCALE_O_HEIGHT\tMain output height.\n");
  fprintf(stderr, "  -U\t\t\tEnable scale for Thumbnail. Make sure the scale "
    "parameters are supplied.\n");
  fprintf(stderr, "  -j SCALE_I_WIDTH\tThumbnail input width.\n");
  fprintf(stderr, "  -k SCALE_I_HEIGHT\tThumbnail input height.\n");
  fprintf(stderr, "  -X SCALE_H_OFFSET\tThumbnail horizontal offset.\n");
  fprintf(stderr, "  -Y SCALE_V_OFFSET\tThumbnail vertical offset.\n");
  fprintf(stderr, "  -J SCALE_O_WIDTH\tThumbnail output width.\n");
  fprintf(stderr, "  -K SCALE_O_HEIGHT\tThumbnail output height.\n");
  fprintf(stderr, "  -a ABORT_TIME\t\tThe duration before an abort is "
    "issued (i milliseconds).\n");
  fprintf(stderr, "  -R RESTART_INTERVAL\tRestart Interval in number of MCUs\n");
  fprintf(stderr, "  -c INSTANCE COUNT\tNumber of encoder instances \n");
  fprintf(stderr, "  -b BURST COUNT\tNumber of burst mode captures \n");
  fprintf(stderr, "\n");
}

/** omx_test_enc_get_input:
 *
 *  Arguments:
 *    @argc
 *    @argv
 *    @p_test -
 *
 *  Return:
 *       0 or -ve values
 *
 *  Description:
 *       get user input
 *
 **/
int omx_test_enc_get_input(int argc, char *argv[],
  omx_enc_test_args_t *p_test)
{
  int c;
  fprintf(stderr, "====================================================="
    "========\n");
  fprintf(stderr, "Encoder test\n");
  fprintf(stderr, "====================================================="
    "========\n");

  while ((c = getopt(argc, argv, "I:O:W:H:F:Q:r:ti:w:h:f:p:um:n:x:y:M:N:"
    "Uj:k:X:Y:J:K:a:c:b:PZ")) != -1) {
    switch (c) {
    case 'O':
      p_test->output_file = optarg;
      fprintf(stderr, "%-25s%s\n", "Output image path",
        p_test->output_file);
      break;
    case 'I':
      p_test->main.file_name = optarg;
      fprintf(stderr, "%-25s%s\n", "Input image path", p_test->main.file_name);
      break;
    case 'i':
      p_test->thumbnail.file_name = optarg;
      fprintf(stderr, "%-25s%s\n", "Thumbnail image path",
        p_test->thumbnail.file_name);
      break;
    case 'Q':
      p_test->main.quality = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Main image quality", p_test->main.quality);
      break;
    case 'W':
      p_test->main.width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input width", p_test->main.width);
      break;
    case 'w':
      p_test->thumbnail.width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Thumbnail width", p_test->thumbnail.width);
      break;
    case 'H':
      p_test->main.height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input height", p_test->main.height);
      break;
    case 'h':
      p_test->thumbnail.height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Thumbnail height", p_test->thumbnail.height);
      break;
    case 'r':
      p_test->rotation = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Rotation", p_test->rotation);
      break;
    case 't':
      p_test->encode_thumbnail = 1;
      fprintf(stderr, "%-25s%s\n", "Encode thumbnail", "true");
      break;
    case 'F': {
      int format = 0;
      format = atoi(optarg);
      CLAMP(format, 0, 16);
      p_test->main.eColorFormat = col_formats[format].eColorFormat;
      p_test->main.chroma_wt = col_formats[format].chroma_wt;
      fprintf(stderr, "%-25s%s\n", "Input format",
        col_formats[format].format_str);
      break;
    }
    case 'f': {
      int format = 0;
      format = atoi(optarg);
      CLAMP(format, 0, 16);
      p_test->thumbnail.eColorFormat = col_formats[format].eColorFormat;
      fprintf(stderr, "%-25s%s\n", "Thumbnail Input format",
        col_formats[format].format_str);
      break;
    }
    case 'p':
      fprintf(stderr, "%-25s\n", "Preference not supported");
      return -1;
    case 'u':
      p_test->main_scale_cfg.enable = 1;
      fprintf(stderr, "%-25s%s\n", "scale enabled for main image", "true");
      break;
    case 'm':
      p_test->main_scale_cfg.input_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale input width",
        p_test->main_scale_cfg.input_width);
      break;
    case 'n':
      p_test->main_scale_cfg.input_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale input height",
        p_test->main_scale_cfg.input_height);
      break;
    case 'M':
      p_test->main_scale_cfg.output_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale output width",
        p_test->main_scale_cfg.output_width);
      break;
    case 'N':
      p_test->main_scale_cfg.output_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale output height",
        p_test->main_scale_cfg.output_height);
      break;
    case 'x':
      p_test->main_scale_cfg.h_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale h offset",
        p_test->main_scale_cfg.h_offset);
      break;
    case 'y':
      p_test->main_scale_cfg.v_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "main image scale v offset",
        p_test->main_scale_cfg.v_offset);
      break;
    case 'U':
      p_test->tn_scale_cfg.enable = 1;
      fprintf(stderr, "%-25s%s\n", "scale enabled for thumbnail", "true");
      break;
    case 'j':
      p_test->tn_scale_cfg.input_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale input width",
        p_test->tn_scale_cfg.input_width);
      break;
    case 'k':
      p_test->tn_scale_cfg.input_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale input height",
        p_test->tn_scale_cfg.input_height);
      break;
    case 'J':
      p_test->tn_scale_cfg.output_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale output width",
        p_test->tn_scale_cfg.output_width);
      break;
    case 'K':
      p_test->tn_scale_cfg.output_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale output height",
        p_test->tn_scale_cfg.output_height);
      break;
    case 'X':
      p_test->tn_scale_cfg.h_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale h offset",
        p_test->tn_scale_cfg.h_offset);
      break;
    case 'Y':
      p_test->tn_scale_cfg.v_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "thumbnail scale v offset",
        p_test->tn_scale_cfg.v_offset);
      break;
    case 'P':
      p_test->use_pmem = 1;
      fprintf(stderr, "%-25s%s\n", "Use PMEM", "true");
      break;
    case 'c':
      p_test->instance_cnt = atoi(optarg);
      CLAMP(p_test->instance_cnt, 1, MAX_INSTANCES);
      fprintf(stderr, "%-25s%d\n", "Instance count", p_test->instance_cnt);
      break;
    case 'b':
      p_test->burst_count = atoi(optarg);
      CLAMP(p_test->burst_count, 1, MAX_TEST_BUFFERS);
      fprintf(stderr, "%-25s%d\n", "Burst count", p_test->burst_count);
      break;
    case 'a':
      p_test->abort_time = atoi(optarg);
      CLAMP(p_test->abort_time, 1, 5000);
      fprintf(stderr, "%-25s%d\n", "Abort time", p_test->abort_time);
      break;
    default:;
    }
  }
  if (!p_test->main.file_name || !p_test->output_file ||
      !p_test->main.width || !p_test->main.height) {
    fprintf(stderr, "Missing required arguments.\n");
    omx_test_enc_print_usage();
    return -1;
  }

  if (p_test->tn_scale_cfg.output_width % 2 != 0 ||
    p_test->tn_scale_cfg.output_height % 2 != 0 ||
    p_test->thumbnail.width % 2 != 0 ||
    p_test->thumbnail.height % 2 != 0) {
    fprintf(stderr, "\nERROR: Thumbnail input/output w and h must be an even value\n");
    return -1;
  }

  if (p_test->encode_thumbnail) {
    if (!p_test->thumbnail.file_name ||
      !p_test->thumbnail.width ||
      !p_test->thumbnail.height) {
      /* use parameters from main image*/
      p_test->thumbnail.file_name = p_test->main.file_name;
      p_test->thumbnail.width = p_test->main.width;
      p_test->thumbnail.height = p_test->main.height;
    }
  }
  return 0;
}

/** main:
 *
 *  Arguments:
 *    @argc
 *    @argv
 *
 *  Return:
 *       0 or -ve values
 *
 *  Description:
 *       main function
 *
 **/
int main(int argc, char* argv[])
{
  int ret = 0;
  omx_enc_test_args_t test_args;
  omx_enc_test_t client[MAX_INSTANCES];
  int i = 0;

  QIDBG_HIGH("%s:%d] enter", __func__, __LINE__);

  /*Initialize OMX Component*/
  OMX_Init();

  /*Init test args struct with default values*/
  omx_test_enc_init_test_args(&test_args);

  /*Get Command line input and fill test args struct*/
  ret = omx_test_enc_get_input(argc, argv, &test_args);
  if (ret) {
    return -1;
  }

  QIDBG_HIGH("%s:%d] cnt %d", __func__, __LINE__, test_args.instance_cnt);

  for (i = 0; i < test_args.instance_cnt; i++) {
    omx_test_enc_init(&client[i], &test_args, i);

    ret = pthread_create(&client[i].thread_id, NULL, omx_test_enc_encode,
      &client[i]);
    if (ret != 0) {
       fprintf(stderr, "Error in thread creation\n");
      return 0;
    }
  }

  for (i = 0; i < test_args.instance_cnt; i++) {
      QIDBG_HIGH("%s:%d] thread id > 0", __func__, __LINE__);
      pthread_join(client[i].thread_id, NULL);
  }

  OMX_Deinit();
  for (i = 0; i < test_args.instance_cnt; i++)
    fprintf(stderr, "Encoding %d completed %s in %lld milliseconds\n", i,
      (client[i].last_error != OMX_ErrorNone) ?
      "in failure" : "successfully", client[i].encode_time);

  return 0;
}
