
/*******************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include "qomx_jpeg_dec_test.h"
#include <sys/time.h>

/**
 * MACROS and CONSTANTS
 **/
#define TEMP_H 10
#define TEMP_W 10

#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))

#define CLAMP(x, min, max) MIN(MAX((x), (min)), (max))

#define TIME_IN_US(r) ((uint64_t)r.tv_sec * 1000000LL + r.tv_usec)

/** omx_test_color_format_t:
 *
 * test color format mapping
 **/
typedef struct {
  char *format_str;
  int eColorFormat;
} omx_test_color_format_t;

/** col_formats:
 *
 * Color format mapping from testapp to OMX
 **/
static const omx_test_color_format_t col_formats[17] =
{
  { "YCRCBLP_H2V2",      (int)OMX_QCOM_IMG_COLOR_FormatYVU420SemiPlanar },
  { "YCBCRLP_H2V2",               (int)OMX_COLOR_FormatYUV420SemiPlanar },
  { "YCRCBLP_H2V1",      (int)OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar },
  { "YCBCRLP_H2V1",               (int)OMX_COLOR_FormatYUV422SemiPlanar },
  { "YCRCBLP_H1V2", (int)OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar_h1v2 },
  { "YCBCRLP_H1V2", (int)OMX_QCOM_IMG_COLOR_FormatYUV422SemiPlanar_h1v2 },
  { "YCRCBLP_H1V1",      (int)OMX_QCOM_IMG_COLOR_FormatYVU444SemiPlanar },
  { "YCBCRLP_H1V1",      (int)OMX_QCOM_IMG_COLOR_FormatYUV444SemiPlanar },
  {    "IYUV_H2V2",          (int)OMX_QCOM_IMG_COLOR_FormatYVU420Planar },
  {    "YUV2_H2V2",                   (int)OMX_COLOR_FormatYUV420Planar },
  {    "IYUV_H2V1",          (int)OMX_QCOM_IMG_COLOR_FormatYVU422Planar },
  {    "YUV2_H2V1",                   (int)OMX_COLOR_FormatYUV422Planar },
  {    "IYUV_H1V2",     (int)OMX_QCOM_IMG_COLOR_FormatYVU422Planar_h1v2 },
  {    "YUV2_H1V2",     (int)OMX_QCOM_IMG_COLOR_FormatYUV422Planar_h1v2 },
  {    "IYUV_H1V1",          (int)OMX_QCOM_IMG_COLOR_FormatYVU444Planar },
  {    "YUV2_H1V1",          (int)OMX_QCOM_IMG_COLOR_FormatYUV444Planar },
  {   "MONOCHROME",                     (int)OMX_COLOR_FormatMonochrome }
};

/** preference_str:
 *
 * Decoding preference string
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

/** omx_test_dec_deallocate_buffer:
 *
 *  Arguments:
 *    @p_client: decoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Deallocate buffers
 *
 **/
OMX_ERRORTYPE omx_test_dec_deallocate_buffer(buffer_test_t *p_buffer)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  int rc = 0;
  if (p_buffer->p_pmem_fd >= 0) {
    rc = buffer_deallocate(p_buffer);
    memset(p_buffer, 0x0, sizeof(buffer_test_t));
  } else {
    free(p_buffer->addr);
    p_buffer->addr = NULL;
  }
  return ret;
}

/** omx_test_dec_send_buffers:
 *
 *  Arguments:
 *    @data: decoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Sends the buffer to the OMX component
 *
 **/
OMX_ERRORTYPE omx_test_dec_send_buffers(void *data)
{
  omx_dec_test_t *p_client = (omx_dec_test_t *)data;
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  int i = 0;
  QOMX_BUFFER_INFO lbuffer_info;

  memset(&lbuffer_info, 0x0, sizeof(QOMX_BUFFER_INFO));
  QIDBG_MED("%s:%d] buffer %d", __func__, __LINE__, i);
  lbuffer_info.fd = p_client->in_buffer[i].p_pmem_fd;
  ret = OMX_UseBuffer(p_client->p_handle, &(p_client->p_in_buffers[i]), 0,
    &lbuffer_info, p_client->in_buffer[i].size,
    p_client->in_buffer[i].addr);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }

  QIDBG_MED("%s:%d]", __func__, __LINE__);
  return ret;
}

/** omx_test_dec_free_buffers:
 *
 *  Arguments:
 *    @data: decoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Free the buffers sent to the OMX component
 *
 **/
