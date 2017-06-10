/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#ifndef QC_CORE_OMX_COMPONENT_H
#define QC_CORE_OMX_COMPONENT_H

#ifdef __cplusplus
extern "C" {
#endif

/** qomx_component_get_version:
 *  @hComp: component handle
 *  @componentName: component name
 *  @componentVersion: component version
 *  @specVersion: OMX spec version
 *  @componentUUID: unique id for component
 *
 *  This function is used to get the version of the component
 **/
OMX_ERRORTYPE qomx_component_get_version(OMX_IN OMX_HANDLETYPE hComp,
  OMX_OUT OMX_STRING componentName,
  OMX_OUT OMX_VERSIONTYPE* componentVersion,
  OMX_OUT OMX_VERSIONTYPE* specVersion,
  OMX_OUT OMX_UUIDTYPE* componentUUID);

/** qomx_component_send_command:
 *  @hComp: component handle
 *  @cmd: command type
 *  @param1: command parameter
 *  @cmdData: command data
 *
 *  This function is used to send command to the component
 **/
OMX_ERRORTYPE qomx_component_send_command(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_COMMANDTYPE cmd,
  OMX_IN OMX_U32 param1,
  OMX_IN OMX_PTR cmdData);

/** qomx_component_get_parameter:
 *  @hComp: component handle
 *  @paramIndex: parameter index
 *  @paramData: parameter data
 *
 *  This function is used to get parameter from the component
 **/
OMX_ERRORTYPE qomx_component_get_parameter(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE paramIndex,
  OMX_INOUT OMX_PTR paramData);

/** qomx_component_set_parameter:
 *  @hComp: component handle
 *  @paramIndex: parameter index
 *  @paramData: parameter data
 *
 *  This function is used to set parameter to the component
 **/
OMX_ERRORTYPE qomx_component_set_parameter(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE paramIndex,
  OMX_IN OMX_PTR paramData);

/** qomx_component_get_config:
 *  @hComp: component handle
 *  @configIndex: config param index
 *  @configData: config data pointer
 *
 *  This function is used to get the config from the component
 **/
OMX_ERRORTYPE qomx_component_get_config(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE configIndex,
  OMX_INOUT OMX_PTR configData);

/** qomx_component_set_config:
 *  @hComp: component handle
 *  @configIndex: config param index
 *  @configData: config data pointer
 *
 *  This function is used to set the config to the component
 **/
OMX_ERRORTYPE qomx_component_set_config(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE configIndex,
  OMX_IN OMX_PTR configData);

/** qomx_component_get_extension_index:
 *  @hComp: component handle
 *  @paramName: name of the parameter
 *  @indexType: extension index
 *
 *  Gets the extension index of the parameter
 **/
OMX_ERRORTYPE qomx_component_get_extension_index(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_STRING paramName,
  OMX_OUT OMX_INDEXTYPE* indexType);

/** qomx_component_get_state:
 *  @hComp: component handle
 *  @state: state of the component
 *
 *  Gets the state of the component
 **/
OMX_ERRORTYPE qomx_component_get_state(OMX_IN OMX_HANDLETYPE hComp,
  OMX_OUT OMX_STATETYPE* state);

/** qomx_component_tunnel_request:
 *  @hComp: component handle
 *  @port: port type
 *  @peerComponent: peer component
 *  @peerPort: peer port type
 *  @tunnelSetup: pointer to tunnel setup structure
 *
 *  Sets up the tunneling for the component
 **/
OMX_ERRORTYPE qomx_component_tunnel_request(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_HANDLETYPE peerComponent,
  OMX_IN OMX_U32 peerPort,
  OMX_INOUT OMX_TUNNELSETUPTYPE* tunnelSetup);

/** qomx_component_use_buffer:
 *  @hComp: component handle
 *  @bufferHdr: buffer header
 *  @port: port type
 *  @appData: application private data
 *  @bytes: buffer length
 *  @buffer: buffer address
 *
 *  This function is used to send the buffers allocated by the
 *  client
 **/
OMX_ERRORTYPE qomx_component_use_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 bytes,
  OMX_IN OMX_U8* buffer);

/** qomx_component_allocate_buffer:
 *  @hComp: component handle
 *  @bufferHdr: buffer header
 *  @port: port type
 *  @appData: application private data
 *  @bytes: buffer length
 *
 *  This function is used to request the component to allocate
 *  the buffers
 **/
OMX_ERRORTYPE qomx_component_allocate_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr, OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 bytes);

/** qomx_component_free_buffer:
 *  @hComp: component handle
 *  @port: port type
 *  @buffer: buffer header
 *
 *  This function is used to free the buffers
 **/
OMX_ERRORTYPE qomx_component_free_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer);

/** qomx_component_empty_this_buffer:
 *  @hComp: component handle
 *  @buffer: buffer header
 *
 *  This function is used to send the input buffers to the
 *  client for processing
 **/
OMX_ERRORTYPE qomx_component_empty_this_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer);

/** qomx_component_fill_this_buffer:
 *  @hComp: component handle
 *  @buffer: buffer header
 *
 *  This function is used to send the output buffers to the
 *  client for processing
 **/
OMX_ERRORTYPE qomx_component_fill_this_buffer(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer);

/** qomx_component_set_callbacks:
 *  @hComp: component handle
 *  @callbacks: callback pointer
 *  @appData: application data
 *
 *  This function is used to set the callback function pointers
 **/
OMX_ERRORTYPE qomx_component_set_callbacks(OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_CALLBACKTYPE* callbacks,
  OMX_IN OMX_PTR appData);

/** qomx_component_init:
 *  @hComp: component handle
 *
 *  This function is used to initialize the component
 **/
OMX_ERRORTYPE qomx_component_init(OMX_IN OMX_HANDLETYPE hComp);

/** qomx_component_deinit:
 *  @hComp: component handle
 *
 *  This function is used to uninitialize the component
 **/
OMX_ERRORTYPE qomx_component_deinit(OMX_IN OMX_HANDLETYPE hComp);

/** qomx_component_use_EGL_image:
 *  @hComp: component handle
 *  @bufferHdr: buffer header
 *  @port: port type
 *  @appData: application private data
 *  @eglImage: EGL image pointer
 *
 *  This function is used to send the EGL image to the component
 **/
OMX_ERRORTYPE qomx_component_use_EGL_image(OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN void* eglImage);

/** qomx_component_role_enum:
 *  @hComp: component handle
 *  @role: role of the component
 *  @index: role index
 *
 *  This function is used to enumerate the roles of the
 *  component
 **/
OMX_ERRORTYPE qomx_component_role_enum(OMX_IN OMX_HANDLETYPE hComp,
  OMX_OUT OMX_U8* role,
  OMX_IN OMX_U32 index);

/** create_component_fns:
 *  @aobj: object pointer
 *
 *  This function creates the component functions and retuns the
 *  object pointer
 **/
void *create_component_fns(OMX_PTR aobj);

#ifdef __cplusplus
}
#endif

#endif
