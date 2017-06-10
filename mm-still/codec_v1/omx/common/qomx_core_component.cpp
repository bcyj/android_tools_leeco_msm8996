/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include "qomx_core.h"
#include "qomx_core_component.h"
#include "QOMXImageCodec.h"


/*==============================================================================
* Function : create_component_fns
* Parameters: aobj - Pointer to the omx component class
* Return value : void*
* Description: This function initilaizes the component handle that will be
* passed back to the client on initialization with all the function pointers.
==============================================================================*/
void *create_component_fns(OMX_PTR aobj)
{
  QIDBG_HIGH("%s %d:] Component ptr is %p\n", __func__, __LINE__, aobj);
  QOMXImageCodec *lobj = (QOMXImageCodec *)aobj;
  OMX_COMPONENTTYPE *lcomponent_fns =  lobj->ComponentHandle();
  lcomponent_fns->nSize               = sizeof(OMX_COMPONENTTYPE);
  lcomponent_fns->nVersion.nVersion   = OMX_SPEC_VERSION;
  lcomponent_fns->pApplicationPrivate = 0;
  lcomponent_fns->pComponentPrivate   = aobj;
  lcomponent_fns->AllocateBuffer      = qomx_component_allocate_buffer;
  lcomponent_fns->FreeBuffer          = qomx_component_free_buffer;
  lcomponent_fns->GetParameter        = qomx_component_get_parameter;
  lcomponent_fns->SetParameter        = qomx_component_set_parameter;
  lcomponent_fns->SendCommand         = qomx_component_send_command;
  lcomponent_fns->FillThisBuffer      = qomx_component_fill_this_buffer;
  lcomponent_fns->EmptyThisBuffer     = qomx_component_empty_this_buffer;
  lcomponent_fns->GetState            = qomx_component_get_state;
  lcomponent_fns->GetComponentVersion = qomx_component_get_version;
  lcomponent_fns->GetConfig           = qomx_component_get_config;
  lcomponent_fns->SetConfig           = qomx_component_set_config;
  lcomponent_fns->GetExtensionIndex   = qomx_component_get_extension_index;
  lcomponent_fns->ComponentTunnelRequest = qomx_component_tunnel_request;
  lcomponent_fns->UseBuffer           = qomx_component_use_buffer;
  lcomponent_fns->SetCallbacks        = qomx_component_set_callbacks;
  lcomponent_fns->UseEGLImage         = qomx_component_use_EGL_image;
  lcomponent_fns->ComponentRoleEnum   = qomx_component_role_enum;
  lcomponent_fns->ComponentDeInit     = qomx_component_deinit;

  qomx_component_init(lcomponent_fns);
  return (void *)lcomponent_fns;
}

/*==============================================================================
* Function : qomx_component_init
* Parameters: hComp
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_init(OMX_IN OMX_HANDLETYPE hComp)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_init(hComp);
  }
  return lrc;
}
/*==============================================================================
* Function : qomx_component_allocate_buffer
* Parameters: hComp, bufferHdr, port, appData, bytes
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_allocate_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 bytes)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_allocate_buffer(hComp, bufferHdr,
      port, appData, bytes);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_free_buffer
* Parameters: hComp, port, buffer
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_free_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
   (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);

  if(lthis){
    lrc = lthis->omx_component_free_buffer(hComp, port, buffer);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_empty_this_buffer
* Parameters: hComp, buffer
* Return Value : OMX_ERRORTYPE
* Description: The client makes this call with the input buffer indicating the
* component to start processing the input buffer
==============================================================================*/
OMX_ERRORTYPE qomx_component_empty_this_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);

  if(lthis){
    lrc = lthis->omx_component_empty_this_buffer(hComp, buffer);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_fill_this_buffer
* Parameters: hComp, buffer
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_fill_this_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE *buffer)
{

  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);

  if(lthis){
    lrc = lthis->omx_component_fill_this_buffer(hComp, buffer);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_set_callbacks
* Parameters: hComp, buffer
* Return Value : OMX_ERRORTYPE
* Description: Set the callback function from the OMX client
==============================================================================*/
OMX_ERRORTYPE qomx_component_set_callbacks(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_CALLBACKTYPE *callbacks,
  OMX_IN OMX_PTR appData)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);

  if(lthis){
  	QIDBG_ERROR("%s: %d: This ptr addr %p\n",__func__, __LINE__, lthis);
    lrc = lthis->omx_component_set_callbacks(hComp, callbacks, appData);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_deinit
* Parameters: hComp
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_deinit(OMX_IN OMX_HANDLETYPE hComp)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  OMX_STATETYPE lstate;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);

  if(lthis){
    lthis->omx_component_get_state(hComp, &lstate);
    QIDBG_HIGH("%s %d: Component State: %d Calling Deinit", __func__, __LINE__,
      (int)lstate);
    lrc = lthis->omx_component_deinit(hComp);
    delete(lthis);
  }

  return lrc;
}