OMX_ERRORTYPE omx_test_dec_free_buffers(void *data)
{
  omx_dec_test_t *p_client = (omx_dec_test_t *)data;
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  int i = 0;

  QIDBG_MED("%s:%d] buffer %d", __func__, __LINE__, i);
  ret = OMX_FreeBuffer(p_client->p_handle, 0, p_client->p_in_buffers[i]);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }
  ret = OMX_FreeBuffer(p_client->p_handle, 1 , p_client->p_out_buffers[i]);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }
  QIDBG_MED("%s:%d]", __func__, __LINE__);
  return ret;
}

/** omx_test_dec_change_state:
 *
 *  Arguments:
 *    @p_client: decoder test client
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
OMX_ERRORTYPE omx_test_dec_change_state(omx_dec_test_t *p_client,
  OMX_STATETYPE new_state, omx_pending_func_t p_exec)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  QIDBG_MED("%s:%d] new_state %d p_exec %p", __func__, __LINE__,
    new_state, p_exec);


  p_client->pending_event_type = OMX_CommandStateSet;
  p_client->event_pending = OMX_TRUE;
  ret = OMX_SendCommand(p_client->p_handle, OMX_CommandStateSet,
    new_state, NULL);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return OMX_ErrorIncorrectStateTransition;
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  if (p_client->error_flag) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return OMX_ErrorIncorrectStateTransition;
  }
  if (p_exec) {
    ret = p_exec(p_client);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      return OMX_ErrorIncorrectStateTransition;
    }
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  if (OMX_TRUE == p_client->event_pending) {
    pthread_mutex_lock(&p_client->lock);
    QIDBG_MED("%s:%d] before wait", __func__, __LINE__);
    pthread_cond_wait(&p_client->cond, &p_client->lock);
    QIDBG_MED("%s:%d] after wait", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->lock);
  }

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return ret;
}

/** omx_test_dec_set_io_ports:
 *
 *  Arguments:
 *    @p_client: decoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Configure OMX buffer ports
 *
 **/
OMX_ERRORTYPE omx_test_dec_set_io_ports(omx_dec_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;

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

  p_client->inputPort->nPortIndex = 0;
  p_client->outputPort->nPortIndex = 1;

  ret = OMX_GetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
    p_client->inputPort);
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

  p_client->inputPort->format.image.nFrameWidth = p_client->width;
  p_client->inputPort->format.image.nFrameHeight = p_client->height;
  p_client->inputPort->format.image.nStride =
    p_client->inputPort->format.image.nFrameWidth;
  p_client->inputPort->format.image.nSliceHeight =
    p_client->inputPort->format.image.nFrameHeight;
  p_client->inputPort->format.image.eColorFormat =
    p_client->eColorFormat;
  p_client->inputPort->nBufferSize = p_client->in_buffer[0].size;
  p_client->inputPort->nBufferCountActual = p_client->buf_count;
  ret = OMX_SetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
    p_client->inputPort);
  if (ret) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return ret;
  }

  return ret;
}

/** omx_test_dec_allocate_buffer:
 *
 *  Arguments:
 *    @p_client: decoder test client
 *
 *  Return:
 *       OpenMax error values
 *
 *  Description:
 *       Allocate buffers
 *
 **/
OMX_ERRORTYPE omx_test_dec_allocate_buffer(buffer_test_t *p_buffer,
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
    p_buffer->p_pmem_fd = -1;
    /* Allocate heap memory */
    p_buffer->addr = (uint8_t *)malloc(p_buffer->size);
    if (NULL == p_buffer->addr) {
      QIDBG_ERROR("%s:%d] Error",__func__, __LINE__);
      return OMX_ErrorUndefined;
    }
  }
  return ret;
}

/** omx_test_dec_set_rotation_angle:
 *
 *  Arguments:
 *     @p_client: decoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      set rotation angle
 *
 **/
OMX_ERRORTYPE omx_test_dec_set_rotation_angle(omx_dec_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  OMX_CONFIG_ROTATIONTYPE rotType;
  rotType.nPortIndex = 1;
  rotType.nRotation = p_client->rotation;
  ret = OMX_SetConfig(p_client->p_handle, OMX_IndexConfigCommonRotate,
    &rotType);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }
  return ret;
}

/** omx_test_dec_set_scaling_params:
 *
 *  Arguments:
 *     @p_client: decoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      set scaling parameters
 *
 **/
OMX_ERRORTYPE omx_test_dec_set_scaling_params(omx_dec_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  OMX_CONFIG_RECTTYPE recttype;
  if (!p_client->scale_cfg.enable) {
    QIDBG_HIGH("%s:%d] Scaling not enabled", __func__, __LINE__);
    return ret;
  }
  recttype.nLeft = p_client->scale_cfg.h_offset;
  recttype.nTop = p_client->scale_cfg.v_offset;
  recttype.nWidth = p_client->scale_cfg.output_width;
  recttype.nHeight = p_client->scale_cfg.output_height;
  recttype.nPortIndex = 1;
  ret = OMX_SetConfig(p_client->p_handle, OMX_IndexConfigCommonOutputCrop,
    &recttype);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }
  return ret;
}

