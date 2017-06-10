/*============================================================================
Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <string.h>

#define DEFAULT_COLOR_FORMAT OMX_YCRCBLP_H2V2

#include "omx_component.h"

int init(OMX_COMPONENTTYPE * compType );
void jpegdAbort(omx_jpegd_comp * comp);

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


void deinit(omx_jpegd_comp * comp);
void release(omx_jpegd_comp * comp);
void configureDecoder(omx_jpegd_comp * comp);

OMX_VERSIONTYPE version = {{1,1,2,0}};

static int hw_encode =1 ;
omx_jpeg_queue_item getItem;
omx_jpeg_queue_item tempItem;

/*==============================================================================
 * FUNCTION    - get_component_fns
 *
 * DESCRIPTION:
 *============================================================================*/

void get_component_fns(OMX_COMPONENTTYPE *component_fns)
{
  OMX_DBG_ERROR("%s: E\n", __func__);

  component_fns->nSize               = sizeof(OMX_COMPONENTTYPE);
  //component_fns->nVersion.nVersion   = OMX_SPEC_VERSION;
  component_fns->pApplicationPrivate = 0;
  //component_fns->pComponentPrivate   = obj_ptr;

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

  //initialize
  init(component_fns);

  OMX_DBG_INFO("%s: X\n", __func__);
}


/*------------------------------------------------------------------------
* Function : omx_component_image_allocate_buffer
*
* Description:
---------------------------------------------------------------------------*/
OMX_ERRORTYPE
omx_component_image_allocate_buffer(OMX_IN OMX_HANDLETYPE hComp,
               OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
               OMX_IN OMX_U32                        port,
               OMX_IN OMX_PTR                     appData,
               OMX_IN OMX_U32                       bytes)
{

OMX_ERRORTYPE rc = OMX_ErrorNone;

OMX_DBG_INFO("%s: E\n", __func__);

    rc  = OMX_ErrorInsufficientResources;
return rc;
}

/*------------------------------------------------------------------------
* Function : omx_component_image_free_buffer
*
* Description:
---------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_free_buffer(OMX_IN OMX_HANDLETYPE         hComp,
           OMX_IN OMX_U32                 port,
           OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;

OMX_DBG_INFO("%s: E\n", __func__);

    omx_jpegd_comp * comp = GET_COMP(hComp);
    omx_jpeg_queue_item item;
    //check for port disabling
    /*if(comp->currentState == OMX_StateIdle && comp->inTransition
            && comp->targetState == OMX_StateLoaded)*/
    {
        OMX_MM_FREE(buffer->pPlatformPrivate);
        OMX_MM_FREE(buffer);
        comp->bufferCount--;
        if(!comp->bufferCount){
            item.message = OMX_JPEG_MESSAGE_TRANSACT_COMPLETE;
            postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
        }
    }
    /*else {
        rc = OMX_ErrorIncorrectStateOperation;
    }*/

return rc;

}