/*==============================================================================
* Function : qomx_component_get_parameter
* Parameters: hComp, paramIndex, paramData
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_get_parameter(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE paramIndex,
  OMX_INOUT OMX_PTR paramData)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_get_parameter(hComp, paramIndex, paramData);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_set_parameter
* Parameters: hComp, paramIndex, paramData
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_set_parameter(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE paramIndex,
  OMX_IN OMX_PTR paramData)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_set_parameter(hComp, paramIndex, paramData);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_get_config
* Parameters: hComp, configIndex, configData
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/

OMX_ERRORTYPE qomx_component_get_config(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE configIndex,
  OMX_INOUT OMX_PTR configData)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_get_config(hComp, configIndex, configData);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_set_config
* Parameters: hComp, configIndex, configData
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_set_config(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE configIndex,
  OMX_IN OMX_PTR configData)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_set_config(hComp, configIndex, configData);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_get_state
* Parameters: hComp, state
* Return Value : OMX_ERRORTYPE
* Description: Get the current state of the OMX component
==============================================================================*/
OMX_ERRORTYPE qomx_component_get_state(OMX_IN OMX_HANDLETYPE  hComp,
 OMX_OUT OMX_STATETYPE *state)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_get_state(hComp, state);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_send_command
* Parameters: hComp, param1, cmdData
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_send_command(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_COMMANDTYPE cmd,
  OMX_IN OMX_U32 param1,
  OMX_IN OMX_PTR cmdData)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_send_command(hComp, cmd, param1, cmdData);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_get_version
* Parameters: hComp, componentName, componentVersion, specVersion, componentUUID
* Return Value : OMX_ERRORTYPE
* Description: Get the OMX Specification version being used
==============================================================================*/
OMX_ERRORTYPE qomx_component_get_version(OMX_IN OMX_HANDLETYPE hComp,
  OMX_OUT OMX_STRING componentName,
  OMX_OUT OMX_VERSIONTYPE *componentVersion,
  OMX_OUT OMX_VERSIONTYPE *specVersion,
  OMX_OUT OMX_UUIDTYPE *componentUUID)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_get_version(hComp, componentName,
     componentVersion, specVersion, componentUUID);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_get_extension_index
* Parameters: hComp, paramName, indexType
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_get_extension_index(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_STRING paramName,
  OMX_OUT OMX_INDEXTYPE *indexType)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_get_extension_index(hComp, paramName,indexType);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_tunnel_request
* Parameters: hComp, port, peerComponent, peerPort, tunnelSetup
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_tunnel_request(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_HANDLETYPE peerComponent,
  OMX_IN OMX_U32 peerPort,
  OMX_INOUT OMX_TUNNELSETUPTYPE *tunnelSetup)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_tunnel_request(hComp, port, peerComponent,
      peerPort, tunnelSetup);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_use_buffer
* Parameters: bufferHdr, port, appData, bytes, buffer
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_use_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE **bufferHdr,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 bytes,
  OMX_IN OMX_U8 *buffer)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_use_buffer(hComp, bufferHdr, port, appData,
      bytes, buffer);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_use_EGL_image
* Parameters: hComp, bufferHdr, port, appData, eglImage
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_use_EGL_image(OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE **bufferHdr,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN void *eglImage)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_use_EGL_image(hComp, bufferHdr, port,
      appData, eglImage);
  }
  return lrc;
}

/*==============================================================================
* Function : qomx_component_role_enum
* Parameters: hComp, role, index
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE qomx_component_role_enum(OMX_IN OMX_HANDLETYPE hComp,
  OMX_OUT OMX_U8 *role,
  OMX_IN OMX_U32 index)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  QOMXImageCodec *lthis =
    (QOMXImageCodec *)(((OMX_COMPONENTTYPE *)hComp)->pComponentPrivate);
  if(lthis){
    lrc = lthis->omx_component_role_enum(hComp, role, index);
  }
  return lrc;
}