/** omx_test_dec_fill_input_buffer:
 *
 *  Arguments:
 *     @p_client: decoder test client
 *     @filename: input file name
 *     @p_buffer: buffer to be filled
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      read data from file
 *
 **/
OMX_ERRORTYPE omx_test_dec_fill_input_buffer(omx_dec_test_t *p_client,
  const char *filename,
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
  QIDBG_HIGH("%s:%d] input file size is %d", __func__, __LINE__, file_size);
  p_buffer->size = file_size;

  ret = omx_test_dec_allocate_buffer(p_buffer, p_client->use_pmem);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }
  fread(p_buffer->addr, 1, p_buffer->size, fp);

error:
  fclose(fp);
  return ret;
}

/** omx_test_dec_check_for_completion:
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
OMX_BOOL omx_test_dec_check_for_completion(omx_dec_test_t *p_client)
{
  if ((p_client->ebd_count == p_client->buf_count) &&
    (p_client->fbd_count == p_client->buf_count)) {
    return OMX_TRUE;
  }
  return OMX_FALSE;
}

/** omx_test_dec_ebd:
 *
 *  Arguments:
 *     @hComponent: decoder component handle
 *     @pAppData: decoder client
 *     @pBuffer: OMX buffer
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      empty buffer done callback
 *
 **/
OMX_ERRORTYPE omx_test_dec_ebd(OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData, OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
  omx_dec_test_t *p_client = (omx_dec_test_t *) pAppData;
  OMX_ERRORTYPE ret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] count %d ", __func__, __LINE__, p_client->ebd_count);
  pthread_mutex_lock(&p_client->lock);
  p_client->ebd_count++;
  if (omx_test_dec_check_for_completion(p_client)) {
    pthread_cond_signal(&p_client->cond);
  }
  pthread_mutex_unlock(&p_client->lock);
  return OMX_ErrorNone;
}

/** omx_test_dec_event_handler:
 *
 *  Arguments:
 *     @hComponent: decoder component handle
 *     @pAppData: decoder client
 *     @pBuffer: OMX buffer
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      fill buffer done callback
 *
 **/
OMX_ERRORTYPE omx_test_dec_fbd(OMX_OUT OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_PTR pAppData, OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
  omx_dec_test_t *p_client = (omx_dec_test_t *) pAppData;
  int rc = 0;
  OMX_ERRORTYPE ret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] length = %d state %d", __func__, __LINE__,
    (int)pBuffer->nFilledLen, p_client->state);

  if (QOMX_DEC_STATE_ACTIVE != p_client->state) {
    /* get the buffers */
    return OMX_ErrorNone;
  }
  QIDBG_MED("%s:%d] file = %s", __func__, __LINE__,
    p_client->output_file[p_client->fbd_count]);
  DUMP_TO_FILE(p_client->output_file[p_client->fbd_count], pBuffer->pBuffer,
    (int)pBuffer->nFilledLen);

  pthread_mutex_lock(&p_client->lock);
  p_client->fbd_count++;
  if (omx_test_dec_check_for_completion(p_client)) {
    pthread_cond_signal(&p_client->cond);
  }
  pthread_mutex_unlock(&p_client->lock);

  return OMX_ErrorNone;
}

/** omx_test_dec_event_handler:
 *
 *  Arguments:
 *     @hComponent: decoder component handle
 *     @pAppData: decoder client
 *     @eEvent: event type
 *     @nData1: event parameter
 *     @nData2: event parameter
 *     @pEventData: event data
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      uninitialize decoder test client
 *
 **/
OMX_ERRORTYPE omx_test_dec_event_handler(OMX_IN OMX_HANDLETYPE hComponent,
  OMX_IN OMX_PTR pAppData,
  OMX_IN OMX_EVENTTYPE eEvent,
  OMX_IN OMX_U32 nData1,
  OMX_IN OMX_U32 nData2,
  OMX_IN OMX_PTR pEventData)
{
  omx_dec_test_t *p_client = (omx_dec_test_t *)pAppData;

  QIDBG_HIGH("%s:%d] %d %d %d", __func__, __LINE__, eEvent, (int)nData1,
    (int)nData2);

  pthread_mutex_lock(&p_client->lock);
  if (OMX_EventError == eEvent) {
    p_client->error_flag = OMX_TRUE;
    pthread_cond_signal(&p_client->cond);
  } else if (OMX_EventCmdComplete == eEvent) {
    if (nData1 == p_client->pending_event_type) {
      if (p_client->event_pending == OMX_TRUE) {
        p_client->event_pending = OMX_FALSE;
        pthread_cond_signal(&p_client->cond);
      }
    } else {
      QIDBG_HIGH("%s:%d] Invalid unexpected event %x", __func__, __LINE__,
        (int)nData1);
    }
  } else if (OMX_EventPortSettingsChanged == eEvent) {
    p_client->state = QOMX_DEC_STATE_RECONFIG_OUT_PORT;
    p_client->event_pending = OMX_FALSE;
    pthread_cond_signal(&p_client->cond);
  }
  pthread_mutex_unlock(&p_client->lock);
  QIDBG_HIGH("%s:%d]", __func__, __LINE__);
  return OMX_ErrorNone;
}