/*------------------------------------------------------------------------
* Function : omx_component_image_get_parameter
*
* Description: The OMX_GetParameter macro will get a parameter setting from a component
*              Can be invoked when the component is in any state except the

---------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_get_parameter(OMX_IN OMX_HANDLETYPE hComp,
             OMX_IN OMX_INDEXTYPE paramIndex,
             OMX_INOUT OMX_PTR     paramData)
{
    OMX_ERRORTYPE rc = OMX_ErrorNone;

    omx_jpegd_comp * comp = GET_COMP(hComp);
    //stateCheck();
    //checkVersion();

    if(paramData == NULL)
        return OMX_ErrorBadParameter;
    pthread_mutex_lock(&comp->mLock);

    switch(paramIndex){
    case OMX_IndexParamImageInit:{
        OMX_PORT_PARAM_TYPE *destType = (OMX_PORT_PARAM_TYPE *)paramData;
        memcpy(paramData, comp->portParam, sizeof(OMX_PORT_PARAM_TYPE));
        break;
    }

    case OMX_IndexParamImagePortFormat:{
        OMX_IMAGE_PARAM_PORTFORMATTYPE *destFormat =
                (OMX_IMAGE_PARAM_PORTFORMATTYPE*)paramData;
        OMX_IMAGE_PARAM_PORTFORMATTYPE * srcFormat =
                (destFormat->nPortIndex == INPUT_PORT)?
                        comp->inputFormatType:comp->outputFormatType;
        memcpy(destFormat, srcFormat, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
        break;
    }

    case OMX_IndexParamPortDefinition:{
        OMX_PARAM_PORTDEFINITIONTYPE *destPort =
                (OMX_PARAM_PORTDEFINITIONTYPE *) paramData;
        OMX_PARAM_PORTDEFINITIONTYPE *srcPort =
                (destPort->nPortIndex==INPUT_PORT)?comp->inPort:comp->outPort;
        memcpy(destPort, srcPort, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
        break;
    }

    case OMX_JPEG_EXT_USER_PREFERENCES:{
      omx_jpeg_user_preferences *user_preferences =
      (omx_jpeg_user_preferences*)paramData;
      memcpy(user_preferences,&comp->preferences,
      sizeof(omx_jpeg_user_preferences));
      break;
    }
    case OMX_JPEG_EXT_REGION:{
      omx_jpeg_region *region = (omx_jpeg_region*)paramData;
      memcpy(region,&comp->region,sizeof(omx_jpeg_region));
      break;
   }
   case OMX_JPEG_EXT_IMAGE_TYPE:{
        omx_jpeg_type * decoderType =
                (omx_jpeg_type* )paramData;
        memcpy(decoderType,&comp->decoder_type,
                sizeof(omx_jpeg_type));
       break;
    }


    default:{
        OMX_DBG_ERROR("please check");
        break;
    }

    }
    pthread_mutex_unlock(&comp->mLock);
    return rc;
}

/*------------------------------------------------------------------------
* Function : omx_component_image_set_parameter
*
* Description:
---------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_set_parameter(OMX_IN OMX_HANDLETYPE     hComp,
             OMX_IN OMX_INDEXTYPE paramIndex,
             OMX_IN OMX_PTR        paramData)
{
    OMX_ERRORTYPE rc = OMX_ErrorNone;

    omx_jpegd_comp * comp = GET_COMP(hComp);
    //stateCheck(); - should be OMX_StateLoaded state  or port should be disabled
    //checkVersion();

    if(paramData == NULL)
        return OMX_ErrorBadParameter;

    pthread_mutex_lock(&comp->mLock);
    switch(paramIndex){

    case OMX_IndexParamImageInit:{
        OMX_PORT_PARAM_TYPE *destType = (OMX_PORT_PARAM_TYPE *)paramData;
        //TODO check
        break;
    }

    case OMX_IndexParamImagePortFormat:{
        //TODO check
        break;
    }

    case OMX_IndexParamPortDefinition:{

        OMX_PARAM_PORTDEFINITIONTYPE *destPort =
               ( OMX_PARAM_PORTDEFINITIONTYPE *) paramData;
        //TODO more checks
        //updating only sizes
        if(destPort->nPortIndex == INPUT_PORT){
            comp->inPort->format.image.nFrameWidth
                = destPort->format.image.nFrameWidth;
            comp->inPort->format.image.nFrameHeight
                    = destPort->format.image.nFrameHeight;
            comp->inPort->format.image.nStride
                    = CEILING16(comp->inPort->format.image.nFrameWidth);
            comp->inPort->format.image.nSliceHeight
                    = CEILING16(comp->inPort->format.image.nFrameHeight);
        /*    comp->inPort->nBufferSize = YUV_SIZER(
                comp->inPort->format.image.nStride,
                comp->inPort->format.image.nSliceHeight);*/
            comp->outPort->nBufferSize = comp->inPort->nBufferSize;
        }
        break;
    }

    case OMX_JPEG_EXT_THUMBNAIL:{
        memcpy(&comp->thumbnail, paramData, sizeof(comp->thumbnail));
        comp->thumbnailPresent = 1;
        break;
    }

    case OMX_JPEG_EXT_IMAGE_TYPE:{
        OMX_DBG_INFO("%s: OMX_JPEG_EXT_IMAGE_TYPE\n",__func__);
        omx_jpeg_type * decoderType =
                (omx_jpeg_type* )paramData;
        memcpy(&comp->decoder_type, decoderType,
                sizeof(omx_jpeg_type));
        OMX_DBG_INFO("%s:omx_jpeg_type is %d/n",__func__,decoderType->image_type);
        configureDecoder(comp);
       break;
    }
    case OMX_JPEG_EXT_USER_PREFERENCES:{
      omx_jpeg_user_preferences *user_preferences =
      (omx_jpeg_user_preferences*)paramData;
      memcpy(&comp->preferences,user_preferences,
      sizeof(omx_jpeg_user_preferences));
      break;
    }

    case OMX_JPEG_EXT_REGION:{
        omx_jpeg_region *region = (omx_jpeg_region*)paramData;
        memcpy(&comp->region,region,sizeof(omx_jpeg_region));
        break;
   }


    default:{
       OMX_DBG_ERROR("please check");
       break;
    }
    }

    pthread_mutex_unlock(&comp->mLock);
OMX_DBG_INFO("%s: E\n", __func__);


return rc;
}

/*------------------------------------------------------------------------
* Function : omx_component_image_send_command
*
* Description:
---------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_send_command(OMX_IN OMX_HANDLETYPE hComp,
            OMX_IN OMX_COMMANDTYPE  cmd,
            OMX_IN OMX_U32       param1,
            OMX_IN OMX_PTR      cmdData)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;
    omx_jpegd_comp * comp = GET_COMP(hComp);
    omx_jpeg_queue_item item;
    pthread_mutex_lock(&comp->mLock);
    //later change to switch
    if(cmd == OMX_CommandStateSet){
        item.message = OMX_JPEG_MESSAGE_CHANGE_STATE;
        item.args[0].iValue = param1;
        postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
    } else if(cmd == OMX_CommandFlush){
        jpegdAbort(comp);
    }
    pthread_mutex_unlock(&comp->mLock);

OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}

/*------------------------------------------------------------------------
* Function : omx_component_image_empty_this_buffer
*
* Description:
---------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_empty_this_buffer(OMX_IN OMX_HANDLETYPE         hComp,
                OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
    OMX_ERRORTYPE rc = OMX_ErrorNone;
    OMX_DBG_ERROR("%s", __func__);

    omx_jpeg_queue_item item;
    omx_jpegd_comp * comp = GET_COMP(hComp);
    pthread_mutex_lock(&comp->mLock);

    item.message = OMX_JPEG_MESSAGE_ETB;
    item.args[0].pValue = buffer->pPlatformPrivate;
    OMX_DBG_INFO("%s : input is %p", __func__,item.args[0].pValue);
    postMessage(comp->queue, OMX_JPEG_QUEUE_ETB, &item);

    pthread_mutex_unlock(&comp->mLock);
    return rc;
}

/*------------------------------------------------------------------------
* Function : omx_component_image_fill_this_buffer
*
* Description:
---------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_fill_this_buffer(OMX_IN OMX_HANDLETYPE         hComp,
               OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;
    omx_jpeg_queue_item item;
    omx_jpegd_comp * comp = GET_COMP(hComp);
    pthread_mutex_lock(&comp->mLock);
    item.message = OMX_JPEG_MESSAGE_FTB;
    item.args[0].pValue = buffer->pPlatformPrivate;
    postMessage(comp->queue, OMX_JPEG_QUEUE_FTB, &item);
    pthread_mutex_unlock(&comp->mLock);
OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}


/*------------------------------------------------------------------------
* Function : omx_component_image_set_callbacks
*
* Description:
---------------------------------------------------------------------------*/
OMX_ERRORTYPE
omx_component_image_set_callbacks(OMX_IN OMX_HANDLETYPE        hComp,
             OMX_IN OMX_CALLBACKTYPE* callbacks,
             OMX_IN OMX_PTR             appData)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;
    OMX_DBG_INFO("%s set callback %p", __func__,callbacks);
    omx_jpegd_comp * comp = GET_COMP(hComp);
    pthread_mutex_lock(&comp->mLock);
    comp->callback = callbacks;
    comp->callbackAppData = appData;
    pthread_mutex_unlock(&comp->mLock);

OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}

/*------------------------------------------------------------------------
* Function : configureDecoder
*
* Description:Configure the decoder functions depending on the decoder type
---------------------------------------------------------------------------*/
void configureDecoder(omx_jpegd_comp * comp){
   OMX_DBG_INFO("%s: decoder type is %d\n", __func__,comp->decoder_type.image_type);
    switch(comp->decoder_type.image_type) {
    case OMX_JPEG:
    case OMX_JPS:
        OMX_DBG_INFO("%s: Calling configure_jpeg_decoder\n", __func__);
        configure_jpeg_decoder(comp->decoder);
        break;
    case OMX_MPO:{
        OMX_DBG_INFO("%s: Calling configure_mpo_decoder\n", __func__);
        configure_mpo_decoder(comp->decoder);
        break;
    }
   }
}

/*------------------------------------------------------------------------
* Function : omx_component_image_deinit
*
* Description:
---------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_deinit(OMX_IN OMX_HANDLETYPE hComp)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;

    OMX_DBG_INFO("%s: E\n", __func__);

    omx_jpeg_queue_item item;
    omx_jpegd_comp * comp = GET_COMP(hComp);
    pthread_mutex_lock(&comp->mLock);
    deinit(comp);
    pthread_mutex_unlock(&comp->mLock);

    OMX_DBG_INFO("%s: X\n", __func__);
return rc;
}

/*------------------------------------------------------------------------
* Function : postMessage
*
* Description:
---------------------------------------------------------------------------*/

int postMessage(omx_jpeg_message_queue * mqueue, omx_jpeg_queue_type type,
        omx_jpeg_queue_item * item){

    int ret = 0;

    pthread_mutex_lock(&mqueue->lock);
    switch(type){
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
    pthread_cond_signal(&mqueue->cond);
    pthread_mutex_unlock(&mqueue->lock);

    return ret;
}


/*------------------------------------------------------------------------
* Function : omx_component_image_use_input_buffer
*
* Description: This function enables the client to pass a buffer to be used as
* the inout buffer
---------------------------------------------------------------------------*/
OMX_ERRORTYPE
omx_component_image_use_input_buffer(OMX_IN OMX_HANDLETYPE hComp,
          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
          OMX_IN OMX_U32                        port,
          OMX_IN OMX_PTR                     appData,
          OMX_IN OMX_U32                       bytes,
          OMX_IN OMX_U8*                      pBuffer){

    OMX_ERRORTYPE rc = OMX_ErrorNone;
    OMX_DBG_ERROR("%s : E", __func__);

    omx_jpegd_comp * comp = GET_COMP(hComp);
    omx_jpeg_pmem_info * info = (omx_jpeg_pmem_info *)appData;
    OMX_BUFFERHEADERTYPE * bufferHeader = OMX_MM_MALLOC(
            sizeof(OMX_BUFFERHEADERTYPE));

    if(!bufferHeader) {
        OMX_DBG_ERROR("%s : bufferHeader MALLOC failed", __func__);
        return OMX_ErrorInsufficientResources;
    }

    OMX_MM_ZERO(bufferHeader, OMX_BUFFERHEADERTYPE);

    //create input buffer
    omx_jpegd_input_buffer * buffer =
            OMX_MM_MALLOC(sizeof(omx_jpegd_input_buffer));
    if(!buffer) {
        OMX_MM_FREE(bufferHeader);
        OMX_DBG_ERROR("%s : bufferHeader MALLOC failed", __func__);
        return OMX_ErrorInsufficientResources;
    }
    OMX_MM_ZERO(buffer, omx_jpegd_input_buffer);
    *ppBufferHdr = bufferHeader;
    bufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);
    bufferHeader->nVersion = version;
    bufferHeader->pBuffer = pBuffer;
    bufferHeader->nAllocLen = bytes;
    bufferHeader->nFilledLen = 0;
    bufferHeader->nOffset = info->offset;
    bufferHeader->pAppPrivate = appData;
    bufferHeader->pInputPortPrivate = comp->inPort;
    bufferHeader->pPlatformPrivate = buffer;

    buffer->comp = comp;
    buffer->inputHeader = bufferHeader;
    buffer->fd = info->fd;
    buffer->offset = info->offset;
    buffer->length = bytes;
    buffer->addr = pBuffer;
     OMX_DBG_ERROR("pbuffer in use inout bufffer = %p",pBuffer);

     //input port populated
    comp->inPort->bPopulated = OMX_TRUE;

    return rc;

}
/*------------------------------------------------------------------------
* Function : configureOutputBuffer
*
* Description: This function configures the output buffer to be used
*              during decoding by the decoder.
*
---------------------------------------------------------------------------*/
int configureOutputBuffer(omx_jpegd_comp * comp){

        OMX_DBG_INFO("%s: E", __func__);

    omx_jpegd_output_buffer * buffer =
            OMX_MM_MALLOC(sizeof(omx_jpegd_output_buffer));
    if(!buffer) {
        OMX_DBG_ERROR("%s : buffer MALLOC failed", __func__);
        return OMX_ErrorInsufficientResources;
    }
    OMX_MM_ZERO(buffer, omx_jpegd_output_buffer);
    OMX_BUFFERHEADERTYPE * bufferHeader = OMX_MM_MALLOC(
            sizeof(OMX_BUFFERHEADERTYPE));
    if(!bufferHeader) {
        OMX_MM_FREE(buffer);
        OMX_DBG_ERROR("%s : buffer MALLOC failed", __func__);
        return OMX_ErrorInsufficientResources;
    }
    OMX_MM_ZERO(bufferHeader, OMX_BUFFERHEADERTYPE);
    bufferHeader->nSize = sizeof(OMX_BUFFERHEADERTYPE);
    bufferHeader->nVersion = version;
    bufferHeader->nFilledLen = 0;
    bufferHeader->nOffset = 0;
    bufferHeader->pOutputPortPrivate = comp->outPort;
    bufferHeader->pPlatformPrivate = buffer;
    buffer->outputHeader = bufferHeader;
    buffer->fd = -1;
    buffer->offset = 0;

    comp->outputBuffer = buffer;
    comp->outPort->bPopulated = OMX_TRUE;

    OMX_DBG_INFO("%s: X", __func__);

    return 0;
}



