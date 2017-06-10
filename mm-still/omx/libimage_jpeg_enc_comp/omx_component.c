/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "omx_component.h"

#define JPEGE_FRAGMENT_SIZE (64*1024)
#define DEFAULT_MAIN_QUALITY 75
#define DEFAULT_THUMBNAIL_QUALITY 75
#define DEFAULT_COLOR_FORMAT YCRCBLP_H2V2
pthread_mutex_t jpegoutput_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t jpegStop_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t jpegencoding_mutex = PTHREAD_MUTEX_INITIALIZER;

static OMX_COMPONENTTYPE *local_fns;
static OMX_IN OMX_CALLBACKTYPE* local_callbacks;
static OMX_IN OMX_PTR local_appData;

omx_jpeg_queue_item getItem;
omx_jpeg_queue_item tempItem;

int init(void);

OMX_ERRORTYPE
omx_component_image_use_output_buffer(OMX_IN OMX_HANDLETYPE hComp,
OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
OMX_IN OMX_U32                        port,
OMX_IN OMX_PTR                     appData,
OMX_IN OMX_U32                       bytes,
OMX_IN OMX_U8*                      buffer);
OMX_ERRORTYPE
omx_component_image_use_input_buffer(OMX_IN OMX_HANDLETYPE hComp,
OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
OMX_IN OMX_U32                        port,
OMX_IN OMX_PTR                     appData,
OMX_IN OMX_U32                       bytes,
OMX_IN OMX_U8*                      buffer);

void omx_jpeg_message_queue_init(omx_jpeg_message_queue * queue);
int postMessage(omx_jpeg_message_queue * mqueue, omx_jpeg_queue_type type,
omx_jpeg_queue_item * item);
void deinit(omx_jpeg_comp * comp);

int jpegEncode(omx_jpeg_comp * comp, omx_jpeg_input_buffer * inputBuffer1,
               omx_jpeg_input_buffer * inputBuffer2,
               omx_jpeg_output_buffer * outputBuffer);

int jpegStop(omx_jpeg_comp *comp);
void errorHandler(omx_jpeg_message_queue * mqueue);
void release(omx_jpeg_comp * comp);
void jpegAbort(omx_jpeg_comp * comp);
int handleHardwareEncodeFailure(omx_jpeg_comp * comp);
void releaseOMXBuffers(omx_jpeg_comp *comp);

static OMX_VERSIONTYPE version = {{1,1,2,0}};
static int cancelPicture =0;
/*==============================================================================
 * FUNCTION    - OMX_GetHandle -
 *
 * DESCRIPTION:
 *============================================================================*/
void get_component_fns(OMX_COMPONENTTYPE *component_fns)
{
  OMX_DBG_INFO("%s: E\n", __func__);
  component_fns->nSize               = sizeof(OMX_COMPONENTTYPE);
  /*component_fns->nVersion.nVersion   = OMX_SPEC_VERSION;*/
  component_fns->pApplicationPrivate = 0;
  /*component_fns->pComponentPrivate   = obj_ptr;*/

  component_fns->AllocateBuffer      = omx_component_image_allocate_buffer;
  component_fns->FreeBuffer          = omx_component_image_free_buffer;
  component_fns->GetParameter        = omx_component_image_get_parameter;
  component_fns->SetParameter        = omx_component_image_set_parameter;
  component_fns->SendCommand         = omx_component_image_send_command;
  component_fns->FillThisBuffer      = omx_component_image_fill_this_buffer;
  component_fns->EmptyThisBuffer     = omx_component_image_empty_this_buffer;
  component_fns->GetState            = omx_component_image_get_state;
  component_fns->GetComponentVersion = omx_component_image_get_version;
  component_fns->GetConfig           = omx_component_image_get_config;
  component_fns->SetConfig           = omx_component_image_set_config;
  component_fns->GetExtensionIndex   = omx_component_image_get_extension_index;
  component_fns->ComponentTunnelRequest = omx_component_image_tunnel_request;
  component_fns->UseBuffer           = omx_component_image_use_buffer;
  component_fns->SetCallbacks        = omx_component_image_set_callbacks;
  component_fns->UseEGLImage         = omx_component_image_use_EGL_image;
  component_fns->ComponentRoleEnum   = omx_component_image_role_enum;
  component_fns->ComponentDeInit     = omx_component_image_deinit;

  local_fns = component_fns;
  OMX_DBG_INFO("%s: X\n", __func__);
}