/** omx_test_dec_disable_out_port:
 *
 *  Arguments:
 *     @p_client: decoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      Disable the output port
 *
 **/
OMX_ERRORTYPE omx_test_dec_disable_output_port(omx_dec_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  QIDBG_HIGH("%s:%d] ", __func__, __LINE__);


  p_client->pending_event_type = OMX_CommandPortDisable;
  p_client->event_pending = OMX_TRUE;
  ret = OMX_SendCommand(p_client->p_handle, OMX_CommandPortDisable,
    1, NULL);
  if (OMX_ErrorNone != ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  if (OMX_TRUE == p_client->error_flag) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }

  pthread_mutex_lock(&p_client->lock);
  if (OMX_TRUE == p_client->event_pending) {
    QIDBG_MED("%s:%d] before wait", __func__, __LINE__);
    pthread_cond_wait(&p_client->cond, &p_client->lock);
    QIDBG_MED("%s:%d] after wait", __func__, __LINE__);
  }

error:

  pthread_mutex_unlock(&p_client->lock);
  QIDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return ret;
}

/** omx_test_dec_get_subsampling_factor:
 *
 *  Arguments:
 *     @format: image format
 *     @p_w_factor: width factor
 *     @p_h_factor: height factor
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      Get the subsampling factor for the image
 *
 **/
void omx_test_dec_get_subsampling_factor(int format,
  float *p_w_factor, float *p_h_factor,
  float *p_size_factor)
{
  switch (format) {
  case OMX_QCOM_IMG_COLOR_FormatYVU420SemiPlanar:
  case OMX_COLOR_FormatYUV420SemiPlanar:
  case OMX_QCOM_IMG_COLOR_FormatYVU420Planar:
  case OMX_COLOR_FormatYUV420Planar:
    *p_w_factor = .5;
    *p_h_factor = .5;
    *p_size_factor = 1.5;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar:
  case OMX_COLOR_FormatYUV422SemiPlanar:
  case OMX_QCOM_IMG_COLOR_FormatYVU422Planar:
  case OMX_COLOR_FormatYUV422Planar:
    *p_w_factor = .5;
    *p_h_factor = 1.0;
    *p_size_factor = 2.0;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar_h1v2:
  case OMX_QCOM_IMG_COLOR_FormatYUV422SemiPlanar_h1v2:
  case OMX_QCOM_IMG_COLOR_FormatYVU422Planar_h1v2:
  case OMX_QCOM_IMG_COLOR_FormatYUV422Planar_h1v2:
    *p_w_factor = 1.0;
    *p_h_factor = .5;
    *p_size_factor = 2.0;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU444SemiPlanar:
  case OMX_QCOM_IMG_COLOR_FormatYUV444SemiPlanar:
  case OMX_QCOM_IMG_COLOR_FormatYVU444Planar:
  case OMX_QCOM_IMG_COLOR_FormatYUV444Planar:
    *p_w_factor = 1.0;
    *p_h_factor = 1.0;
    *p_size_factor = 3.0;
    break;
  default:
  case OMX_COLOR_FormatMonochrome:
    *p_w_factor = 1.0;
    *p_h_factor = 1.0;
    *p_size_factor = 1.0;
    break;
  }
}

/** omx_test_dec_enable_output_port:
 *
 *  Arguments:
 *     @p_client: decoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      Enable the output port
 *
 **/