/*------------------------------------------------------------------------
* Function : omx_component_image_use_input_buffer
*
* Description: This function enables the client to pass a buffer to be used as
* the output buffer
---------------------------------------------------------------------------*/
OMX_ERRORTYPE
omx_component_image_use_output_buffer(OMX_IN OMX_HANDLETYPE hComp,
          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
          OMX_IN OMX_U32                        port,
          OMX_IN OMX_PTR                     appData,
          OMX_IN OMX_U32                       bytes,
          OMX_IN OMX_U8*                      pBuffer){

    OMX_DBG_ERROR("%s", __func__);
    OMX_DBG_ERROR("%s output buffer is %p", __func__,pBuffer);
    OMX_ERRORTYPE rc = OMX_ErrorNone;

    omx_jpegd_comp * comp = GET_COMP(hComp);
    OMX_BUFFERHEADERTYPE * bufferHeader = OMX_MM_MALLOC(
            sizeof(OMX_BUFFERHEADERTYPE));
    if(!bufferHeader) {
        OMX_DBG_ERROR("%s : bufferHeader MALLOC failed", __func__);
        return OMX_ErrorInsufficientResources;
    }
    OMX_MM_ZERO(bufferHeader, OMX_BUFFERHEADERTYPE);

    omx_jpegd_output_buffer * buffer =
            OMX_MM_MALLOC(sizeof(omx_jpegd_output_buffer));
    if(!buffer) {
        OMX_DBG_ERROR("%s : buffer MALLOC failed", __func__);
        OMX_MM_FREE(bufferHeader);
        return OMX_ErrorInsufficientResources;
    }
    OMX_MM_ZERO(buffer, omx_jpegd_output_buffer);

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

    buffer->outputHeader = bufferHeader;
    buffer->fd = -1;
    buffer->offset = 0;
    buffer->length = bytes;
    buffer->addr = pBuffer;

    //output port populated with 1 buffer
    comp->outPort->bPopulated = OMX_TRUE;

    return rc;

}

/*----------------------------------------------------------------------
* Function : omx_component_image_get_state
*
* Description:
------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_get_state(OMX_IN OMX_HANDLETYPE  hComp,
         OMX_OUT OMX_STATETYPE* state)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;

OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}


/*------------------------------------------------------------------------
* Function : omx_component_image_get_version
*
* Description:
---------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_get_version(OMX_IN OMX_HANDLETYPE               hComp,
                    OMX_OUT OMX_STRING          componentName,
                    OMX_OUT OMX_VERSIONTYPE* componentVersion,
                    OMX_OUT OMX_VERSIONTYPE*      specVersion,
                    OMX_OUT OMX_UUIDTYPE*       componentUUID)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;

OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}

/*------------------------------------------------------------------------
* Function : omx_component_image_get_config
*
* Description:
---------------------------------------------------------------------------*/

OMX_ERRORTYPE
omx_component_image_get_config(OMX_IN OMX_HANDLETYPE      hComp,
          OMX_IN OMX_INDEXTYPE configIndex,
          OMX_INOUT OMX_PTR     configData)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;

OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}


/*------------------------------------------------------------------------
* Function : omx_component_image_set_config
*
* Description:
---------------------------------------------------------------------------*/
OMX_ERRORTYPE
omx_component_image_set_config(OMX_IN OMX_HANDLETYPE      hComp,
          OMX_IN OMX_INDEXTYPE configIndex,
          OMX_IN OMX_PTR        configData)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;


    omx_jpegd_comp * comp = GET_COMP(hComp);
    pthread_mutex_lock(&comp->mLock);
    if(configIndex == OMX_IndexConfigCommonRotate){
        OMX_CONFIG_ROTATIONTYPE *rotType = (OMX_CONFIG_ROTATIONTYPE *)configData;
        if(rotType->nPortIndex == OUTPUT_PORT)
            comp->rotation = rotType->nRotation;
    }

   if(configIndex == OMX_IndexConfigCommonOutputCrop){
        OMX_CONFIG_RECTTYPE * rectType = (OMX_CONFIG_RECTTYPE*) configData;
        if(rectType->nPortIndex == OUTPUT_PORT){

            memcpy(&comp->outputCrop, rectType, sizeof(OMX_CONFIG_RECTTYPE));

            //change the output dimension
        }

    }
    pthread_mutex_unlock(&comp->mLock);

OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}

/*------------------------------------------------------------------------
* Function : omx_component_image_get_extension_index
*
* Description:
---------------------------------------------------------------------------*/
OMX_ERRORTYPE
omx_component_image_get_extension_index(OMX_IN OMX_HANDLETYPE      hComp,
                  OMX_IN OMX_STRING      paramName,
                  OMX_OUT OMX_INDEXTYPE* indexType)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;
    int ret = strncmp(paramName, OMX_JPEG_PREFIX, OMX_JPEG_PREFIX_LENGTH);
    int i = 0;

    if(ret)
        return OMX_ErrorBadParameter;

    for(i=OMX_JPEG_EXT_START;i<OMX_JPEG_EXT_END; i++){
       if (!strncmp(paramName+OMX_JPEG_PREFIX_LENGTH,
       omx_jpeg_ext_name[i-OMX_JPEG_EXT_START],
       strlen(omx_jpeg_ext_name[i-OMX_JPEG_EXT_START]))
       && (strlen(paramName+OMX_JPEG_PREFIX_LENGTH) ==
       strlen(omx_jpeg_ext_name[i-OMX_JPEG_EXT_START])))
       {
           *indexType = i;
            return rc;
        }
    }
    OMX_DBG_ERROR("jpeg ext failure -1");
    rc = OMX_ErrorBadParameter;

    OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}