/*==============================================================================
 * FUNCTION    - omx_component_image_allocate_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_allocate_buffer(OMX_IN OMX_HANDLETYPE hComp,
OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
OMX_IN OMX_U32                        port,
OMX_IN OMX_PTR                     appData,
OMX_IN OMX_U32                       bytes)
{

  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s: E\n", __func__);
  rc = init();
  OMX_DBG_INFO("%s: X\n", __func__);
  return rc;
}

OMX_ERRORTYPE
omx_component_image_free_buffer(OMX_IN OMX_HANDLETYPE hComp,
OMX_IN OMX_U32 port,
OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_HIGH("%s: E\n", __func__);
  OMX_DBG_INFO("%s: port = %d\n", __func__, (int)port);
  omx_jpeg_comp * comp = GET_COMP(hComp);
  omx_jpeg_queue_item item;

  //Need to do a jpegStop before releaseOMXBuffers since encoding
  //cane be in progress in case of abort
  jpegStop(comp);
  releaseOMXBuffers(comp);

  /*check for port disabling, need to handle states*/
  /*if (comp->currentState == OMX_StateIdle && comp->inTransition
          && comp->targetState == OMX_StateLoaded)
  {*/
  OMX_MM_FREE(buffer->pPlatformPrivate);
  OMX_MM_FREE(buffer);
  comp->bufferCount--;
  /*}
  else {
      rc = OMX_ErrorIncorrectStateOperation;
  }*/
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_empty_this_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_empty_this_buffer(OMX_IN OMX_HANDLETYPE hComp,
OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s", __func__);
  /*check state, check version, check buffer
  assuming we got the buffer correctly
  start the encoding here*/
  omx_jpeg_queue_item item;
  omx_jpeg_comp * comp = GET_COMP(hComp);
  pthread_mutex_lock(&comp->mLock);

  item.message = OMX_JPEG_MESSAGE_ETB;
  item.args[0].pValue = buffer->pPlatformPrivate;
  OMX_DBG_INFO("%s: input is %p", __func__, item.args[0].pValue);
  postMessage(comp->queue, OMX_JPEG_QUEUE_ETB, &item);

  pthread_mutex_unlock(&comp->mLock);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_fill_this_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_fill_this_buffer(OMX_IN OMX_HANDLETYPE hComp,
OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  omx_jpeg_queue_item item;
  omx_jpeg_comp * comp = GET_COMP(hComp);
  pthread_mutex_lock(&comp->mLock);
  item.message = OMX_JPEG_MESSAGE_FTB;
  item.args[0].pValue = buffer->pPlatformPrivate;
  postMessage(comp->queue, OMX_JPEG_QUEUE_FTB, &item);
  pthread_mutex_unlock(&comp->mLock);
  OMX_DBG_INFO("%s: E\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_set_callbacks -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_set_callbacks(OMX_IN OMX_HANDLETYPE        hComp,
OMX_IN OMX_CALLBACKTYPE* callbacks,
OMX_IN OMX_PTR             appData)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s:set callback %p",  __func__, callbacks);
  omx_jpeg_comp * comp = GET_COMP(hComp);
  local_callbacks = callbacks;
  local_appData = appData;
  OMX_DBG_INFO("%s: x\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_deinit -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_deinit(OMX_IN OMX_HANDLETYPE hComp)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s: E\n", __func__);
  omx_jpeg_queue_item item;
  omx_jpeg_comp * comp = GET_COMP(hComp);
  deinit(comp);
  return rc;
}



/*==============================================================================
 * FUNCTION    - omx_component_image_get_parameter -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_get_parameter(OMX_IN OMX_HANDLETYPE hComp,
OMX_IN OMX_INDEXTYPE paramIndex,
OMX_INOUT OMX_PTR     paramData)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s", __func__);
  omx_jpeg_comp * comp = GET_COMP(hComp);
  /*stateCheck();
  checkVersion();*/

  if (paramData == NULL)
    return OMX_ErrorBadParameter;
  pthread_mutex_lock(&comp->mLock);

  switch (paramIndex) {
  case OMX_IndexParamImageInit:{
      OMX_PORT_PARAM_TYPE *destType = (OMX_PORT_PARAM_TYPE *)paramData;
      memcpy(paramData, comp->portParam, sizeof(OMX_PORT_PARAM_TYPE));
      break;
    }

  case OMX_IndexParamImagePortFormat:{
      OMX_IMAGE_PARAM_PORTFORMATTYPE *destFormat =
      (OMX_IMAGE_PARAM_PORTFORMATTYPE*)paramData;
      OMX_IMAGE_PARAM_PORTFORMATTYPE * srcFormat =
      ((destFormat->nPortIndex == INPUT_PORT) || (destFormat->nPortIndex ==
                                                  INPUT_PORT1))?
      comp->inputFormatType:comp->outputFormatType;
      memcpy(destFormat, srcFormat, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
      break;
    }

  case OMX_IndexParamPortDefinition:{
      OMX_PARAM_PORTDEFINITIONTYPE *destPort =
      (OMX_PARAM_PORTDEFINITIONTYPE *) paramData;
      OMX_PARAM_PORTDEFINITIONTYPE *srcPort;
      if (destPort->nPortIndex==INPUT_PORT) {
        srcPort = comp->inPort;
      } else if (destPort->nPortIndex==INPUT_PORT1) {
        srcPort = comp->inPort1;
      } else {
        srcPort = comp->outPort;
      }
      memcpy(destPort, srcPort, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
      break;
    }

  case OMX_IndexParamQFactor:{
      OMX_IMAGE_PARAM_QFACTORTYPE* factorType =
      (OMX_IMAGE_PARAM_QFACTORTYPE* )paramData;
      memcpy(factorType, &comp->mainImageQuality,
      sizeof(OMX_IMAGE_PARAM_QFACTORTYPE));
      break;
    }


  case OMX_JPEG_EXT_BUFFER_OFFSET:{
      memcpy(paramData, &comp->offset, sizeof(omx_jpeg_buffer_offset));
      break;
    }

  case OMX_JPEG_EXT_MOBICAT:
    memcpy(paramData, &comp->mobiData, sizeof(omx_jpeg_mobicat));
    break;

  case OMX_JPEG_EXT_ACBCR_OFFSET:{
      memcpy(paramData, &comp->aoffset, sizeof(omx_jpeg_buffer_offset));
      break;
    }

  case OMX_JPEG_EXT_THUMBNAIL_QUALITY:{
      omx_jpeg_thumbnail_quality * qualityFactorType =
      (omx_jpeg_thumbnail_quality* )paramData;
      memcpy(qualityFactorType, &comp->thumbnailQuality,
      sizeof(omx_jpeg_thumbnail_quality));
      break;

    }
  case OMX_JPEG_EXT_USER_PREFERENCES:{
      omx_jpeg_user_preferences *user_preferences =
      (omx_jpeg_user_preferences*)paramData;
      memcpy(user_preferences,&comp->preferences,
      sizeof(omx_jpeg_user_preferences));
      break;
    }

  default:{
      OMX_DBG_ERROR("%s:Get Parameter:Unknown Case",  __func__);
      break;
    }
  }
  pthread_mutex_unlock(&comp->mLock);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_set_parameter -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_set_parameter(OMX_IN OMX_HANDLETYPE     hComp,
OMX_IN OMX_INDEXTYPE paramIndex,
OMX_IN OMX_PTR        paramData)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
//  OMX_DBG_INFO("%s", __func__);
  omx_jpeg_comp * comp = GET_COMP(hComp);
  /*stateCheck();
  checkVersion();*/

  if (paramData == NULL)
    return OMX_ErrorBadParameter;

  pthread_mutex_lock(&comp->mLock);
  switch (paramIndex) {

  case OMX_IndexParamImageInit:{
      OMX_PORT_PARAM_TYPE *destType = (OMX_PORT_PARAM_TYPE *)paramData;
    /*TODO check*/
      break;
    }

  case OMX_IndexParamImagePortFormat:{
    /*TODO check*/
      break;
    }

  case OMX_IndexParamPortDefinition:{
      OMX_PARAM_PORTDEFINITIONTYPE *destPort =
      (OMX_PARAM_PORTDEFINITIONTYPE *) paramData;
    /*TODO more checks
    updating only sizes*/
      if (destPort->nPortIndex == INPUT_PORT) {
        rc = exif_init(&comp->exifInfo);
        comp->inPort->format.image.nFrameWidth =
          destPort->format.image.nFrameWidth;
        comp->inPort->format.image.nFrameHeight =
          destPort->format.image.nFrameHeight;
        comp->inPort->format.image.nStride =
         CEILING16(comp->inPort->format.image.nFrameWidth);
        comp->inPort->format.image.nSliceHeight =
      CEILING16(comp->inPort->format.image.nFrameHeight);
        comp->inPort->nBufferSize = destPort->nBufferSize;
        comp->outPort->nBufferSize = comp->inPort->nBufferSize;
      } else if (destPort->nPortIndex == INPUT_PORT1) {

        comp->inPort1->format.image.nFrameWidth =
          destPort->format.image.nFrameWidth;
        comp->inPort1->format.image.nFrameHeight =
           destPort->format.image.nFrameHeight;
        comp->inPort1->format.image.nStride =
          CEILING16(comp->inPort1->format.image.nFrameWidth);
        comp->inPort1->format.image.nSliceHeight =
           CEILING16(comp->inPort1->format.image.nFrameHeight);
        comp->inPort1->nBufferSize = YUV_SIZER(
        comp->inPort1->format.image.nStride,
        comp->inPort1->format.image.nSliceHeight);
      }
      break;
    }
  case OMX_IndexParamQFactor:{
      OMX_IMAGE_PARAM_QFACTORTYPE* factorType =
      (OMX_IMAGE_PARAM_QFACTORTYPE* )paramData;
      memcpy(&comp->mainImageQuality, factorType,
      sizeof(OMX_IMAGE_PARAM_QFACTORTYPE));
      break;
    }

  case OMX_JPEG_EXT_EXIF:{
      int rc;
      omx_jpeg_exif_info_tag * tag = (omx_jpeg_exif_info_tag *)paramData;
      OMX_DBG_INFO("Calling exif set tag from OMX component\n");
      rc = exif_set_tag(comp->exifInfo, tag->tag_id, &tag->tag_entry);
      ONWARNING(rc);
      break;
    }

  case OMX_JPEG_EXT_THUMBNAIL:{
      memcpy(&comp->thumbnail, paramData, sizeof(comp->thumbnail));
      comp->thumbnailPresent = 1;
      break;
    }

  case OMX_JPEG_EXT_THUMBNAIL_QUALITY:{
      omx_jpeg_thumbnail_quality * qualityFactorType =
      (omx_jpeg_thumbnail_quality* )paramData;
      memcpy(&comp->thumbnailQuality, qualityFactorType,
      sizeof(omx_jpeg_thumbnail_quality));
      break;
    }

  case OMX_JPEG_EXT_BUFFER_OFFSET:{
      omx_jpeg_buffer_offset * offset1 = (omx_jpeg_buffer_offset *)paramData;
      memcpy(&comp->offset, offset1, sizeof(omx_jpeg_buffer_offset));
      break;
    }

  case OMX_JPEG_EXT_MOBICAT: {
    comp->mobicatenable = 1;
    omx_jpeg_mobicat* mb_data = (omx_jpeg_mobicat*)paramData;
    comp->mobiData.mobicatData = (uint8_t*)mb_data->mobicatData;
    comp->mobiData.mobicatDataLength = mb_data->mobicatDataLength;
    break;
  }

  case OMX_JPEG_EXT_ACBCR_OFFSET:{
    omx_jpeg_buffer_offset * offset2 = (omx_jpeg_buffer_offset *)paramData;
    memcpy(&comp->aoffset, offset2, sizeof(omx_jpeg_buffer_offset));
    break;
  }

  case OMX_JPEG_EXT_USER_PREFERENCES:{
      omx_jpeg_user_preferences *user_preferences =
      (omx_jpeg_user_preferences*)paramData;
      memcpy(&comp->preferences,user_preferences,
      sizeof(omx_jpeg_user_preferences));
      break;
    }

  default:{
      OMX_DBG_ERROR("%s:Set Parameter: Unknown Case", __func__);
      break;
    }
  }

  pthread_mutex_unlock(&comp->mLock);
 // OMX_DBG_INFO("%s\n", __func__);

  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_get_config -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_get_config(OMX_IN OMX_HANDLETYPE      hComp,
OMX_IN OMX_INDEXTYPE configIndex,
OMX_INOUT OMX_PTR     configData)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s: E\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_set_config -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_set_config(OMX_IN OMX_HANDLETYPE      hComp,
OMX_IN OMX_INDEXTYPE configIndex,
OMX_IN OMX_PTR        configData)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;

  omx_jpeg_comp * comp = GET_COMP(hComp);
  pthread_mutex_lock(&comp->mLock);
  if (configIndex == OMX_IndexConfigCommonRotate) {
    OMX_CONFIG_ROTATIONTYPE *rotType =
    (OMX_CONFIG_ROTATIONTYPE *)configData;
    if (rotType->nPortIndex == OUTPUT_PORT)
      comp->rotation = rotType->nRotation;
    OMX_DBG_HIGH("%s: Rotation set to %d\n",__func__, comp->rotation);
  }

  if (configIndex == OMX_IndexConfigCommonInputCrop) {
    OMX_CONFIG_RECTTYPE * rectType = (OMX_CONFIG_RECTTYPE*) configData;
    if (rectType->nPortIndex == OUTPUT_PORT) {
      memcpy(&comp->inputCrop, rectType, sizeof(OMX_CONFIG_RECTTYPE));
    }
  }
  if (configIndex == OMX_IndexConfigCommonOutputCrop) {
    OMX_CONFIG_RECTTYPE * rectType = (OMX_CONFIG_RECTTYPE*) configData;
    if (rectType->nPortIndex == OUTPUT_PORT) {
      memcpy(&comp->outputCrop, rectType, sizeof(OMX_CONFIG_RECTTYPE));
    }
  }
  pthread_mutex_unlock(&comp->mLock);
  OMX_DBG_INFO("%s\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_get_state -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_get_state(OMX_IN OMX_HANDLETYPE  hComp,
OMX_OUT OMX_STATETYPE* state)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_send_command -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_send_command(OMX_IN OMX_HANDLETYPE hComp,
OMX_IN OMX_COMMANDTYPE  cmd,
OMX_IN OMX_U32       param1,
OMX_IN OMX_PTR      cmdData)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  omx_jpeg_comp * comp = GET_COMP(hComp);
  omx_jpeg_queue_item item;
  OMX_DBG_INFO("%s: cmd=%d param1=%d\n", __func__, cmd, param1);
  pthread_mutex_lock(&comp->mLock);
  if (cmd == OMX_CommandStateSet) {
    item.message = OMX_JPEG_MESSAGE_CHANGE_STATE;
    item.args[0].iValue = param1;
    postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
  } else if (cmd == OMX_CommandFlush) {
    /*irrespective of type for port flush all equivalent to stopping*/
    cancelPicture =1;
    jpegAbort(comp);
  }
  pthread_mutex_unlock(&comp->mLock);
  OMX_DBG_INFO("%s: X\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_get_version -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_get_version(OMX_IN OMX_HANDLETYPE               hComp,
OMX_OUT OMX_STRING          componentName,
OMX_OUT OMX_VERSIONTYPE* componentVersion,
OMX_OUT OMX_VERSIONTYPE*      specVersion,
OMX_OUT OMX_UUIDTYPE*       componentUUID)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_get_extension_index -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_get_extension_index(OMX_IN OMX_HANDLETYPE      hComp,
OMX_IN OMX_STRING      paramName,
OMX_OUT OMX_INDEXTYPE* indexType)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  int ret = strncmp(paramName, OMX_JPEG_PREFIX, OMX_JPEG_PREFIX_LENGTH);
  int i = 0;

  if (ret)
    return OMX_ErrorBadParameter;

  for (i=OMX_JPEG_EXT_START;i<OMX_JPEG_EXT_END; i++) {
    if (!strncmp(paramName+OMX_JPEG_PREFIX_LENGTH,
    omx_jpeg_ext_name[i-OMX_JPEG_EXT_START],
    strlen(omx_jpeg_ext_name[i-OMX_JPEG_EXT_START]))
    && (strlen(paramName+OMX_JPEG_PREFIX_LENGTH) ==
    strlen(omx_jpeg_ext_name[i-OMX_JPEG_EXT_START]))) {
      *indexType = i;
      return rc;
    }
  }
  OMX_DBG_ERROR("%s:jpeg ext failure -1", __func__);
  rc = OMX_ErrorBadParameter;
  OMX_DBG_INFO("%s\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_tunnel_request -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_tunnel_request(OMX_IN OMX_HANDLETYPE                hComp,
OMX_IN OMX_U32                        port,
OMX_IN OMX_HANDLETYPE        peerComponent,
OMX_IN OMX_U32                    peerPort,
OMX_INOUT OMX_TUNNELSETUPTYPE* tunnelSetup)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_use_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_use_buffer(OMX_IN OMX_HANDLETYPE                hComp,
OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
OMX_IN OMX_U32                        port,
OMX_IN OMX_PTR                     appData,
OMX_IN OMX_U32                       bytes,
OMX_IN OMX_U8*                      buffer)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s", __func__);
  /*not checking if in loaded state as race conditions*/
  omx_jpeg_comp * comp = GET_COMP(hComp);
  pthread_mutex_lock(&comp->mLock);
  if (port == INPUT_PORT || port == INPUT_PORT1) {
    omx_component_image_use_input_buffer(hComp, bufferHdr, port, appData,
    bytes, buffer);
  } else {
    omx_component_image_use_output_buffer(hComp, bufferHdr, port, appData,
    bytes, buffer);
  }
  comp->bufferCount++;
  /*verify to transfer to idle state*/
  if (comp->inPort->bPopulated && comp->outPort->bPopulated) {
    /*Commenting this out due to threading issue.need to handle state changes
      in a better way. Verify transition to idle state - this happens in change
      state
      while(!comp->inTransition && !(comp->targetState == OMX_StateIdle)){
      OMX_DBG_INFO("Waiting for component to be in transition and target
              state to be OMX_StateIdle\n");
      pthread_cond_wait(&comp->cond, &comp->mLock);
      }*/
    omx_jpeg_queue_item item;
    comp->inTransition = 0;
    comp->currentState = OMX_StateIdle;
    item.message = OMX_JPEG_MESSAGE_EVENT;
    item.args[0].iValue = OMX_EventCmdComplete;
    item.args[1].iValue = OMX_CommandStateSet;
    item.args[2].iValue = OMX_StateIdle;
    postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
  }
  pthread_mutex_unlock(&comp->mLock);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_use_EGL_image -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_use_EGL_image(OMX_IN OMX_HANDLETYPE                hComp,
OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
OMX_IN OMX_U32                        port,
OMX_IN OMX_PTR                     appData,
OMX_IN void*                      eglImage)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_use_EGL_image -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_role_enum(OMX_IN OMX_HANDLETYPE hComp,
OMX_OUT OMX_U8*        role,
OMX_IN OMX_U32        index)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s\n", __func__);
  return rc;
}

/*==============================================================================
 * FUNCTION    - jpegInvokeStop -
 *
 * DESCRIPTION:
 *============================================================================*/
void jpegInvokeStop(omx_jpeg_comp * comp)
{
  /*This will be single threaded as will be called by message  thread
  lock protection*/
  omx_jpeg_queue_item item;
  OMX_DBG_INFO("%s :E", __func__);

    if(!cancelPicture){
    pthread_mutex_lock(&comp->abort_mutex);
    OMX_DBG_HIGH("%s:Picture not cancelled, Posting Callback\n",__func__);
    item.message = OMX_JPEG_MESSAGE_ETB_DONE;
    item.args[0].pValue = comp->inputBuffer;
    postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);

    item.message = OMX_JPEG_MESSAGE_FTB_DONE;
    item.args[0].pValue = comp->outputBuffer;
    postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
    pthread_mutex_unlock(&comp->abort_mutex);
    }

  OMX_DBG_INFO("%s :X", __func__);
}

/*==============================================================================
 * FUNCTION    - jpegAbort -
 *
 * DESCRIPTION:
 *============================================================================*/
void jpegAbort(omx_jpeg_comp * comp)
{
  OMX_DBG_HIGH("%s: E", __func__);
  /*single threaded caller*/
  omx_jpeg_queue_item item;

  pthread_mutex_lock(&comp->abort_mutex);

  //Stop the jpeg Engine
  jpegStop(comp);
  /*Release buffers only after Stopping engine since encoding
    may still be in progress during abort */
  releaseOMXBuffers(comp);
 // }

  //Flush the queue - In case abort is called when ETB/FTB done is
  //posted to the Queue and not handled yet
  omx_jpeg_queue_flush(&comp->queue->command);

  /*Actually have to do above for all buffers in the etb and ftb queue*/
  item.message = OMX_JPEG_MESSAGE_EVENT;
  item.args[0].iValue = OMX_EVENT_JPEG_ABORT;
  item.args[1].iValue = 0;
  item.args[2].iValue = 0;
  postMessage(comp->queue, OMX_JPEG_QUEUE_ABORT, &item);

  /* reset calcelPicture flag */
  cancelPicture = 0;
  pthread_mutex_unlock(&comp->abort_mutex);
  OMX_DBG_HIGH("%s: X", __func__);

}

/*==============================================================================
 * FUNCTION    - jpege_event_handler -
 *
 * DESCRIPTION:
 *============================================================================*/
void jpege_event_handler(void *p_user_data, jpeg_event_t event, void *p_arg)
{
  int rc;
  OMX_DBG_INFO("%s:got event %d", __func__, event);
  omx_jpeg_queue_item item;
  omx_jpeg_output_buffer * buffer = (omx_jpeg_output_buffer  *)p_user_data;
  if (event == JPEG_EVENT_THUMBNAIL_DROPPED) {
    item.message = OMX_JPEG_MESSAGE_EVENT;
    item.args[0].iValue = OMX_EventError;
    item.args[1].iValue = OMX_EVENT_THUMBNAIL_DROPPED;
    item.args[2].iValue = 0;
    postMessage(buffer->comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
  } else if (event == JPEG_EVENT_ERROR) {
    OMX_DBG_ERROR("%s:got event JPEG_EVENT_ERROR", __func__);
    if((buffer->comp->jpege_config.preference == OMX_JPEG_PREF_HW_ACCELERATED_ONLY)||
       (buffer->comp->jpege_config.preference == OMX_JPEG_PREF_HW_ACCELERATED_PREFERRED)) {
         rc = handleHardwareEncodeFailure(buffer->comp);
         if(!rc){
            return;
          }
     }
    item.message = OMX_JPEG_MESSAGE_EVENT;
    item.args[0].iValue = OMX_EventError;
    item.args[1].iValue = OMX_EVENT_JPEG_ERROR;
    item.args[2].iValue = 0;
    postMessage(buffer->comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
  } else {
    if (event == JPEG_EVENT_DONE) {
      OMX_DBG_HIGH("%s:got event JPEG_EVENT_DONE", __func__);
    }
    jpegInvokeStop(buffer->comp);
  }
  return;

}
/*==============================================================================
 * FUNCTION    - handleHardwareEncodeFailure -
 *
 * DESCRIPTION:
 *============================================================================*/

int handleHardwareEncodeFailure(omx_jpeg_comp * comp){
   int rc =0;
   FILE *fout;
   uint8_t* buf_ptr;
   jpeg_buffer_t jpege_dest_buffer[2];
   OMX_DBG_HIGH("%s:E", __func__);

   fout = fopen("/data/test_buffer.yuv", "wb");
   if (fout)
   {//fwrite(comp->inputBuffer->addr, 1, comp->inputBuffer->length, fout);

        jpeg_buffer_get_addr(comp->inputBuffer->main_img_info.p_fragments[0].color.yuv.luma_buf, &buf_ptr);
        fwrite(buf_ptr, 1,comp->inputBuffer->main_img_info.width * comp->inputBuffer->main_img_info.height , fout);
        jpeg_buffer_get_addr(comp->inputBuffer->main_img_info.p_fragments[0].color.yuv.chroma_buf, &buf_ptr);
        fwrite(buf_ptr, 1, comp->inputBuffer->main_img_info.width * comp->inputBuffer->main_img_info.height/2, fout);
        fclose(fout);
   }
    if (!cancelPicture) {
     pthread_mutex_lock(&comp->abort_mutex);
     rc = jpege_abort(comp->jpeg_encoder);
     ONWARNING(rc);
     jpege_destroy(&comp->jpeg_encoder);
     pthread_mutex_lock(&jpegStop_mutex);
     comp->isJpegEngineActive = 0;
     pthread_mutex_unlock(&jpegStop_mutex);

    jpeg_buffer_destroy(&comp->outputBuffer->jpege_dest_buffer[0]);
    jpeg_buffer_destroy(&comp->outputBuffer->jpege_dest_buffer[1]);

    comp->jpege_config.preference = OMX_JPEG_PREF_SOFTWARE_ONLY;

    rc = jpege_init(&comp->jpeg_encoder, jpege_event_handler,
         comp->outputBuffer);
    if(rc)
       pthread_mutex_unlock(&comp->abort_mutex);
    ONERROR(rc, errorHandler(comp->queue));

   /*comp->inputBuffer->jpege_source.p_thumbnail = &comp->inputBuffer->tn_img_info;
   comp->inputBuffer->jpege_source.p_main = &comp->inputBuffer->main_img_info;*/
   OMX_DBG_INFO("%s:jpege_source %p\n",__func__,&comp->inputBuffer->jpege_source);
   rc = jpege_set_source(comp->jpeg_encoder, &comp->inputBuffer->jpege_source);
   if(rc)
       pthread_mutex_unlock(&comp->abort_mutex);
   ONERROR(rc, errorHandler(comp->queue));

   rc = jpeg_buffer_init(&jpege_dest_buffer[0]);
   if(rc)
       pthread_mutex_unlock(&comp->abort_mutex);
   ONERROR(rc, errorHandler(comp->queue));
   rc = jpeg_buffer_init(&jpege_dest_buffer[1]);
   if(rc)
       pthread_mutex_unlock(&comp->abort_mutex);
   ONERROR(rc, errorHandler(comp->queue));

   rc = jpeg_buffer_allocate(jpege_dest_buffer[0],
   JPEGE_FRAGMENT_SIZE, 0);
   if(rc)
       pthread_mutex_unlock(&comp->abort_mutex);
   ONERROR(rc, errorHandler(comp->queue));
   rc = jpeg_buffer_allocate(jpege_dest_buffer[1],
   JPEGE_FRAGMENT_SIZE, 0);
   if(rc)
       pthread_mutex_unlock(&comp->abort_mutex);
   ONERROR(rc, errorHandler(comp->queue));

   #ifndef USE_JPEG1_LIB
   comp->outputBuffer->jpege_dest.p_buffer = &jpege_dest_buffer[0];
   #else
   int i = 0;
   for (i=0; i<2; i++){
      comp->outputBuffer->jpege_dest.buffers[i] = jpege_dest_buffer[i];
   }
  #endif

   rc = jpege_set_destination(comp->jpeg_encoder, &comp->outputBuffer->jpege_dest);
   if(rc)
       pthread_mutex_unlock(&comp->abort_mutex);
   ONERROR(rc, errorHandler(comp->queue));

    rc = jpege_start(comp->jpeg_encoder, &comp->jpege_config, &comp->exifInfo);
    OMX_DBG_INFO("%s:jpege_start rc=%d", __func__, rc);
    if(rc)
       pthread_mutex_unlock(&comp->abort_mutex);
    ONERROR(rc, errorHandler(comp->queue));

    pthread_mutex_lock(&jpegStop_mutex);
    comp->isJpegEngineActive = 1;
    pthread_mutex_unlock(&jpegStop_mutex);

    pthread_mutex_unlock(&comp->abort_mutex);
  }
   OMX_DBG_INFO("%s:X", __func__);

   return rc;
}

#ifdef USE_JPEG1_LIB
/*==============================================================================
 * FUNCTION    - jpege_output_produced_handler2 -
 *
 * DESCRIPTION:
 *============================================================================*/
void jpege_output_produced_handler2(void *p_user_data, void *p_arg,
jpeg_buffer_t buffer)
{
  uint32_t buf_size;
  uint8_t *buf_ptr;
  omx_jpeg_output_buffer * omxBuffer;
  int rv;

  OMX_DBG_INFO("%s", __func__);
  jpeg_buffer_get_actual_size(buffer, &buf_size);
  jpeg_buffer_get_addr(buffer, &buf_ptr);
  omxBuffer = (omx_jpeg_output_buffer *)p_arg;
  memcpy(omxBuffer->outputHeader->pBuffer +
  omxBuffer->outputHeader->nFilledLen,
  buf_ptr, buf_size);
  omxBuffer->outputHeader->nFilledLen += buf_size;

  jpeg_buffer_set_actual_size(buffer, 0);
}
#else
/*==============================================================================
 * FUNCTION    - jpege_output_produced_handler -
 *
 * DESCRIPTION:
 *============================================================================*/
int jpege_output_produced_handler(void *p_user_data, void *p_arg,
jpeg_buffer_t buffer, uint8_t last_buf_flag)
{
  uint32_t buf_size;
  uint8_t *buf_ptr;
  omx_jpeg_output_buffer * omxBuffer;
  int rv;

  OMX_DBG_INFO("%s:E", __func__);
  jpeg_buffer_get_actual_size(buffer, &buf_size);
  jpeg_buffer_get_addr(buffer, &buf_ptr);
  omxBuffer = (omx_jpeg_output_buffer *)p_arg;
  pthread_mutex_lock(&jpegoutput_mutex);
  if ((omxBuffer != NULL) && (omxBuffer->outputHeader != NULL) &&
     (omxBuffer->outputHeader->pBuffer != NULL) &&
     (buf_ptr!= NULL) && buf_size > 0) {
    memcpy(omxBuffer->outputHeader->pBuffer +
           omxBuffer->outputHeader->nFilledLen,
           buf_ptr, buf_size);
    omxBuffer->outputHeader->nFilledLen += buf_size;
    pthread_mutex_unlock(&jpegoutput_mutex);
  } else {
    /*incase the output buffer happens to be null*/
    OMX_DBG_ERROR("%s:output buffer is NULL",__func__);
    pthread_mutex_unlock(&jpegoutput_mutex);
    return -1;
  }


  jpeg_buffer_set_actual_size(buffer, 0);
  rv = jpege_enqueue_output_buffer(
  omxBuffer->comp->jpeg_encoder,
  &buffer, 1);

  OMX_DBG_INFO("%s: X", __func__);

  return rv;
}
#endif

/*==============================================================================
 * FUNCTION    - initInport -
 *
 * DESCRIPTION:
 *============================================================================*/
void initInport(omx_jpeg_comp * comp)
{
  OMX_DBG_INFO("%s", __func__);
  comp->inPort = OMX_MM_MALLOC(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  if(!comp->inPort) {
    OMX_DBG_ERROR("%s: comp->inPort malloc failed",__func__);
    return;
  }
  OMX_MM_ZERO(comp->inPort, OMX_PARAM_PORTDEFINITIONTYPE);
  comp->inPort->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  comp->inPort->nVersion = version;
  comp->inPort->nPortIndex = INPUT_PORT;
  comp->inPort->eDir = OMX_DirInput;
  comp->inPort->nBufferCountActual = 1;
  comp->inPort->nBufferCountMin = 1;
  comp->inPort->bEnabled = OMX_TRUE;
  comp->inPort->bPopulated = OMX_FALSE;
  comp->inPort->eDomain = OMX_PortDomainImage;
  comp->inPort->bBuffersContiguous = OMX_TRUE;
  /*make this get page size*/
  comp->inPort->nBufferAlignment = 4096;

  /*setting to null ycbcr mime type */
  comp->inPort->format.image.cMIMEType = NULL;
  /*native renderer defaulted to zero*/
  comp->inPort->format.image.nFrameWidth = DEFAULT_PICTURE_WIDTH;
  comp->inPort->format.image.nFrameHeight = DEFAULT_PICTURE_HEIGHT;
  /* use stride width for gemini constraints*/
  comp->inPort->format.image.nStride = CEILING16(DEFAULT_PICTURE_WIDTH);
  comp->inPort->format.image.nSliceHeight = CEILING16(DEFAULT_PICTURE_HEIGHT);
  comp->inPort->format.image.bFlagErrorConcealment = OMX_FALSE;
  comp->inPort->format.image.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  comp->inPort->format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;

  comp->inPort->nBufferSize = comp->offset.totalSize;
  /*Newly added code for thumbnail port*/
  comp->inPort1 = OMX_MM_MALLOC(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  if(!comp->inPort1) {
    OMX_DBG_ERROR("%s: comp->inPort1 malloc failed",__func__);
    return;
  }
  OMX_MM_ZERO(comp->inPort1, OMX_PARAM_PORTDEFINITIONTYPE);
  comp->inPort1->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  comp->inPort1->nVersion = version;
  comp->inPort1->nPortIndex = INPUT_PORT1;
  comp->inPort1->eDir = OMX_DirInput;
  comp->inPort1->nBufferCountActual = 1;
  comp->inPort1->nBufferCountMin =  1;
  comp->inPort1->bEnabled = OMX_TRUE;
  comp->inPort1->bPopulated = OMX_FALSE;
  comp->inPort1->eDomain = OMX_PortDomainImage;
  comp->inPort1->bBuffersContiguous = OMX_TRUE;
  /*make this get page size*/
  comp->inPort1->nBufferAlignment = 4096;
  OMX_DBG_ERROR("%s,%d\n",__func__,__LINE__);
  /*setting to null ycbcr mime type */
  comp->inPort1->format.image.cMIMEType = NULL;
  /*native renderer defaulted to zero*/
  comp->inPort1->format.image.nFrameWidth = DEFAULT_PICTURE_WIDTH;
  comp->inPort1->format.image.nFrameHeight = DEFAULT_PICTURE_HEIGHT;
  /* use stride width for gemini constraints*/
  comp->inPort1->format.image.nStride = CEILING16(DEFAULT_PICTURE_WIDTH);
  comp->inPort1->format.image.nSliceHeight = CEILING16(DEFAULT_PICTURE_HEIGHT);
  OMX_DBG_ERROR("%s,%d\n",__func__,__LINE__);
  comp->inPort1->format.image.bFlagErrorConcealment = OMX_FALSE;
  comp->inPort1->format.image.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  comp->inPort1->format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;

  comp->inPort1->nBufferSize = comp->offset.totalSize;
  OMX_DBG_INFO("%s: XX\n", __func__);

}

/*==============================================================================
 * FUNCTION    - initOutport -
 *
 * DESCRIPTION:
 *============================================================================*/
void initOutport(omx_jpeg_comp * comp)
{
  OMX_DBG_INFO("%s", __func__);
  comp->outPort = OMX_MM_MALLOC(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  if(!comp->outPort) {
    OMX_DBG_ERROR("%s: comp->outPort malloc failed",__func__);
    return;
  }
  OMX_MM_ZERO(comp->outPort, OMX_PARAM_PORTDEFINITIONTYPE);
  comp->outPort->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  comp->outPort->nVersion = version;
  comp->outPort->nPortIndex = OUTPUT_PORT;
  comp->outPort->eDir = OMX_DirOutput;
  comp->outPort->nBufferCountActual = 1;
  comp->outPort->nBufferCountMin = 1;
  comp->outPort->bEnabled = OMX_TRUE;
  comp->outPort->bPopulated = OMX_FALSE;
  comp->outPort->eDomain = OMX_PortDomainImage;
  comp->outPort->bBuffersContiguous = OMX_TRUE;
  /*make this get page size*/
  comp->outPort->nBufferAlignment = 4096;

  /*setting to null ycbcr mime type */
  comp->outPort->format.image.cMIMEType = "jpeg";
  /*native renderer defaulted to zero*/
  comp->outPort->format.image.nFrameWidth = 0;
  comp->outPort->format.image.nFrameHeight = 0;
  /* use stride width for gemini constraints*/
  /*Setting as output port image width and height*/
  comp->outPort->format.image.nStride = 0;
  comp->outPort->format.image.nSliceHeight = 0;
  comp->outPort->format.image.bFlagErrorConcealment = OMX_FALSE;
  comp->outPort->format.image.eColorFormat = OMX_COLOR_FormatUnused;
  comp->outPort->format.image.eCompressionFormat = OMX_IMAGE_CodingEXIF;

  /*calculate size FIXME gemini worst case for jpeg can do better*/
  /*using from inport as not setting outport values*/
  comp->outPort->nBufferSize = comp->inPort->nBufferSize;
}

/*==============================================================================
 * FUNCTION    - changeState -
 *
 * DESCRIPTION:
 *============================================================================*/
void changeState(omx_jpeg_comp * comp, OMX_STATETYPE state)
{
  OMX_DBG_INFO("%s:from %d to %d", __func__,  comp->currentState, state);
  /*loaded to idle*/
  if (comp->currentState == OMX_StateLoaded) {
    /*try to move to state*/
    if (state == OMX_StateIdle) {
      comp->inTransition = 1;
      comp->targetState = OMX_StateIdle;
      return;
    } else {
      /*add state for waiting for resources
      throw an error*/
      omx_jpeg_queue_item item;
      item.message = OMX_JPEG_MESSAGE_EVENT;
      item.args[0].iValue = OMX_EventError;
      item.args[1].iValue = OMX_ErrorIncorrectStateTransition;
      item.args[2].iValue = 0;
      postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
      return;
    }
    return;
  }

  if (comp->currentState == OMX_StateIdle) {
    omx_jpeg_queue_item item;
    /*need to take care of suspend state*/
    if (state == OMX_StateExecuting) {
      OMX_DBG_INFO("%s:in idle to exe",  __func__);
      /*change and give callbacks*/
      comp->currentState = OMX_StateExecuting;
      item.message = OMX_JPEG_MESSAGE_EVENT;
      item.args[0].iValue = OMX_EventCmdComplete;
      item.args[1].iValue = OMX_CommandStateSet;
      item.args[2].iValue = OMX_StateExecuting;
      postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
      return;
    } else if (state == OMX_StateLoaded) {
      comp->targetState = OMX_StateLoaded;
      comp->inTransition = 1;
      return;
    } else {
      item.message = OMX_JPEG_MESSAGE_EVENT;
      item.args[0].iValue = OMX_EventError;
      item.args[1].iValue = OMX_ErrorIncorrectStateTransition;
      item.args[2].iValue = 0;
      postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
      return;
    }
    return;
  }

  if (comp->currentState == OMX_StateExecuting) {
    omx_jpeg_queue_item item;
    if (state == OMX_StateIdle) {
      OMX_DBG_INFO("%s:Commented jpegInvokeStop", __func__);
      //jpegInvokeStop(comp);
      comp->inTransition = 1;
      comp->targetState = OMX_StateIdle;
      item.message = OMX_JPEG_MESSAGE_TRANSACT_COMPLETE;
      postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
    }
  }
}

/*==============================================================================
 * FUNCTION    - processCommand -
 *
 * DESCRIPTION:
 *============================================================================*/
void processCommand(omx_jpeg_comp* comp, omx_jpeg_queue_item * item)
{
  OMX_DBG_INFO("%s: Message type=%d\n", __func__, item->message);
  switch (item->message) {
  case OMX_JPEG_MESSAGE_STOP:{
      pthread_mutex_lock(&comp->queue->lock);
      comp->queue->initialized = 0;
      pthread_mutex_unlock(&comp->queue->lock);
      break;
    }

  case OMX_JPEG_MESSAGE_START_ENCODE:{
      jpegEncode(comp,
      (omx_jpeg_input_buffer*)item->args[0].pValue,
      (omx_jpeg_input_buffer*)item->args[1].pValue,
      (omx_jpeg_output_buffer*)item->args[2].pValue);
      break;
    }

  case OMX_JPEG_MESSAGE_ETB_DONE:{
      omx_jpeg_input_buffer * buffer =
      (omx_jpeg_input_buffer *)item->args[0].pValue;
      if (!buffer->etbDone) {
        comp->callback->EmptyBufferDone(comp->omxComponent,
        comp->callbackAppData,
        buffer->inputHeader);
        buffer->etbDone = 1;
      }
      break;
    }

  case OMX_JPEG_MESSAGE_FTB_DONE:{
      omx_jpeg_output_buffer * buffer =
      (omx_jpeg_output_buffer *)item->args[0].pValue;
      comp->callback->FillBufferDone(comp->omxComponent,
        comp->callbackAppData,
        buffer->outputHeader);
      break;
    }

  case OMX_JPEG_MESSAGE_CHANGE_STATE:{
      changeState(comp, item->args[0].iValue);
      break;
    }


  case OMX_JPEG_MESSAGE_EVENT:{
      if (comp->callback && comp->callback->EventHandler) {
        comp->callback->EventHandler(comp->omxComponent, comp->callbackAppData,
          (OMX_EVENTTYPE)item->args[0].iValue, item->args[1].iValue,
          item->args[2].iValue, NULL);
      } else {
        OMX_DBG_INFO("%s:OMX_JPEG_MESSAGE_EVENT: NULL", __func__);
      }
      break;
    }

  case OMX_JPEG_MESSAGE_TRANSACT_COMPLETE:{
      comp->inTransition = 0;
      comp->currentState = comp->targetState;
      comp->callback->EventHandler(comp->omxComponent, comp->callbackAppData,
        OMX_EventCmdComplete, OMX_CommandStateSet, comp->currentState, NULL);
      break;
    }

  case OMX_JPEG_MESSAGE_DEINIT: {
      OMX_DBG_INFO("%s:OMX_JPEG_MESSAGE_DEINIT\n", __func__);
      break;
    }

  case OMX_JPEG_MESSAGE_ETB:
  case OMX_JPEG_MESSAGE_FTB:
  case OMX_JPEG_MESSAGE_INITIAL:
  case OMX_JPEG_MESSAGE_FLUSH_COMPLETE:
  case OMX_JPEG_MESSAGE_START_DECODE:
  case OMX_JPEG_MESSAGE_FLUSH:
  case OMX_JPEG_MESSAGE_DECODED_IMAGE:
  case OMX_JPEG_MESSAGE_DECODE_DONE:
    break;
  }
}

/*==============================================================================
 * FUNCTION    - handleMessage -
 *
 * DESCRIPTION:
 *============================================================================*/
void* handleMessage(void *arg){
  omx_jpeg_comp * comp = (omx_jpeg_comp *)arg;
  void * element = NULL;
  omx_jpeg_message_queue * mqueue = comp->queue;
  int ret = 0;
  int commandAvaliable = 0;

  while (true) {
    commandAvaliable = 0;
    pthread_mutex_lock(&mqueue->lock);

    while (mqueue->messageCount == 0 && mqueue->initialized) {
      OMX_DBG_INFO("%s:waiting for message\n", __func__);
      pthread_cond_wait(&mqueue->cond, &mqueue->lock);
      OMX_DBG_INFO("%s:got message\n", __func__);
    }

    /*there can be non zero message count but no actual etbs or ftbs
    because flush cleared it*/
    if (mqueue->abort.size) {
      ret = omx_jpeg_queue_remove(&mqueue->abort, &getItem);
      commandAvaliable = 1;
      OMX_DBG_INFO("%s:read message from abort queue %d\n",
        __func__, getItem.message);
    }else if (mqueue->command.size) {
      ret = omx_jpeg_queue_remove(&mqueue->command, &getItem);
      commandAvaliable = 1;
      OMX_DBG_INFO("%s:read message from command queue %d\n",
        __func__, getItem.message);
    } else {
      OMX_DBG_INFO("%s:etb.size %d ftb.size %d comp->encoding %d\n", __func__,
        mqueue->etb.size, mqueue->ftb.size, comp->encoding);
      /* add state stuff */
      if (mqueue->etb.size && mqueue->ftb.size && !comp->encoding) {
        getItem.message = OMX_JPEG_MESSAGE_START_ENCODE;
        /*remove item from etb*/

        ret = omx_jpeg_queue_remove(&mqueue->etb, &tempItem);
        OMX_DBG_INFO("%s:return value after input remove %d %d ", __func__,
        ret, tempItem.message);
        getItem.args[0].pValue = tempItem.args[0].pValue;

        ret = omx_jpeg_queue_remove(&mqueue->etb, &tempItem);
        OMX_DBG_INFO("%s:return value after input remove %d %d",__func__,
          ret, tempItem.message);
        getItem.args[1].pValue = tempItem.args[0].pValue;

        ret = omx_jpeg_queue_remove(&mqueue->ftb, &tempItem);
        OMX_DBG_INFO("%s:return value after output remove %d %d", __func__,
        ret, tempItem.message);
        getItem.args[2].pValue = tempItem.args[0].pValue;
        commandAvaliable = 1;
      }
    }
    OMX_DBG_INFO("%s:message=%d commandAvaliable %d", __func__,
      getItem.message, commandAvaliable);
    mqueue->messageCount--;
    pthread_mutex_unlock(&mqueue->lock);
    if (commandAvaliable)
      processCommand(comp, &getItem);
    /*Break out of wait*/
    if (mqueue->initialized == 0)
      break;
  }
  /*This is the last function to be called to release resources*/
  release(comp);
  return NULL;
}

/*==============================================================================
 * FUNCTION    - omx_jpeg_queue_insert -
 *
 * DESCRIPTION:
 *============================================================================*/
int omx_jpeg_queue_insert(omx_jpeg_queue* queue, omx_jpeg_queue_item * item)
{
  OMX_DBG_INFO("%s:queue size=%d\n", __func__, queue->size);
  if (queue->size==OMX_JPEG_QUEUE_CAPACITY) {
    OMX_DBG_ERROR("%s:Queue is full\n", __func__);
    return -1;
  }
  memcpy(&queue->container[queue->back], item,
  sizeof(omx_jpeg_queue_item));
  queue->back = (queue->back +1)%OMX_JPEG_QUEUE_CAPACITY;
  queue->size++;
  return 0;
}

/*==============================================================================
 * FUNCTION    - omx_jpeg_queue_insert -
 *
 * DESCRIPTION:
 *============================================================================*/
int omx_jpeg_queue_remove(omx_jpeg_queue * queue, omx_jpeg_queue_item* item){
  if (queue->size == 0) {
    return -1;
  }
  queue->front = (queue->front+1)%OMX_JPEG_QUEUE_CAPACITY;
  memcpy(item, &queue->container[queue->front],
  sizeof(omx_jpeg_queue_item));

  queue->size--;
  return 0;
}

/*==============================================================================
 * FUNCTION    - postMessage -
 *
 * DESCRIPTION:
 *============================================================================*/
int postMessage(omx_jpeg_message_queue * mqueue, omx_jpeg_queue_type type,
omx_jpeg_queue_item * item){
  int ret = 0;
  OMX_DBG_INFO("%s: type=%d, message=%d\n", __func__, type, item->message);
  pthread_mutex_lock(&mqueue->lock);
  switch (type) {
  case OMX_JPEG_QUEUE_COMMAND:
    ret = omx_jpeg_queue_insert(&mqueue->command, item);
    break;
  case OMX_JPEG_QUEUE_ETB:
    ret = omx_jpeg_queue_insert(&mqueue->etb, item);
    break;
  case OMX_JPEG_QUEUE_FTB:
    ret = omx_jpeg_queue_insert(&mqueue->ftb, item);
    break;
  case OMX_JPEG_QUEUE_ABORT:
    ret = omx_jpeg_queue_insert(&mqueue->abort, item);
    break;
  }

  mqueue->messageCount++;
  OMX_DBG_INFO("%s: mqueue->messageCount=%d\n", __func__, mqueue->messageCount);
  pthread_cond_signal(&mqueue->cond);
  pthread_mutex_unlock(&mqueue->lock);
  OMX_DBG_INFO("%s:X\n", __func__);
  return ret;
}

/*==============================================================================
 * FUNCTION    - errorHandler -
 *
 * DESCRIPTION:
 *============================================================================*/
void errorHandler(omx_jpeg_message_queue * mqueue){
    OMX_DBG_ERROR("%s: OMX Internal Error \n",__func__);
    omx_jpeg_queue_item item;

    //ncoding Cannot proceed. Return error
    item.message = OMX_JPEG_MESSAGE_EVENT;
    item.args[0].iValue = OMX_EventError;
    item.args[1].iValue = OMX_EVENT_JPEG_ERROR;
    item.args[2].iValue = 0;
    postMessage(mqueue, OMX_JPEG_QUEUE_COMMAND, &item);

  return;
}

/*==============================================================================
 * FUNCTION    - init -
 *
 * DESCRIPTION:
 *============================================================================*/
int init(void)
{
  int rc;
  OMX_DBG_INFO("%s", __func__);
  omx_jpeg_comp * comp = OMX_MM_MALLOC(sizeof(omx_jpeg_comp));
  if(!comp){
    OMX_DBG_ERROR("%s Cannot allocate OMX component!\n",__func__);
    return  OMX_ErrorInsufficientResources;
  }
  OMX_MM_ZERO(comp, omx_jpeg_comp);
  local_fns->pComponentPrivate = (OMX_PTR)comp;
  comp->omxComponent = local_fns;
  comp->callback = local_callbacks;
  comp->callbackAppData = local_appData;
  comp->portParam = OMX_MM_MALLOC(sizeof(OMX_PORT_PARAM_TYPE));
  if(!comp->portParam){
    OMX_DBG_ERROR("%s Cannot allocate OMX Port param!\n",__func__);
    return -1;
  }
  comp->portParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
  comp->portParam->nVersion = version;
  comp->portParam->nPorts = 3;
  comp->portParam->nStartPortNumber = INPUT_PORT;

  initInport(comp);
  initOutport(comp);

  comp->inputFormatType =
  OMX_MM_MALLOC(sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
  if(!comp->inputFormatType){
    OMX_DBG_ERROR("%s Cannot allocate OMX Input Port Format!\n",__func__);
    return -1;
  }

  OMX_MM_ZERO(comp->inputFormatType, OMX_IMAGE_PARAM_PORTFORMATTYPE);
  comp->inputFormatType->nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
  comp->inputFormatType->nVersion = version;
  comp->inputFormatType->nPortIndex = INPUT_PORT;
  comp->inputFormatType->nIndex = 0;
  comp->inputFormatType->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  comp->inputFormatType->eCompressionFormat = OMX_IMAGE_CodingUnused;

  comp->outputFormatType =
  OMX_MM_MALLOC(sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
  if(!comp->outputFormatType){
     OMX_DBG_ERROR("%s Cannot allocate OMX Image Param Port Format!\n",__func__);
     return -1;
  }
  OMX_MM_ZERO(comp->outputFormatType, OMX_IMAGE_PARAM_PORTFORMATTYPE);
  comp->outputFormatType->nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
  comp->outputFormatType->nVersion = version;
  comp->outputFormatType->eColorFormat = OMX_COLOR_FormatUnused;
  comp->outputFormatType->eCompressionFormat = OMX_IMAGE_CodingEXIF;

  comp->mainImageQuality.nPortIndex = INPUT_PORT;
  comp->mainImageQuality.nQFactor = DEFAULT_MAIN_QUALITY;

  comp->thumbnailQuality.nQFactor = DEFAULT_THUMBNAIL_QUALITY;

  comp->preferences.color_format = DEFAULT_COLOR_FORMAT;
  comp->preferences.thumbnail_color_format = DEFAULT_COLOR_FORMAT;
  comp->preferences.preference = OMX_JPEG_PREF_HW_ACCELERATED_PREFERRED;
  cancelPicture =0;
  comp->encoding=0;
  comp->isJpegEngineActive =0;
  comp->mobicatenable = 0;

  pthread_mutex_init(&comp->mLock, NULL);
  pthread_mutex_init(&comp->abort_mutex, NULL);

  comp->queue = OMX_MM_MALLOC(sizeof(omx_jpeg_message_queue));
  if(!comp->queue){
     OMX_DBG_ERROR("%s: Cannot allocate message queue\n",__func__);
     return -1;
  }
  omx_jpeg_message_queue_init(comp->queue);
  rc = pthread_create(&comp->messageThread, NULL,
  handleMessage, comp);
  ONERROR(rc, errorHandler(comp->queue));


  ONERROR(rc, errorHandler(comp->queue));
  comp->currentState = OMX_StateLoaded;
  comp->num_inputs_freed = 0;
  OMX_DBG_INFO("%s:omx state loaded %d", __func__, comp->currentState);
  return 0;
}

/*==============================================================================
 * FUNCTION    - deinit -
 *
 * DESCRIPTION:
 *============================================================================*/
void deinit(omx_jpeg_comp * comp)
{
  OMX_DBG_HIGH("%s:E", __func__);
  jpegStop(comp);
   /*Need to add this check here to handle the case where abort
     is called in the middle of jpegEncode and encoding flag is
     not set. But jpegEncode actually completes and buffers are
     allocated */
  releaseOMXBuffers(comp);
  pthread_mutex_lock(&comp->queue->lock);
  comp->queue->initialized = 0;
  pthread_cond_signal(&comp->queue->cond);
  pthread_mutex_unlock(&comp->queue->lock);

  OMX_DBG_HIGH("%s:X", __func__);
}

/*==============================================================================
 * FUNCTION    - release -
 *
 * DESCRIPTION:
 *============================================================================*/
void release(omx_jpeg_comp * comp)
{
  OMX_DBG_ERROR("%s:E", __func__);

  pthread_mutex_lock(&comp->lock);
  pthread_cond_destroy(&comp->cond);
  pthread_mutex_destroy(&comp->lock);

  pthread_mutex_lock(&comp->mLock);
  pthread_mutex_destroy(&comp->mLock);

  pthread_mutex_lock(&comp->abort_mutex);
  pthread_mutex_destroy(&comp->abort_mutex);

  if (comp->thumbnailPresent)
     comp->thumbnailPresent = 0;



 //Acquire lock and delete
  pthread_mutex_lock(&comp->queue->lock);
  pthread_cond_destroy(&comp->queue->cond);
  pthread_mutex_destroy(&comp->queue->lock);

  OMX_MM_FREE(comp->queue);
  OMX_MM_FREE(comp->inPort);
  OMX_MM_FREE(comp->inPort1);
  OMX_MM_FREE(comp->outPort);
  OMX_MM_FREE(comp->inputFormatType);
  OMX_MM_FREE(comp->outputFormatType);
  OMX_MM_FREE(comp->portParam);
  comp->omxComponent->pComponentPrivate = NULL;
  OMX_MM_FREE(comp);
//  jpeg_show_leak();
  OMX_DBG_ERROR("%s:X", __func__);
}

/*==============================================================================
 * FUNCTION    - omx_component_image_use_input_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_use_input_buffer(OMX_IN OMX_HANDLETYPE hComp,
OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
OMX_IN OMX_U32                        port,
OMX_IN OMX_PTR                     appData,
OMX_IN OMX_U32                       bytes,
OMX_IN OMX_U8*                      pBuffer){

  OMX_ERRORTYPE rc = OMX_ErrorNone;
  OMX_DBG_INFO("%s", __func__);

  omx_jpeg_comp * comp = GET_COMP(hComp);
  omx_jpeg_pmem_info * info = (omx_jpeg_pmem_info *)appData;
  OMX_BUFFERHEADERTYPE * bufferHeader = OMX_MM_MALLOC(
  sizeof(OMX_BUFFERHEADERTYPE));
  if(!bufferHeader)
    return  OMX_ErrorInsufficientResources;
  OMX_MM_ZERO(bufferHeader, OMX_BUFFERHEADERTYPE);
  int cbcroffset = 0;
  /*create input buffer*/
  omx_jpeg_input_buffer * buffer =
  OMX_MM_MALLOC(sizeof(omx_jpeg_input_buffer));
  if(!buffer) {
    OMX_MM_FREE(bufferHeader);
    return  OMX_ErrorInsufficientResources;
  }
  OMX_MM_ZERO(buffer, omx_jpeg_input_buffer);

  *ppBufferHdr = bufferHeader;
  bufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);
  bufferHeader->nVersion = version;
  bufferHeader->pBuffer = pBuffer;
  bufferHeader->nAllocLen = bytes;
  OMX_DBG_INFO("%s:bytes= %dX", __func__,bytes);
  bufferHeader->nFilledLen = 0;
  /*not checking for offset TODO*/
  bufferHeader->nOffset = info->offset;
  bufferHeader->pAppPrivate = appData;
  bufferHeader->pInputPortPrivate = comp->inPort;
  bufferHeader->pPlatformPrivate = buffer;
  /*TODO other flags need to be checked*/

  buffer->comp = comp;
  buffer->inputHeader = bufferHeader;
  buffer->fd = info->fd;
  buffer->offset = info->offset;
  buffer->length = bytes;
  buffer->addr = pBuffer;

  /*input port populated*/
  comp->inPort->bPopulated = OMX_TRUE;
  comp->inPort1->bPopulated = OMX_TRUE;


  return rc;
}

/*==============================================================================
 * FUNCTION    - omx_component_image_use_output_buffer -
 *
 * DESCRIPTION:
 *============================================================================*/
OMX_ERRORTYPE
omx_component_image_use_output_buffer(OMX_IN OMX_HANDLETYPE hComp,
OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
OMX_IN OMX_U32                        port,
OMX_IN OMX_PTR                     appData,
OMX_IN OMX_U32                       bytes,
OMX_IN OMX_U8*                      pBuffer){

  OMX_DBG_INFO("%s", __func__);
  OMX_ERRORTYPE rc = OMX_ErrorNone;

  omx_jpeg_comp * comp = GET_COMP(hComp);
  OMX_BUFFERHEADERTYPE * bufferHeader = OMX_MM_MALLOC(
  sizeof(OMX_BUFFERHEADERTYPE));
  if(!bufferHeader){
    OMX_DBG_ERROR("%s Cannot allocate bufferHeader!\n",__func__);
    return  OMX_ErrorInsufficientResources;
  }
  OMX_MM_ZERO(bufferHeader, OMX_BUFFERHEADERTYPE);

  omx_jpeg_output_buffer * buffer =
  OMX_MM_MALLOC(sizeof(omx_jpeg_output_buffer));
  if(!buffer){
    OMX_DBG_ERROR("%s Cannot allocate buffer!\n",__func__);
    OMX_MM_FREE(bufferHeader);
    return  OMX_ErrorInsufficientResources;
  }
  OMX_MM_ZERO(buffer, omx_jpeg_output_buffer);

  *ppBufferHdr = bufferHeader;
  bufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);
  bufferHeader->nVersion = version;
  bufferHeader->pBuffer = pBuffer;
  bufferHeader->nAllocLen = comp->inPort->nBufferSize;
  bufferHeader->nFilledLen = 0;
  bufferHeader->nOffset = 0;
  bufferHeader->pAppPrivate = appData;
  bufferHeader->pOutputPortPrivate = comp->outPort;
  bufferHeader->pPlatformPrivate = buffer;
  /*TODO other flags need to be checked*/

  buffer->comp = comp;
  buffer->outputHeader = bufferHeader;
  buffer->fd = -1;
  buffer->offset = 0;
  buffer->length = bytes;
  buffer->addr = pBuffer;

  /*output port populated with 1 buffer*/
  comp->outPort->bPopulated = OMX_TRUE;

  return rc;
}

/*==============================================================================
 * FUNCTION    - jpegEncode -
 *
 * DESCRIPTION:
 *============================================================================*/
int jpegEncode(omx_jpeg_comp * comp, omx_jpeg_input_buffer * inputBuffer,
               omx_jpeg_input_buffer * inputBuffer1,
               omx_jpeg_output_buffer * outputBuffer){
  int cbcrStartOff = 0;
  int padded_size = 0;
  int actual_size = 0;
  int rc = 0;
  uint32_t padding = 0;

  OMX_DBG_HIGH("%s", __func__);
  if (!cancelPicture) {
  OMX_DBG_HIGH("%s Picture not canceled - Start Encode\n", __func__);
  pthread_mutex_lock(&comp->abort_mutex);
  rc = jpege_init(&comp->jpeg_encoder, jpege_event_handler, outputBuffer);
  ERROR_ENCODING(rc, errorHandler(comp->queue));
  rc = jpege_get_default_config(&comp->jpege_config);
  ERROR_ENCODING(rc, errorHandler(comp->queue));

  /*set the component input and output buffer*/
  comp->inputBuffer = inputBuffer;
  comp->inputBuffer1 = inputBuffer1;
  comp->outputBuffer = outputBuffer;
  inputBuffer->main_img_info.color_format = comp->preferences.color_format;
  OMX_DBG_INFO("Main img color_format = %d\n", inputBuffer->main_img_info.color_format);

  if ((inputBuffer->main_img_info.color_format >= JPEG_BITSTREAM_H2V2) &&
     (inputBuffer->main_img_info.color_format < JPEG_COLOR_FORMAT_MAX))  {
    OMX_DBG_INFO("%s: Main image is bit stream\n", __func__);
    rc = jpeg_buffer_init(
       &inputBuffer->main_img_info.p_fragments[0].color.bitstream.bitstream_buf);
       ERROR_ENCODING(rc, errorHandler(comp->queue));
       rc = jpeg_buffer_reset(
         inputBuffer->main_img_info.p_fragments[0].color.bitstream.bitstream_buf);
       ERROR_ENCODING(rc, errorHandler(comp->queue));
       OMX_DBG_INFO("%s: inputBuffer->length =%d \n",__func__, inputBuffer->length);
       rc = jpeg_buffer_use_external_buffer(
         inputBuffer->main_img_info.p_fragments[0].color.bitstream.bitstream_buf,
         inputBuffer->addr,
         inputBuffer->length,
         inputBuffer->fd);
       ERROR_ENCODING(rc, errorHandler(comp->queue));
       inputBuffer->main_img_info.width = comp->inPort->format.image.nFrameWidth;
       inputBuffer->main_img_info.height = comp->inPort->format.image.nFrameHeight;
       inputBuffer->main_img_info.p_fragments[0].width =
       comp->inPort->format.image.nStride;
       inputBuffer->main_img_info.p_fragments[0].height =
       comp->inPort->format.image.nSliceHeight;
       inputBuffer->main_img_info.fragment_cnt = 1;
       //comp->jpege_config.main_cfg.restart_interval = 4;
  } else {

  rc = jpeg_buffer_init(
  &inputBuffer->main_img_info.p_fragments[0].color.yuv.luma_buf);
       ERROR_ENCODING(rc, errorHandler(comp->queue));
  rc = jpeg_buffer_init(
  &inputBuffer->main_img_info.p_fragments[0].color.yuv.chroma_buf);
       ERROR_ENCODING(rc, errorHandler(comp->queue));

  inputBuffer->main_img_info.width = comp->inPort->format.image.nFrameWidth;
  inputBuffer->main_img_info.height = comp->inPort->format.image.nFrameHeight;

  inputBuffer->main_img_info.p_fragments[0].width =
  comp->inPort->format.image.nStride;
  inputBuffer->main_img_info.p_fragments[0].height =
  comp->inPort->format.image.nSliceHeight;
   inputBuffer->main_img_info.p_fragments[0].num_of_planes = 2;

  inputBuffer->main_img_info.fragment_cnt = 1;

  inputBuffer->main_img_info.color_format = comp->preferences.color_format;
  OMX_DBG_INFO("%s: Main img color_format = %d\n",
    __func__, comp->preferences.color_format);
  OMX_DBG_INFO("%s:width %d height %d yoff %d cbcr %d buffsize %d padd %d\n",
    __func__, comp->offset.width,comp->offset.height,
    comp->offset.yOffset,comp->offset.cbcrOffset,
    comp->offset.totalSize, comp->offset.paddedFrameSize);
  /*use the fd*/
  rc = jpeg_buffer_reset(
  inputBuffer->main_img_info.p_fragments[0].color.yuv.luma_buf);
       ERROR_ENCODING(rc, errorHandler(comp->queue));
  rc = jpeg_buffer_reset(
  inputBuffer->main_img_info.p_fragments[0].color.yuv.chroma_buf);
       ERROR_ENCODING(rc, errorHandler(comp->queue));

  OMX_DBG_INFO("%s:input buffer length %d",  __func__, inputBuffer->length);
  rc = jpeg_buffer_use_external_buffer(
  inputBuffer->main_img_info.p_fragments[0].color.yuv.luma_buf,
  inputBuffer->addr,
  inputBuffer->length,
  inputBuffer->fd);
      ERROR_ENCODING(rc, errorHandler(comp->queue));

  //Double padding for zsl rotation
  padding = CEILING16(inputBuffer->main_img_info.height) -
    inputBuffer->main_img_info.height;
  cbcrStartOff = PAD_TO_WORD(inputBuffer->main_img_info.width *
    (inputBuffer->main_img_info.height + (2 * padding)));

  OMX_DBG_INFO("cbcrStartOffset in component is %d\n", cbcrStartOff);

  if (comp->aoffset.cbcrOffset > 0) {
    OMX_DBG_INFO("acbcroffset in component is %d\n", comp->aoffset.cbcrOffset);
    cbcrStartOff = comp->aoffset.cbcrOffset;
  }

  comp->jpege_config.preference = comp->preferences.preference;

  rc = jpeg_buffer_attach_existing(
  inputBuffer->main_img_info.p_fragments[0].color.yuv.chroma_buf,
    inputBuffer->main_img_info.p_fragments[0].color.yuv.luma_buf,
    cbcrStartOff);
     ERROR_ENCODING(rc, errorHandler(comp->queue));
  rc = jpeg_buffer_set_actual_size(
  inputBuffer->main_img_info.p_fragments[0].color.yuv.luma_buf,
  inputBuffer->main_img_info.width *
  inputBuffer->main_img_info.height);
     ERROR_ENCODING(rc, errorHandler(comp->queue));
  rc = jpeg_buffer_set_actual_size(
  inputBuffer->main_img_info.p_fragments[0].color.yuv.chroma_buf,
  inputBuffer->main_img_info.width *
  inputBuffer->main_img_info.height / 2);
     ERROR_ENCODING(rc, errorHandler(comp->queue));

  OMX_DBG_INFO("%s: yoffset = %d\n", __func__, comp->offset.yOffset);
  rc = jpeg_buffer_set_start_offset(
       inputBuffer->main_img_info.p_fragments[0].color.yuv.luma_buf,
       comp->offset.yOffset);
  ERROR_ENCODING(rc, errorHandler(comp->queue));

  rc = jpeg_buffer_set_start_offset(
       inputBuffer->main_img_info.p_fragments[0].color.yuv.chroma_buf,
       comp->offset.cbcrOffset -
       comp->offset.paddedFrameSize);
  ERROR_ENCODING(rc, errorHandler(comp->queue));

#ifndef USE_JPEG1_LIB
  jpeg_buffer_set_phy_offset(
  inputBuffer->main_img_info.p_fragments[0].color.yuv.luma_buf,
  inputBuffer->offset);
#endif

  /*scaling*/
  if(comp->inputCrop.nWidth != 0 && comp->inputCrop.nHeight !=0 &&
    comp->outputCrop.nWidth != 0 && comp->outputCrop.nHeight != 0) {
         OMX_DBG_INFO("%s: Main Image Scaling Enabled", __func__);
         comp->jpege_config.main_cfg.scale_cfg.enable = 1;
         comp->jpege_config.main_cfg.scale_cfg.h_offset =
           comp->inputCrop.nLeft;
         comp->jpege_config.main_cfg.scale_cfg.v_offset =
           comp->inputCrop.nTop;
         comp->jpege_config.main_cfg.scale_cfg.input_width =
            comp->inputCrop.nWidth;
         comp->jpege_config.main_cfg.scale_cfg.input_height =
            comp->inputCrop.nHeight;
         comp->jpege_config.main_cfg.scale_cfg.output_width=
            comp->outputCrop.nWidth;
         comp->jpege_config.main_cfg.scale_cfg.output_height=
              comp->outputCrop.nHeight;
    }
}

 /*thumbnail*/
  if (comp->thumbnailPresent && (comp->inPort1->format.image.nFrameWidth > 0) && (comp->inPort1->format.image.nFrameHeight > 0)) {
    rc = jpeg_buffer_init(
    &inputBuffer->tn_img_info.p_fragments[0].color.yuv.luma_buf);

    ONWARNING(rc);
    rc = jpeg_buffer_init(
    &inputBuffer->tn_img_info.p_fragments[0].color.yuv.chroma_buf);
    ONWARNING(rc);
    inputBuffer->tn_img_info.width =
    comp->inPort1->format.image.nFrameWidth;
    OMX_DBG_INFO("%s:%d,inputBuffer->tn_img_info.width=%d \n", __func__,
      __LINE__,inputBuffer->tn_img_info.width);
    inputBuffer->tn_img_info.height =

    comp->inPort1->format.image.nFrameHeight;
    OMX_DBG_INFO("%s:%d,inputBuffer->tn_img_info.height=%d \n", __func__,
      __LINE__,inputBuffer->tn_img_info.height);

    inputBuffer->tn_img_info.fragment_cnt = 1;
    inputBuffer->tn_img_info.color_format =
    comp->preferences.thumbnail_color_format;
    inputBuffer->tn_img_info.p_fragments[0].width =
    comp->inPort1->format.image.nStride;
    inputBuffer->tn_img_info.p_fragments[0].height
    = comp->inPort1->format.image.nSliceHeight;
     inputBuffer->tn_img_info.p_fragments[0].num_of_planes = 2;
    rc = jpeg_buffer_reset(
    inputBuffer->tn_img_info.p_fragments[0].color.yuv.luma_buf);
    ONWARNING(rc);
    rc = jpeg_buffer_reset(
    inputBuffer->tn_img_info.p_fragments[0].color.yuv.chroma_buf);
    ONWARNING(rc);

    rc = jpeg_buffer_use_external_buffer(
    inputBuffer->tn_img_info.p_fragments[0].color.yuv.luma_buf,
    inputBuffer1->addr,
    inputBuffer1->length,
    inputBuffer1->fd);
    ONWARNING(rc);

    cbcrStartOff = PAD_TO_WORD(inputBuffer->tn_img_info.width *
                             inputBuffer->tn_img_info.height);
    if (comp->aoffset.cbcrOffset > 0) {
       OMX_DBG_INFO("acbcroffset in component is %d\n",comp->aoffset.cbcrOffset );
       cbcrStartOff = comp->aoffset.cbcrOffset;
    }

        /* cbcroffset is needs be calculated again for thumbnail */
    rc = jpeg_buffer_attach_existing(
    inputBuffer->tn_img_info.p_fragments[0].color.yuv.chroma_buf,
    inputBuffer->tn_img_info.p_fragments[0].color.yuv.luma_buf,
    cbcrStartOff);
    ONWARNING(rc);
    rc = jpeg_buffer_set_actual_size(
    inputBuffer->tn_img_info.p_fragments[0].color.yuv.luma_buf,
    inputBuffer->tn_img_info.width *
    inputBuffer->tn_img_info.height);
    ONWARNING(rc);
    rc = jpeg_buffer_set_actual_size(
    inputBuffer->tn_img_info.p_fragments[0].color.yuv.chroma_buf,
    inputBuffer->tn_img_info.width *
    inputBuffer->tn_img_info.height / 2);
    ONWARNING(rc);
    if ((comp->rotation == 90) || (comp->rotation == 180)) {
      rc = jpeg_buffer_set_start_offset(
      inputBuffer->tn_img_info.p_fragments[0].color.yuv.luma_buf,
      (padded_size - actual_size));
        ONWARNING(rc);
      rc = jpeg_buffer_set_start_offset(
      inputBuffer->tn_img_info.p_fragments[0].color.yuv.chroma_buf,
      ((padded_size - actual_size) >> 1));
        ONWARNING(rc);
    }
    comp->jpege_config.thumbnail_present = 1;

      /*for scaling*/

    if (comp->thumbnail.scaling) {
      comp->jpege_config.thumbnail_cfg.scale_cfg.enable = 1;
      comp->jpege_config.thumbnail_cfg.scale_cfg.input_width =
        comp->thumbnail.cropWidth;
      comp->jpege_config.thumbnail_cfg.scale_cfg.input_height =
        comp->thumbnail.cropHeight;
      comp->jpege_config.thumbnail_cfg.scale_cfg.h_offset =
      comp->thumbnail.left;
      comp->jpege_config.thumbnail_cfg.scale_cfg.v_offset =
      comp->thumbnail.top;
    } else {
      comp->jpege_config.thumbnail_cfg.scale_cfg.input_width
      = inputBuffer->tn_img_info.width;
      comp->jpege_config.thumbnail_cfg.scale_cfg.input_height
      = inputBuffer->tn_img_info.height;
      comp->jpege_config.thumbnail_cfg.scale_cfg.v_offset = 0;
      comp->jpege_config.thumbnail_cfg.scale_cfg.h_offset = 0;
    }

    OMX_DBG_INFO("%s:thumbnail width %d height %d",  __func__,
    comp->thumbnail.width, comp->thumbnail.height );

    comp->jpege_config.thumbnail_cfg.scale_cfg.output_width =
       comp->thumbnail.width;
    comp->jpege_config.thumbnail_cfg.scale_cfg.output_height =
      comp->thumbnail.height;
    inputBuffer->jpege_source.p_thumbnail = &inputBuffer->tn_img_info;

  } else {
    comp->jpege_config.thumbnail_present = 0;
  }

  inputBuffer->jpege_source.p_main = &inputBuffer->main_img_info;

  /*quality*/
  comp->jpege_config.main_cfg.quality = comp->mainImageQuality.nQFactor;
  comp->jpege_config.thumbnail_cfg.quality = comp->thumbnailQuality.nQFactor;

  comp->jpege_config.main_cfg.rotation_degree_clk = comp->rotation;
  comp->jpege_config.thumbnail_cfg.rotation_degree_clk = comp->rotation;

  rc = jpege_set_source(comp->jpeg_encoder, &inputBuffer->jpege_source);
   ERROR_ENCODING(rc, errorHandler(comp->queue));


  rc = jpeg_buffer_init(&outputBuffer->jpege_dest_buffer[0]);
   ERROR_ENCODING(rc, errorHandler(comp->queue));
  rc = jpeg_buffer_init(&outputBuffer->jpege_dest_buffer[1]);
   ERROR_ENCODING(rc, errorHandler(comp->queue));

  int is_pmem = 0;
#ifdef USE_JPEG1_LIB
  is_pmem = 1;
#endif

  rc = jpeg_buffer_allocate(outputBuffer->jpege_dest_buffer[0],
    JPEGE_FRAGMENT_SIZE, is_pmem);
 ERROR_ENCODING(rc, errorHandler(comp->queue));
  rc = jpeg_buffer_allocate(outputBuffer->jpege_dest_buffer[1],
    JPEGE_FRAGMENT_SIZE, is_pmem);
 ERROR_ENCODING(rc, errorHandler(comp->queue));

  outputBuffer->jpege_dest.buffer_cnt = 2;
#ifndef USE_JPEG1_LIB
  outputBuffer->jpege_dest.p_buffer =
    &outputBuffer->jpege_dest_buffer[0];
  outputBuffer->jpege_dest.p_output_handler = jpege_output_produced_handler;
#else
  int i = 0;
  for (i=0; i<2; i++)
    outputBuffer->jpege_dest.buffers[i] = outputBuffer->jpege_dest_buffer[i];

  outputBuffer->jpege_dest.p_output_handler = jpege_output_produced_handler2;
#endif
  outputBuffer->jpege_dest.p_arg = outputBuffer;
  rc = jpege_set_destination(comp->jpeg_encoder, &outputBuffer->jpege_dest);
    ERROR_ENCODING(rc, errorHandler(comp->queue));

  pthread_mutex_lock(&jpegencoding_mutex);
  comp->encoding = 1;
  pthread_mutex_unlock(&jpegencoding_mutex);

  if (comp->mobicatenable) {
    rc =  jpege_set_mobicat_data(comp->jpeg_encoder,
      (uint8_t*)comp->mobiData.mobicatData,
      comp->mobiData.mobicatDataLength);
    comp->mobicatenable = 0;
  }

  rc = jpege_start(comp->jpeg_encoder, &comp->jpege_config, &comp->exifInfo);
  ERROR_ENCODING(rc, errorHandler(comp->queue));

  pthread_mutex_lock(&jpegStop_mutex);
  comp->isJpegEngineActive = 1;
  pthread_mutex_unlock(&jpegStop_mutex);

  pthread_mutex_unlock(&comp->abort_mutex);
 }
  OMX_DBG_HIGH("%s : X", __func__);
  return 0;
}

/*==============================================================================
 * FUNCTION    - releaseOMXBuffers -
 *
 * DESCRIPTION:
 *============================================================================*/
 void releaseOMXBuffers(omx_jpeg_comp *comp){

  OMX_DBG_HIGH("%s :E", __func__);
  exif_destroy(&comp->exifInfo);
  pthread_mutex_lock(&jpegencoding_mutex);
  if(comp->encoding)
  {
    OMX_DBG_HIGH("%s : Encoding is true..Releaseing OMX buffers\n",__func__);
    if (comp->thumbnailPresent) {
       jpeg_buffer_destroy(&comp->inputBuffer->
          tn_img_info.p_fragments[0].color.yuv.luma_buf);
      jpeg_buffer_destroy(&comp->inputBuffer->
        tn_img_info.p_fragments[0].color.yuv.chroma_buf);
    }

    jpeg_buffer_destroy(&comp->inputBuffer->
        main_img_info.p_fragments[0].color.yuv.luma_buf);
    jpeg_buffer_destroy(&comp->inputBuffer->
        main_img_info.p_fragments[0].color.yuv.chroma_buf);

    jpeg_buffer_destroy(&comp->outputBuffer->jpege_dest_buffer[0]);
    jpeg_buffer_destroy(&comp->outputBuffer->jpege_dest_buffer[1]);

    comp->encoding=0;
   }
  pthread_mutex_unlock(&jpegencoding_mutex);

  OMX_DBG_HIGH("%s :X", __func__);


}
/*==============================================================================
 * FUNCTION    - jpegStop -
 *
 * DESCRIPTION:
 *============================================================================*/
int jpegStop(omx_jpeg_comp *comp){

  int rc = 0;
  OMX_DBG_HIGH("%s : E", __func__);

  pthread_mutex_lock(&jpegStop_mutex);
  if (comp->isJpegEngineActive){
    OMX_DBG_HIGH("%s JpegEngine is active..Got lock\n",__func__);
  rc = jpege_abort(comp->jpeg_encoder);
  OMX_DBG_HIGH("%s : Jpeg Abort Done\n",__func__);
  ONWARNING(rc);

  jpege_destroy(&comp->jpeg_encoder);
    comp->isJpegEngineActive = 0;
  }
  pthread_mutex_unlock(&jpegStop_mutex);
  OMX_DBG_HIGH("%s :X", __func__);
  return 0;
}

/*==============================================================================
 * FUNCTION    - omx_jpeg_queue_init -
 *
 * DESCRIPTION:
 *============================================================================*/
void omx_jpeg_queue_init(omx_jpeg_queue * queue){
  OMX_DBG_INFO("%s", __func__);
  queue->back = 0;
  queue->front = -1;
  queue->size = 0;
}

/*==============================================================================
 * FUNCTION    - omx_jpeg_message_queue_init -
 *
 * DESCRIPTION:
 *============================================================================*/
void omx_jpeg_message_queue_init(omx_jpeg_message_queue * queue){

  omx_jpeg_queue_init(&queue->command);
  omx_jpeg_queue_init(&queue->etb);
  omx_jpeg_queue_init(&queue->ftb);
  omx_jpeg_queue_init(&queue->abort);
  pthread_mutex_init(&queue->lock, NULL);
  pthread_cond_init(&queue->cond, NULL);
  queue->messageCount = 0;
  queue->initialized = 1;
}