OMX_ERRORTYPE omx_test_dec_enable_output_port(omx_dec_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  QIDBG_HIGH("%s:%d] ", __func__, __LINE__);
  float w_factor, h_factor, size_factor;
  QOMX_BUFFER_INFO lbuffer_info;

  memset(&lbuffer_info, 0x0, sizeof(QOMX_BUFFER_INFO));

  /* get output port settings */
  ret = OMX_GetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
    p_client->outputPort);
  if (ret) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return ret;
  }

  if (p_client->width && p_client->height) {
    p_client->outputPort->format.image.nFrameWidth = p_client->width;
    p_client->outputPort->format.image.nStride =
        p_client->outputPort->format.image.nFrameWidth;
    p_client->outputPort->format.image.nFrameHeight = p_client->height;
    p_client->outputPort->format.image.nSliceHeight =
        p_client->outputPort->format.image.nFrameHeight;
  }

  /* allocate the buffer */
  omx_test_dec_get_subsampling_factor(
    (int)p_client->outputPort->format.image.eColorFormat, &w_factor,
    &h_factor, &size_factor);

  QIDBG_HIGH("%s:%d] Image details dim %dx%d format %x size %f",
    __func__, __LINE__,
    (int)p_client->outputPort->format.image.nFrameWidth,
    (int)p_client->outputPort->format.image.nFrameHeight,
    (int)p_client->outputPort->format.image.eColorFormat,
    size_factor);

  p_client->out_buffer[0].size =
    p_client->outputPort->format.image.nStride *
    p_client->outputPort->format.image.nSliceHeight *
    size_factor;
  ret = omx_test_dec_allocate_buffer(&p_client->out_buffer[0],
    p_client->use_pmem);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  p_client->outputPort->nBufferSize = p_client->out_buffer[0].size;
  p_client->outputPort->nBufferCountActual = p_client->buf_count;
  ret = OMX_SetParameter(p_client->p_handle, OMX_IndexParamPortDefinition,
      p_client->outputPort);
  if (ret) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return ret;
  }


  p_client->pending_event_type = OMX_CommandPortEnable;
  p_client->event_pending = OMX_TRUE;
  ret = OMX_SendCommand(p_client->p_handle, OMX_CommandPortEnable,
    1, NULL);
  if (OMX_ErrorNone != ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  lbuffer_info.fd = p_client->out_buffer[0].p_pmem_fd;
  ret = OMX_UseBuffer(p_client->p_handle, &(p_client->p_out_buffers[0]),
      1, &lbuffer_info, p_client->out_buffer[0].size,
      p_client->out_buffer[0].addr);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return ret;
  }

  if (OMX_TRUE == p_client->error_flag) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    ret = OMX_ErrorIncorrectStateTransition;
    goto error;
  }

  pthread_mutex_lock(&p_client->lock);
  if (OMX_TRUE == p_client->event_pending) {
    QIDBG_MED("%s:%d] before wait", __func__, __LINE__);
    pthread_cond_wait(&p_client->cond, &p_client->lock);
    QIDBG_MED("%s:%d] after wait", __func__, __LINE__);
  }
  pthread_mutex_unlock(&p_client->lock);

error:
  QIDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return ret;
}

/** omx_test_dec_reconfig_output_port:
 *
 *  Arguments:
 *     @p_client: decoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      Reconfigure the output port and start the decoding
 *
 **/
OMX_ERRORTYPE omx_test_dec_reconfig_output_port(omx_dec_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  QIDBG_HIGH("%s:%d] ", __func__, __LINE__);

  ret = omx_test_dec_disable_output_port(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  ret = omx_test_dec_enable_output_port(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }


  p_client->state = QOMX_DEC_STATE_ACTIVE;

  ret = OMX_FillThisBuffer(p_client->p_handle,
    p_client->p_out_buffers[0]);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  /* wait for the events*/
  pthread_mutex_lock(&p_client->lock);
  if (OMX_FALSE == omx_test_dec_check_for_completion(p_client)) {
    if (p_client->error_flag == OMX_FALSE) {
      QIDBG_MED("%s:%d] before wait", __func__, __LINE__);
      pthread_cond_wait(&p_client->cond, &p_client->lock);
      QIDBG_MED("%s:%d] after wait", __func__, __LINE__);
    }
  }
  pthread_mutex_unlock(&p_client->lock);
  QIDBG_HIGH("%s:%d] X", __func__, __LINE__);

  return ret;

error:
  QIDBG_HIGH("%s:%d] Error X", __func__, __LINE__);
  return ret;
}

/** omx_test_dec_deinit:
 *
 *  Arguments:
 *     @p_client: decoder test client
 *
 *  Return:
 *     Openmax error types
 *
 *  Description:
 *      uninitialize decoder test client
 *
 **/
OMX_ERRORTYPE omx_test_dec_deinit(omx_dec_test_t *p_client)
{
  OMX_ERRORTYPE ret = OMX_ErrorNone;
  int i = 0;
  QIDBG_HIGH("%s:%d] state %d", __func__, __LINE__, p_client->state);

  if (p_client->state) {
    p_client->state = QOMX_DEC_STATE_IDLE;
    //Reseting error flag since we are deining the component
    p_client->error_flag = OMX_FALSE;
    ret = omx_test_dec_change_state(p_client, OMX_StateIdle, NULL);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      p_client->last_error = ret;
      goto error;
    }
    QIDBG_HIGH("%s:%d] ", __func__, __LINE__);
    ret = omx_test_dec_change_state(p_client, OMX_StateLoaded,
      omx_test_dec_free_buffers);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      p_client->last_error = ret;
      goto error;
    }
  }