/*------------------------------------------------------------------------
* Function : omx_component_image_tunnel_request
*
* Description:
---------------------------------------------------------------------------*/
OMX_ERRORTYPE
omx_component_image_tunnel_request(OMX_IN OMX_HANDLETYPE                hComp,
                       OMX_IN OMX_U32                        port,
                       OMX_IN OMX_HANDLETYPE        peerComponent,
                       OMX_IN OMX_U32                    peerPort,
                       OMX_INOUT OMX_TUNNELSETUPTYPE* tunnelSetup)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;

OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}


/*-------------------------------------------------------------------------------------------------
* Function : omx_component_image_use_buffer
*
* Description: Call the functions use_input_buffer or use_output_buffer depending ont he port type.
---------------------------------------------------------------------------------------------------*/
OMX_ERRORTYPE
omx_component_image_use_buffer(OMX_IN OMX_HANDLETYPE                hComp,
          OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
          OMX_IN OMX_U32                        port,
          OMX_IN OMX_PTR                     appData,
          OMX_IN OMX_U32                       bytes,
          OMX_IN OMX_U8*                      buffer)
{
    OMX_ERRORTYPE rc = OMX_ErrorNone;

    //not checking if in loaded state as race conditions
    omx_jpegd_comp * comp = GET_COMP(hComp);
    pthread_mutex_lock(&comp->mLock);

    if(port == INPUT_PORT){
        omx_component_image_use_input_buffer(hComp, bufferHdr, port, appData,
                bytes, buffer);
    }/* else {
        omx_component_image_use_output_buffer(hComp, bufferHdr, port, appData,
                bytes, buffer);
    }*/

    comp->bufferCount++;
    //We will not use an external buffer as the output buffer since we
    // dont know the size of the images in the file.
    if(!comp->outPort->bPopulated){
       configureOutputBuffer(comp);
    }

    if(comp->inPort->bPopulated && comp->outPort->bPopulated){
            //&& comp->inTransition && comp->targetState == OMX_StateIdle){
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

/*-------------------------------------------------------------------------------------------------
* Function : omx_component_image_use_EGL_image
*
* Description:
---------------------------------------------------------------------------------------------------*/
OMX_ERRORTYPE
omx_component_image_use_EGL_image(OMX_IN OMX_HANDLETYPE                hComp,
            OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
            OMX_IN OMX_U32                        port,
            OMX_IN OMX_PTR                     appData,
            OMX_IN void*                      eglImage)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;

OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}


/*-------------------------------------------------------------------------------------------------
* Function : omx_component_image_role_enum
*
* Description:
---------------------------------------------------------------------------------------------------*/
 OMX_ERRORTYPE
omx_component_image_role_enum(OMX_IN OMX_HANDLETYPE hComp,
                  OMX_OUT OMX_U8*        role,
                  OMX_IN OMX_U32        index)
{
OMX_ERRORTYPE rc = OMX_ErrorNone;

OMX_DBG_INFO("%s: E\n", __func__);

return rc;
}

/*-------------------------------------------------------------------------------------------------
* Function : jpegdInvokeStop
*
* Description:Called after Decode is done. Sends ETBD and FTBD events
---------------------------------------------------------------------------------------------------*/
void jpegdInvokeStop(omx_jpegd_comp * comp){
   //This will be single threaded as will be called by message  thread

   OMX_DBG_INFO("%s : E\n", __func__);
   omx_jpeg_queue_item item;
    if(comp->decoding){
        comp->decoder->stop(comp);
     }
     OMX_DBG_INFO("%s: JPEGD Stop done\n",__func__);
     item.message = OMX_JPEG_MESSAGE_ETB_DONE;
     item.args[0].pValue = comp->inputBuffer;
     postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);

    OMX_DBG_INFO("%s : X\n",__func__);

}

/*==============================================================================
 * FUNCTION    - jpegdAbort -
 *
 * DESCRIPTION:
 *============================================================================*/
void jpegdAbort(omx_jpegd_comp * comp)
{
  OMX_DBG_INFO("%s: E", __func__);
  omx_jpeg_queue_item item;
  /*single threaded caller*/
  comp->decoder->stop(comp);

  /*Actually have to do above for all buffers in the etb and ftb queue*/
  item.message = OMX_JPEG_MESSAGE_EVENT;
  item.args[0].iValue = OMX_EVENT_JPEG_ABORT;
  item.args[1].iValue = 0;
  item.args[2].iValue = 0;
  postMessage(comp->queue, OMX_JPEG_QUEUE_ABORT, &item);
  OMX_DBG_INFO("%s: X", __func__);

}
/*-------------------------------------------------------------------------------------------------
* Function : jpegFlush
*
* Description:
---------------------------------------------------------------------------------------------------*/
void jpegFlush(omx_jpegd_comp * comp){
    //single threaded caller
    omx_jpeg_queue_item item;
    jpegdInvokeStop(comp);
    //actually have to do above for all buffers in the etb and ftb queue
    item.message = OMX_JPEG_MESSAGE_EVENT;
    item.args[0].iValue = OMX_EventCmdComplete;
    item.args[1].iValue = OMX_CommandFlush;
    //??? TODO for args2 ndata2
    postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);

}

/*-------------------------------------------------------------------------------------------------
* Function : jpegd_event_handler
*
* Description: JPEGD Event handler passed during inititialization
---------------------------------------------------------------------------------------------------*/
void jpegd_event_handler(void *p_user_data, jpeg_event_t event, void *p_arg) {
    OMX_DBG_ERROR("OMX Decoder: In event handler - got event %d", event);

    omx_jpeg_queue_item item;
    omx_jpegd_comp * comp =
            (omx_jpegd_comp *)p_user_data;
    if(event == JPEG_EVENT_DONE)
    {
        comp->decode_success =true;
    }
     if (event != JPEG_EVENT_WARNING)
    {
        pthread_mutex_lock(&comp->mLock);
        comp->decoding = false;
        pthread_cond_signal(&comp->cond);
        pthread_mutex_unlock(&comp->mLock);

    }
    if (event == JPEG_EVENT_ERROR && p_arg)
    {
        fprintf(stderr, "%s: ERROR: %s\n",__func__, (char *)p_arg);
    }

}