error:

  for (i = 0; i < p_client->buf_count; i++) {
    ret = omx_test_dec_deallocate_buffer(&p_client->in_buffer[i]);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    }

    ret = omx_test_dec_deallocate_buffer(&p_client->out_buffer[i]);
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

  if (p_client->p_handle) {
    OMX_FreeHandle(p_client->p_handle);
    p_client->p_handle = NULL;
  }
  return ret;
}

/** omx_test_dec_decode:
 *
 *  Arguments:
 *     @p_client: decoder test client
 *
 *  Return:
 *       none
 *
 *  Description:
 *       main decode function for decoder test client
 *
 **/
void *omx_test_dec_decode(void *data)
{
  omx_dec_test_t *p_client = (omx_dec_test_t *)data;
  int ret = 0;
  int i = 0;
  struct timeval time[2];

  QIDBG_ERROR("%s:%d] Before OMX_GetHandle", __func__, __LINE__);

  ret = OMX_GetHandle(&p_client->p_handle,
    "OMX.qcom.image.jpeg.decoder", p_client,
    &p_client->callbacks);

  QIDBG_ERROR("%s:%d] After OMX_GetHandle", __func__, __LINE__);

  if ((ret != OMX_ErrorNone) || (p_client->p_handle == NULL)) {
    QIDBG_ERROR("%s:%d] ", __func__, __LINE__);
    goto error;
  }

  ret = omx_test_dec_fill_input_buffer(p_client, p_client->in_file_name,
    &p_client->in_buffer[0]);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  ret = omx_test_dec_set_io_ports(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  /*Set rotation angle*/
  // TODO

  /*Set scaling parameters*/
  // TODO

  ret = omx_test_dec_change_state(p_client, OMX_StateIdle,
    omx_test_dec_send_buffers);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  gettimeofday(&time[0], NULL);
  p_client->state = QOMX_DEC_STATE_ACTIVE;
  ret = omx_test_dec_change_state(p_client, OMX_StateExecuting, NULL);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

#ifdef DUMP_INPUT
  DUMP_TO_FILE("/data/test.yuv",
    p_client->p_in_buffers[p_client->ebd_count]->pBuffer,
    (int)p_client->p_in_buffers[p_client->ebd_count]->nAllocLen);
#endif

  p_client->event_pending = OMX_TRUE;
  ret = OMX_EmptyThisBuffer(p_client->p_handle,
    p_client->p_in_buffers[p_client->ebd_count]);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }

  /* wait for the events*/
  pthread_mutex_lock(&p_client->lock);

  if (((p_client->ebd_count == 0) || (p_client->fbd_count == 0)) &&
    (p_client->error_flag == OMX_FALSE) &&
    (p_client->state != QOMX_DEC_STATE_RECONFIG_OUT_PORT)) {
    QIDBG_MED("%s:%d] before wait", __func__, __LINE__);
    pthread_cond_wait(&p_client->cond, &p_client->lock);
    QIDBG_MED("%s:%d] after wait", __func__, __LINE__);
  }
  pthread_mutex_unlock(&p_client->lock);

  /* Check for port settings changed */
  if (QOMX_DEC_STATE_RECONFIG_OUT_PORT == p_client->state) {
    ret = omx_test_dec_reconfig_output_port(p_client);
    if (ret) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      goto error;
    }
  }

  gettimeofday(&time[1], NULL);
  p_client->decode_time = TIME_IN_US(time[1]) - TIME_IN_US(time[0]);
  p_client->decode_time /= 1000LL;

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  /*invoke OMX deinit*/
  ret = omx_test_dec_deinit(p_client);
  if (ret) {
    QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    goto error;
  }
  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  return NULL;

error:
  p_client->last_error = ret;
  QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
  return NULL;
}

/** omx_test_dec_init:
 *
 *  Arguments:
 *     @p_client: decoder test client
 *     @p_test: test arguments
 *
 *  Return:
 *       none
 *
 *  Description:
 *       initialize decoder test client
 *
 **/
void omx_test_dec_init(omx_dec_test_t *p_client, omx_dec_test_args_t *p_test,
  int id)
{
  QIDBG_HIGH("%s:%d] enter", __func__, __LINE__);

  int i = 0;
  memset(p_client, 0x0, sizeof(omx_dec_test_t));
  p_client->abort_time = p_test->abort_time;
  p_client->in_file_name = p_test->in_file_name;
  p_client->eColorFormat = p_test->eColorFormat;
  p_client->rotation = p_test->rotation;
  p_client->abort_time = p_test->abort_time;
  p_client->use_pmem = p_test->use_pmem;
  p_client->scale_cfg = p_test->scale_cfg;
  p_client->b2b_count = p_test->b2b_count;
  p_client->buf_count = 1;
  p_client->width = p_test->width;
  p_client->height = p_test->height;

  if ((p_client->buf_count == 1) && (p_test->instance_cnt == 1)) {
    strlcpy(p_client->output_file[0], p_test->output_file,
      strlen(p_test->output_file)+1);
  } else {
    for (i = 0; i < p_client->buf_count; i++)
      STR_ADD_EXT(p_test->output_file, p_client->output_file[i], id, i);
  }

  /*Set function callbacks*/
  p_client->callbacks.EmptyBufferDone = omx_test_dec_ebd;
  p_client->callbacks.FillBufferDone = omx_test_dec_fbd;
  p_client->callbacks.EventHandler = omx_test_dec_event_handler;

  pthread_mutex_init(&p_client->lock, NULL);
  pthread_cond_init(&p_client->cond, NULL);
  QIDBG_HIGH("%s:%d] exit", __func__, __LINE__);
}

/** omx_test_dec_init_test_args:
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
void omx_test_dec_init_test_args(omx_dec_test_args_t *p_test)
{
  /*Initialize the test argument structure*/
  memset(p_test, 0, sizeof(sizeof(omx_dec_test_args_t)));
  p_test->eColorFormat = col_formats[0].eColorFormat;
  p_test->use_pmem = 1;
  p_test->instance_cnt = 1;
  p_test->b2b_count = 1;
  p_test->width = 0;
  p_test->height = 0;
}

/** omx_test_dec_print_usage:
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
void omx_test_dec_print_usage()
{
  fprintf(stderr, "Usage: program_name [options] [-I <input file>] \n");
  fprintf(stderr, "Mandatory options:\n");
  fprintf(stderr, "  -I FILE\t\tPath to the input file.\n");
  fprintf(stderr, "  -O FILE\t\tPath for the output file.\n");
  fprintf(stderr, "  -W FILE\t\tDefault image width\n");
  fprintf(stderr, "  -H FILE\t\tDefault image height\n");
  fprintf(stderr, "  -F FORMAT\t\tDefault image format:\n");
  fprintf(stderr, "\t\t\t\t%s (0), %s (1), %s (2) %s (3)\n"
    "%s (4), %s (5), %s (6) %s (7)\n %s (8), %s (9), %s (10) %s (11)\n"
    "%s (12), %s (13), %s (14) %s (15) %s (16)\n",
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
  fprintf(stderr, "  -P [0/1] Use PMEM buffers");
  fprintf(stderr, "  -u\t\t\tEnable scale for Image. Make sure the scale"
    "paramters are supplied. Scale will not be applied if scale input"
    "dimensions dont match the actual image dimensions\n");
  fprintf(stderr, "  -m SCALE_I_WIDTH\tMain input width.\n");
  fprintf(stderr, "  -n SCALE_I_HEIGHT\tMain input height.\n");
  fprintf(stderr, "  -x SCALE_H_OFFSET\tMain horizontal offset.\n");
  fprintf(stderr, "  -y SCALE_V_OFFSET\tMain vertical offset.\n");
  fprintf(stderr, "  -M SCALE_O_WIDTH\tMain output width.\n");
  fprintf(stderr, "  -N SCALE_O_HEIGHT\tMain output height.\n");
  fprintf(stderr, "  -S Scale factor used as a replacement for other"
    " scale parameters if the input size is unknown."
    " 1)1/8 2)1/4 3)3/8 4)1/2 5)5/8 6)3/4 7)7/8\n");
  fprintf(stderr, "  -a ABORT_TIME\t\tThe duration before an abort is \
    issued (i milliseconds).\n");
  fprintf(stderr, "  -c INSTANCE COUNT\tNumber of decoder instances \n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  -b B2B COUNT\tNumber of back to back captures \n");
  fprintf(stderr, "\n");
}

/** omx_test_dec_get_input:
 *
 *  Arguments:
 *    @argc
 *    @argv
 *    @p_test - pointer to test argument
 *
 *  Return:
 *       0 or -ve values
 *
 *  Description:
 *       get user input
 *
 **/