/*-------------------------------------------------------------------------------------------------
* Function : jpegd_input_req_handler
*
* Description: This function assumes the entire JPEG/JPS/MPO file that has been pre-fetched into a big
* buffer. It then copies JPEG/MPO/JPS data from the big buffer to one of the local
* ping-pong fashioned buffer when requested.

---------------------------------------------------------------------------------------------------*/
int jpegd_input_req_handler(void *p_user_data,
                           jpeg_buffer_t   buffer,
                           uint32_t  start_offset,
                           uint32_t        length){

  uint32_t entire_buf_size, buf_size;
  uint8_t *entire_buf_ptr, *buf_ptr;
  uint32_t bytes_to_read;


  omx_jpegd_comp *comp = (omx_jpegd_comp *)p_user_data;

  jpeg_buffer_get_max_size((jpeg_buffer_t)comp->inputBuffer->addr,&entire_buf_size);
  OMX_DBG_INFO("%s -Max size of input buffer %d\n",__func__,entire_buf_size);
  jpeg_buffer_get_addr((jpeg_buffer_t)comp->inputBuffer->addr,&entire_buf_ptr);

  jpeg_buffer_get_max_size(buffer, &buf_size);
  jpeg_buffer_get_addr(buffer, &buf_ptr);

  bytes_to_read = MIN(length, buf_size);

  if(entire_buf_size < start_offset){

     fprintf(stderr, "%s - start offset is too high, start_offset = 0x%x & entire_buf_size = 0x%x\n",__func__,start_offset,entire_buf_size);
     return 0;

   }

   bytes_to_read = MIN(bytes_to_read, (entire_buf_size - start_offset));

   if (bytes_to_read > 0)
   {
      memcpy(buf_ptr, entire_buf_ptr + start_offset, bytes_to_read);
      return bytes_to_read;
   }

   else
    {
       // The caller needs this returned 0 to know that the entire Jpeg data
       // has been consumed.
       return 0;
    }
}