int omx_test_dec_get_input(int argc, char *argv[],
  omx_dec_test_args_t *p_test)
{
  int c;
  fprintf(stderr, "====================================================="
    "========\n");
  fprintf(stderr, "Decoder test\n");
  fprintf(stderr, "====================================================="
    "========\n");

  while ((c = getopt(argc, argv, "I:O:W:H:F:Q:r:P:um:n:x:y:M:N:"
    "Uj:k:X:Y:J:K:a:c:b:PZ")) != -1) {
    switch (c) {
    case 'O':
      p_test->output_file = optarg;
      fprintf(stderr, "%-25s%s\n", "Output image path",
        p_test->output_file);
      break;
    case 'I':
      p_test->in_file_name = optarg;
      fprintf(stderr, "%-25s%s\n", "Input image path", p_test->in_file_name);
      break;
    case 'r':
      p_test->rotation = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Rotation", p_test->rotation);
      break;
    case 'W':
      p_test->width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Default width", p_test->width);
      break;
    case 'H':
      p_test->height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Default height", p_test->height);
      break;
    case 'F': {
      int format = 0;
      format = atoi(optarg);
      CLAMP(format, 0, 16);
      p_test->eColorFormat = col_formats[format].eColorFormat;
      fprintf(stderr, "%-25s%s\n", "Default image format",
        col_formats[format].format_str);
      break;
    }
    case 'P': {
      p_test->use_pmem = (atoi(optarg) > 0);
      fprintf(stderr, "%-25s%s\n", "Use PMEM", p_test->use_pmem
        ? "true" : "false");
      break;
    }
    case 'u':
      p_test->scale_cfg.enable = 1;
      fprintf(stderr, "%-25s\n", "scale enabled");
      break;
    case 'm':
      p_test->scale_cfg.input_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "image scale input width",
        p_test->scale_cfg.input_width);
      break;
    case 'n':
      p_test->scale_cfg.input_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "image scale input height",
        p_test->scale_cfg.input_height);
      break;
    case 'M':
      p_test->scale_cfg.output_width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "image scale output width",
        p_test->scale_cfg.output_width);
      break;
    case 'N':
      p_test->scale_cfg.output_height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "image scale output height",
        p_test->scale_cfg.output_height);
      break;
    case 'x':
      p_test->scale_cfg.h_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "image scale h offset",
        p_test->scale_cfg.h_offset);
      break;
    case 'y':
      p_test->scale_cfg.v_offset = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "image scale v offset",
        p_test->scale_cfg.v_offset);
      break;
    case 'S': {
      int scale_int = 0;
      scale_int = atoi(optarg);
      CLAMP(scale_int, 1, 7);
      p_test->scale_factor = (float)scale_int/(float)8.0;
      fprintf(stderr, "%-25s%f\n", "Scale factor ", p_test->scale_factor);
      break;
    }
    case 'c':
      p_test->instance_cnt = atoi(optarg);
      CLAMP(p_test->instance_cnt, 1, MAX_INSTANCES);
      fprintf(stderr, "%-25s%d\n", "Instance count", p_test->instance_cnt);
      break;
    case 'b':
      p_test->b2b_count = atoi(optarg);
      CLAMP(p_test->b2b_count, 1, MAX_DECODE_B2B_COUNT);
      fprintf(stderr, "%-25s%d\n", "Burst count", p_test->b2b_count);
      break;
    default:;
    }
  }
  if (!p_test->in_file_name || !p_test->output_file) {
    fprintf(stderr, "Missing required arguments.\n");
    omx_test_dec_print_usage();
    return -1;
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
  omx_dec_test_args_t test_args;
  omx_dec_test_t client[MAX_INSTANCES];
  int i = 0;

  QIDBG_HIGH("%s:%d] enter", __func__, __LINE__);

  /*Initialize OMX Component*/
  OMX_Init();

  /*Init test args struct with default values*/
  omx_test_dec_init_test_args(&test_args);

  /*Get Command line input and fill test args struct*/
  ret = omx_test_dec_get_input(argc, argv, &test_args);
  if (ret) {
    return -1;
  }

  QIDBG_HIGH("%s:%d] cnt %d", __func__, __LINE__, test_args.instance_cnt);

  for (i = 0; i < test_args.instance_cnt; i++) {
    omx_test_dec_init(&client[i], &test_args, i);

    ret = pthread_create(&client[i].thread_id, NULL, omx_test_dec_decode,
      &client[i]);
    if (ret != 0) {
       fprintf(stderr, "Error in thread creation\n");
      return 0;
    }
  }

  for (i = 0; i < test_args.instance_cnt; i++) {
    pthread_join(client[i].thread_id, NULL);
  }

  OMX_Deinit();
  for (i = 0; i < test_args.instance_cnt; i++)
    fprintf(stderr, "Decoding %d completed %s in %lld milliseconds\n", i,
      (client[i].last_error != OMX_ErrorNone) ?
      "in failure" : "successfully", client[i].decode_time);

  return 0;
}