/*-------------------------------------------------------------------------------------------------
* Function : initInport
*
* Description: Initialize the Input port on the OMX component
---------------------------------------------------------------------------------------------------*/
void initInport(omx_jpegd_comp * comp){
    comp->inPort = OMX_MM_MALLOC(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    if(!comp->inPort) {
        OMX_DBG_ERROR("%s : comp->inPort MALLOC failed", __func__);
        return;
    }
    OMX_MM_ZERO(comp->inPort, OMX_PARAM_PORTDEFINITIONTYPE);
    comp->inPort->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    comp->inPort->nVersion = version;
    comp->inPort->nPortIndex = INPUT_PORT;
    comp->inPort->eDir = OMX_DirInput;
    comp->inPort->nBufferCountActual = 1;
    comp->inPort->nBufferCountMin =  1;
    comp->inPort->bEnabled = OMX_TRUE;
    comp->inPort->bPopulated = OMX_FALSE;
    comp->inPort->eDomain = OMX_PortDomainImage;
    comp->inPort->bBuffersContiguous = OMX_TRUE;
    //make this get page size
    comp->inPort->nBufferAlignment = 4096;

    comp->inPort->format.image.cMIMEType = NULL;
    //native renderer defaulted to zero
    comp->inPort->format.image.nFrameWidth = DEFAULT_PICTURE_WIDTH;
    comp->inPort->format.image.nFrameHeight = DEFAULT_PICTURE_HEIGHT;
    comp->inPort->format.image.nStride = CEILING16(DEFAULT_PICTURE_WIDTH);
    comp->inPort->format.image.nSliceHeight = CEILING16(DEFAULT_PICTURE_HEIGHT);
    comp->inPort->format.image.bFlagErrorConcealment = OMX_FALSE;
    //eColorFormat field is valid when eCompressionFormat is OMX_IMAGE_CodingUnused
    comp->inPort->format.image.eColorFormat = OMX_COLOR_FormatUnused;
    comp->inPort->format.image.eCompressionFormat = OMX_IMAGE_CodingEXIF;
    comp->inPort->nBufferSize = DEFAULT_PICTURE_WIDTH * DEFAULT_PICTURE_HEIGHT * 3 / 2;

}

/*-------------------------------------------------------------------------------------------------
* Function : initOutport
*
* Description: Initialize the Output port on the OMX component
---------------------------------------------------------------------------------------------------*/
void initOutport(omx_jpegd_comp * comp){
    comp->outPort = OMX_MM_MALLOC(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    if(!comp->outPort) {
        OMX_DBG_ERROR("%s : bufferHeader MALLOC failed", __func__);
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

    comp->outPort->nBufferAlignment = 4096;
    //setting to null ycbcr mime type ?
    comp->outPort->format.image.cMIMEType = NULL;
    comp->outPort->format.image.nFrameWidth = 0;
    comp->outPort->format.image.nFrameHeight = 0;
    comp->outPort->format.image.nStride = 0;
    comp->outPort->format.image.nSliceHeight = 0;

    comp->outPort->format.image.bFlagErrorConcealment = OMX_FALSE;
    comp->outPort->format.image.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
    comp->outPort->format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
    comp->outPort->nBufferSize = comp->inPort->nBufferSize;

}
/*-------------------------------------------------------------------------------------------------
* Function : changeState
*
* Description: Change the state of the OMC component
---------------------------------------------------------------------------------------------------*/
void changeState(omx_jpegd_comp * comp, OMX_STATETYPE state){

    OMX_DBG_ERROR("target state %d currn state %d", state, comp->currentState);

    //loaded to idle
     if(comp->currentState == OMX_StateLoaded){
        //try to move state
        if(state == OMX_StateIdle){
            comp->inTransition = 1;
            comp->targetState = OMX_StateIdle;
            return;
        }else {
            //add state for waiting for resources
            //throw an error
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

     if(comp->currentState == OMX_StateIdle){
        omx_jpeg_queue_item item;
        //need to take care of suspend state
        if(state == OMX_StateExecuting){
           OMX_DBG_ERROR("in idle to exe");
            //just change and give callbacks
            comp->currentState = OMX_StateExecuting;
            item.message = OMX_JPEG_MESSAGE_EVENT;
            item.args[0].iValue = OMX_EventCmdComplete;
            item.args[1].iValue = OMX_CommandStateSet;
            item.args[2].iValue = OMX_StateExecuting;
            postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
            return;
           }
         return;
    }

    if(comp->currentState == OMX_StateExecuting){
        omx_jpeg_queue_item item;
        if(state == OMX_StateIdle){
            jpegdInvokeStop(comp);
            comp->inTransition = 1;
            comp->targetState = OMX_StateIdle;
            item.message = OMX_JPEG_MESSAGE_TRANSACT_COMPLETE;
            postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);
        }
    }
}


/*-------------------------------------------------------------------------------------------------
* Function : processCommand
*
* Description: process commands from the message queue
---------------------------------------------------------------------------------------------------*/
void processCommand(omx_jpegd_comp* comp, omx_jpeg_queue_item * item){

     OMX_DBG_INFO("%s - command is %d\n",__func__,item->message);
     switch(item->message){

     case OMX_JPEG_MESSAGE_STOP:{
        pthread_mutex_lock(&comp->queue->lock);
        comp->queue->initialized = 0;
        pthread_mutex_unlock(&comp->queue->lock);
        break;
      }

    case OMX_JPEG_MESSAGE_START_DECODE:{
        comp->decoder->decode(comp,
           (omx_jpegd_input_buffer*)item->args[0].pValue);
        break;
      }

   case OMX_JPEG_MESSAGE_ETB_DONE:{
        omx_jpegd_input_buffer * buffer =
                (omx_jpegd_input_buffer *)item->args[0].pValue;
        if(!buffer->etbDone){
            comp->callback->EmptyBufferDone(comp->omxComponent,
                comp->callbackAppData,
                buffer->inputHeader);
            buffer->etbDone = 1;
        }
        break;
      }

    case OMX_JPEG_MESSAGE_FTB_DONE:{
        OMX_DBG_INFO("In OMX_JPEG_MESSAGE_FTB_DONE\n");
        omx_jpegd_output_buffer * buffer =
                (omx_jpegd_output_buffer *)item->args[0].pValue;
        if(!buffer->ftbDone){
            comp->callback->FillBufferDone(comp->omxComponent,
                comp->callbackAppData,
                buffer->outputHeader);
            buffer->ftbDone = 1;
        }
        break;
      }

    case OMX_JPEG_MESSAGE_CHANGE_STATE:{
        changeState(comp, item->args[0].iValue);
        break;
      }

    case OMX_JPEG_MESSAGE_EVENT:{
             comp->callback->EventHandler(comp->omxComponent, comp->callbackAppData,
                (OMX_EVENTTYPE)item->args[0].iValue, item->args[1].iValue,
                item->args[2].iValue, NULL);
        break;
      }

    case OMX_JPEG_MESSAGE_TRANSACT_COMPLETE:{
        comp->inTransition = 0;
        comp->currentState = comp->targetState;
        comp->callback->EventHandler(comp->omxComponent, comp->callbackAppData,
                OMX_EventCmdComplete, OMX_CommandStateSet, comp->currentState, NULL);
        break;
     }
    case OMX_JPEG_MESSAGE_DECODE_DONE:{
         comp->callback->EventHandler(comp->omxComponent, comp->callbackAppData,
                (OMX_EVENTTYPE)item->args[0].iValue, item->args[1].iValue,
                item->args[2].iValue, NULL);
         break;
    }
     case OMX_JPEG_MESSAGE_DECODED_IMAGE:{
          OMX_DBG_INFO("%s: In OMX_JPEG_MESSAGE_DECODED_IMAGE\n",__func__);
          comp->callback->EventHandler(comp->omxComponent, comp->callbackAppData,
                (OMX_EVENTTYPE)item->args[0].iValue, item->args[1].iValue,
                item->args[2].iValue, item->args[1].pValue);
         break;
    }

    case OMX_JPEG_MESSAGE_FLUSH:
    case OMX_JPEG_MESSAGE_ETB:
    case OMX_JPEG_MESSAGE_FTB:
    case OMX_JPEG_MESSAGE_INITIAL:
    case OMX_JPEG_MESSAGE_FLUSH_COMPLETE:
    case OMX_JPEG_MESSAGE_START_ENCODE:
    case OMX_JPEG_MESSAGE_DEINIT:
        break;
   }

}

/*-------------------------------------------------------------------------------------------------
* Function : handleMessage
*
* Description: Callback to handle messages and call processCommand
---------------------------------------------------------------------------------------------------*/
void* handleMessage(void *arg){
    omx_jpegd_comp * comp = (omx_jpegd_comp *)arg;
    void * element = NULL;
    omx_jpeg_message_queue * mqueue = comp->queue;
    int ret = 0;
    int commandAvaliable = 0;

    while(true){
        commandAvaliable = 0;
        pthread_mutex_lock(&mqueue->lock);

        while(mqueue->messageCount == 0 && mqueue->initialized){
            pthread_cond_wait(&mqueue->cond, &mqueue->lock);
        }
        if(mqueue->initialized == 0)
            return NULL;

        /*There can be non zero message count but no actual etbs or ftbs
        because flush cleared it*/
        if(mqueue->command.size){
            ret = omx_jpeg_queue_remove(&mqueue->command, &getItem);
            commandAvaliable = 1;
         }else{
           /* add state stuff */
               if(mqueue->etb.size &&  !comp->decoding){
               getItem.message = OMX_JPEG_MESSAGE_START_DECODE;
               //remove item from etb
               ret = omx_jpeg_queue_remove(&mqueue->etb, &tempItem);
               OMX_DBG_INFO("return value after input remove %d %d ", ret, tempItem.message);
               getItem.args[0].pValue = tempItem.args[0].pValue;
               commandAvaliable = 1;
            }
         }

        OMX_DBG_ERROR("handling message %d", getItem.message);
        mqueue->messageCount--;
        pthread_mutex_unlock(&mqueue->lock);
        if(commandAvaliable)
            processCommand(comp, &getItem);

         /*Break out of wait*/
        if (mqueue->initialized == 0)
        break;
     }
    /*This is the last function to be called to release resources*/
    release(comp);
    return NULL;
}
/*-------------------------------------------------------------------------------------------------
* Function : errorHandler
*
* Description:Initialize the OMX component
---------------------------------------------------------------------------------------------------*/
void errorHandler(){
    return;
}

/*-------------------------------------------------------------------------------------------------
*
* Description:Initialize the OMX component
---------------------------------------------------------------------------------------------------*/
int init(OMX_COMPONENTTYPE * compType ){

     int rc = 0;

     omx_jpegd_comp * comp = OMX_MM_MALLOC(sizeof(omx_jpegd_comp));
     ONERROR(!comp, errorHandler());
     OMX_MM_ZERO(comp, omx_jpegd_comp);
     compType->pComponentPrivate = (OMX_PTR)comp;

     comp->omxComponent = compType;
     comp->portParam = OMX_MM_MALLOC(sizeof(OMX_PORT_PARAM_TYPE));
     ONERROR(!comp->portParam, errorHandler());

     comp->portParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
     comp->portParam->nVersion = version;
     comp->portParam->nPorts = 2;
     comp->portParam->nStartPortNumber = INPUT_PORT;

     initInport(comp);
     initOutport(comp);

     comp->inputFormatType = OMX_MM_MALLOC(sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
     ONERROR(!comp->inputFormatType, errorHandler());
     OMX_MM_ZERO(comp->inputFormatType, OMX_IMAGE_PARAM_PORTFORMATTYPE);
     comp->inputFormatType->nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
     comp->inputFormatType->nVersion = version;
     comp->inputFormatType->nPortIndex = INPUT_PORT;
     comp->inputFormatType->nIndex = 0;
     comp->inputFormatType->eColorFormat = OMX_COLOR_FormatUnused;
     comp->inputFormatType->eCompressionFormat = OMX_IMAGE_CodingEXIF;

     comp->outputFormatType =
            OMX_MM_MALLOC(sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
     ONERROR(!comp->outputFormatType, errorHandler());
     OMX_MM_ZERO(comp->outputFormatType, OMX_IMAGE_PARAM_PORTFORMATTYPE);
     comp->outputFormatType->nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
     comp->outputFormatType->nVersion = version;
     comp->outputFormatType->eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
     comp->outputFormatType->eCompressionFormat = OMX_IMAGE_CodingUnused;
     comp->preferences.color_format = DEFAULT_COLOR_FORMAT;
     comp->totalImageCount = 0;
     comp->decoder_type.image_type = OMX_JPEG;

     pthread_mutex_init(&comp->mLock, NULL);

     //Initialize Message queue
     comp->queue = OMX_MM_MALLOC(sizeof(omx_jpeg_message_queue));
     ONERROR(!comp->queue, errorHandler());
     omx_jpeg_message_queue_init(comp->queue);

     comp->decoder = OMX_MM_MALLOC(sizeof(omx_jpeg_decoder_engine));
     ONERROR(!comp->decoder, errorHandler());

     comp->initialized = 1;

     //Create Message Thread
     rc = pthread_create(&comp->messageThread, NULL,
            handleMessage, comp);
    ONERROR(rc, errorHandler());

    comp->currentState = OMX_StateLoaded;
    OMX_DBG_INFO("omx state loaded %d", comp->currentState);
    return 0;
}

 /*-------------------------------------------------------------------------------------------------
* Function : deinit
*
* Description:
---------------------------------------------------------------------------------------------------*/
void deinit(omx_jpegd_comp * comp){

    omx_jpeg_queue_item item;
    OMX_DBG_INFO("%s : E\n",__func__);

    if (comp->decoding) {
      comp->decoder->stop(comp);
   }
    pthread_mutex_lock(&comp->queue->lock);
    comp->queue->initialized = 0;
    pthread_mutex_unlock(&comp->queue->lock);

    item.message = OMX_JPEG_MESSAGE_DEINIT;
    postMessage(comp->queue, OMX_JPEG_QUEUE_COMMAND, &item);

    OMX_DBG_INFO("%s : X\n",__func__);

}
 /*-------------------------------------------------------------------------------------------------
* Function : release
*
* Description: Release all the resources
---------------------------------------------------------------------------------------------------*/
void release(omx_jpegd_comp * comp){
   int i =0;
    OMX_DBG_INFO("%s :E\n",__func__);
    pthread_cond_destroy(&comp->cond);
    pthread_mutex_destroy(&comp->lock);
    if(comp->decoding){
        comp->decoder->stop(comp);
        if(comp->inputBuffer){
            OMX_MM_FREE(comp->inputBuffer->inputHeader);
            OMX_MM_FREE(comp->inputBuffer);
        }
        if(comp->outputBuffer){
            OMX_MM_FREE(comp->outputBuffer->outputHeader);
            OMX_MM_FREE(comp->outputBuffer);
        }
    }
    while(i< comp->totalImageCount)
    {
       if(comp->outBuffer[i]){
          OMX_MM_FREE(comp->outBuffer[i]);
       }
       i++;
    }
    OMX_MM_FREE(comp->queue);
    OMX_MM_FREE(comp->inPort);
    OMX_MM_FREE(comp->outPort);
    OMX_MM_FREE(comp->inputFormatType);
    OMX_MM_FREE(comp->outputFormatType);
    OMX_MM_FREE(comp->portParam);
    OMX_MM_FREE(comp->decoder);
    comp->omxComponent->pComponentPrivate = NULL;
    OMX_MM_FREE(comp);
    jpeg_show_leak();
    OMX_DBG_INFO("%s : X\n",__func__);
}





