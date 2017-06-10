/*******************************************************************************
* Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#ifndef QOMX_COMPONENT_H
#define QOMX_COMPONENT_H

#include <ctype.h>
#include <semaphore.h>
#include <qomx_core.h>
#include "OMX_Component.h"
#include "QOMX_JpegExtensions.h"
#include <QIQueue.h>
#include <QIMessage.h>
#include <QIHeapBuffer.h>
#include <QOMX_Buffer.h>
#include <QImage.h>
#include <QImageCodecFactoryB.h>
#include <QImageCodecFactoryA.h>

/*===========================================================================
 * Class: QOMXImageCodec
 *
 * Description: This class represents base class of all OMX image codec
 *             components
 *
 * Notes: none
 *==========================================================================*/
class QOMXImageCodec {

public:

  /** QOMXImageCodec
   *
   *  constructor
   **/
  QOMXImageCodec();

  /** ~QOMXImageCodec
   *
   *  destructor
   **/
  virtual ~QOMXImageCodec();

  /** omx_component_get_version:
   *  @hComp: component handle
   *  @componentName: component name
   *  @componentVersion: component version
   *  @specVersion: OMX spec version
   *  @componentUUID: unique id for component
   *
   *  This function is used to get the version of the component
   **/
  virtual OMX_ERRORTYPE omx_component_get_version(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_OUT OMX_STRING componentName,
    OMX_OUT OMX_VERSIONTYPE* componentVersion,
    OMX_OUT OMX_VERSIONTYPE* specVersion,
    OMX_OUT OMX_UUIDTYPE* componentUUID);

  /** omx_component_send_command:
   *  @hComp: component handle
   *  @cmd: command type
   *  @param1: command parameter
   *  @cmdData: command data
   *
   *  This function is used to send command to the component
   **/
  virtual OMX_ERRORTYPE omx_component_send_command(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_COMMANDTYPE cmd,
    OMX_IN OMX_U32 param1,
    OMX_IN OMX_PTR cmdData);

  /** omx_component_get_parameter:
   *  @hComp: component handle
   *  @paramIndex: parameter index
   *  @paramData: parameter data
   *
   *  This function is used to get parameter from the component
   **/
  virtual OMX_ERRORTYPE omx_component_get_parameter(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_INDEXTYPE paramIndex,
    OMX_INOUT OMX_PTR paramData) = 0;

  /** omx_component_set_parameter:
   *  @hComp: component handle
   *  @paramIndex: parameter index
   *  @paramData: parameter data
   *
   *  This function is used to set parameter to the component
   **/
  virtual OMX_ERRORTYPE omx_component_set_parameter(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_INDEXTYPE paramIndex,
    OMX_IN OMX_PTR paramData) = 0;

  /** omx_component_get_config:
   *  @hComp: component handle
   *  @configIndex: config param index
   *  @configData: config data pointer
   *
   *  This function is used to get the config from the component
   **/
  virtual OMX_ERRORTYPE omx_component_get_config(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_INDEXTYPE configIndex,
    OMX_INOUT OMX_PTR configData) = 0;

  /** omx_component_set_config:
   *  @hComp: component handle
   *  @configIndex: config param index
   *  @configData: config data pointer
   *
   *  This function is used to set the config to the component
   **/
  virtual OMX_ERRORTYPE omx_component_set_config(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_INDEXTYPE configIndex,
    OMX_IN OMX_PTR configData) = 0;

  /** omx_component_get_extension_index:
   *  @hComp: component handle
   *  @paramName: name of the parameter
   *  @indexType: extension index
   *
   *  Gets the extension index of the parameter
   **/
  virtual OMX_ERRORTYPE omx_component_get_extension_index(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_STRING paramName,
    OMX_OUT OMX_INDEXTYPE* indexType);

  /** omx_component_get_state:
   *  @hComp: component handle
   *  @state: state of the component
   *
   *  Gets the state of the component
   **/
   OMX_ERRORTYPE omx_component_get_state(OMX_IN OMX_HANDLETYPE hComp,
    OMX_OUT OMX_STATETYPE* state);

  /** omx_component_tunnel_request:
   *  @hComp: component handle
   *  @port: port type
   *  @peerComponent: peer component
   *  @peerPort: peer port type
   *  @tunnelSetup: pointer to tunnel setup structure
   *
   *  Sets up the tunneling for the component
   **/
   OMX_ERRORTYPE omx_component_tunnel_request(OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_U32 port,
    OMX_IN OMX_HANDLETYPE peerComponent,
    OMX_IN OMX_U32 peerPort,
    OMX_INOUT OMX_TUNNELSETUPTYPE* tunnelSetup);

  /** omx_component_use_buffer:
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
  virtual OMX_ERRORTYPE omx_component_use_buffer(OMX_IN OMX_HANDLETYPE hComp,
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
    OMX_IN OMX_U32 port,
    OMX_IN OMX_PTR appData,
    OMX_IN OMX_U32 bytes,
    OMX_IN OMX_U8* buffer) = 0;

  /** omx_component_allocate_buffer:
   *  @hComp: component handle
   *  @bufferHdr: buffer header
   *  @port: port type
   *  @appData: application private data
   *  @bytes: buffer length
   *
   *  This function is used to request the component to allocate
   *  the buffers
   **/
  virtual OMX_ERRORTYPE omx_component_allocate_buffer(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
    OMX_IN OMX_U32 port,
    OMX_IN OMX_PTR appData,
    OMX_IN OMX_U32 bytes) = 0;

  /** omx_component_free_buffer:
   *  @hComp: component handle
   *  @port: port type
   *  @buffer: buffer header
   *
   *  This function is used to free the buffers
   **/
  virtual OMX_ERRORTYPE omx_component_free_buffer(OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_U32 port,
    OMX_IN OMX_BUFFERHEADERTYPE* buffer) = 0;

  /** omx_component_empty_this_buffer:
   *  @hComp: component handle
   *  @buffer: buffer header
   *
   *  This function is used to send the input buffers to the
   *  client for processing
   **/
  virtual OMX_ERRORTYPE omx_component_empty_this_buffer(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_BUFFERHEADERTYPE* buffer);

  /** omx_component_fill_this_buffer:
   *  @hComp: component handle
   *  @buffer: buffer header
   *
   *  This function is used to send the output buffers to the
   *  client for processing
   **/
  virtual OMX_ERRORTYPE omx_component_fill_this_buffer(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_BUFFERHEADERTYPE* buffer);

  /** omx_component_set_callbacks:
   *  @hComp: component handle
   *  @callbacks: callback pointer
   *  @appData: application data
   *
   *  This function is used to set the callback function pointers
   **/
  virtual OMX_ERRORTYPE omx_component_set_callbacks(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_CALLBACKTYPE* callbacks,
    OMX_IN OMX_PTR appData);

  /** omx_component_deinit:
   *  @hComp: component handle
   *
   *  This function is used to uninitialize the component
   **/
  virtual OMX_ERRORTYPE omx_component_deinit(OMX_IN OMX_HANDLETYPE hComp);

  /** omx_component_use_EGL_image:
   *  @hComp: component handle
   *  @bufferHdr: buffer header
   *  @port: port type
   *  @appData: application private data
   *  @eglImage: EGL image pointer
   *
   *  This function is used to send the EGL image to the component
   **/
  OMX_ERRORTYPE omx_component_use_EGL_image(OMX_IN OMX_HANDLETYPE hComp,
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
    OMX_IN OMX_U32 port,
    OMX_IN OMX_PTR appData,
    OMX_IN void* eglImage);

  /** omx_component_role_enum:
   *  @hComp: component handle
   *  @role: role of the component
   *  @index: role index
   *
   *  This function is used to enumerate the roles of the
   *  component
   **/
  virtual OMX_ERRORTYPE omx_component_role_enum(OMX_IN OMX_HANDLETYPE hComp,
    OMX_OUT OMX_U8 *role,
    OMX_IN OMX_U32 index) = 0;


  /** changeState
   *  @aState: new state
   *
   *  This function is used for state transition
   **/
  virtual void changeState(OMX_STATETYPE aState);

  /** omx_component_init:
   *  @hComp: component handle
   *
   *  This function is used to initialize the component
   **/
  virtual OMX_ERRORTYPE omx_component_init(OMX_IN OMX_HANDLETYPE hComp) = 0;

  /** initializeOutputPort
   * a_outPort: port definition
   *
   * This function is used to initialize output port
   **/
  virtual void initializeOutputPort(OMX_PARAM_PORTDEFINITIONTYPE *a_outPort)
    = 0;

  /** initializeInputPort
   * a_outPort: port definition
   *
   * This function is used to initialize input port
   **/
  virtual void initializeInputPort(OMX_PARAM_PORTDEFINITIONTYPE *a_inPort )= 0;

  /** postMessage
   *  a_Message: message object
   *
   *  This function is used to post the message to the message
   *  handler thread
   **/
  virtual OMX_ERRORTYPE postMessage(QIBase *a_Message);

  /** handleMessage
   *
   *  This function is called in the context of handler thread and
   *  is used to get the message from queue
   **/
  virtual OMX_ERRORTYPE handleMessage();

  /** processMessage
   *  @a_Message: message object
   *
   *  This function is called in the context of handler thread and
   *  is used to process the message
   **/
  virtual OMX_ERRORTYPE processMessage(QIMessage *a_Message)= 0;

  /** portEnable
   * @a_portIndex: port index
   *
   *  This method is used to enable the port
   **/
  virtual void portEnable(OMX_U32 a_portIndex) = 0;

  /** portDisable
   * @a_portIndex: port index
   *
   *  Disable the specified port and return any buffers that the
   *  port is holding throught ETB Done or FTB Done. Wait for
   *  OMX_Free_buffer to be complete to finish disabling the port.
   **/
  virtual void portDisable(OMX_U32 a_portIndex) = 0;

  /** Start
   *
   *  Start encode/decode of the image. This method is called when
   *  the etb and ftb are done and the component is in the execute
   *  state. Dequeue the buffers from the etb and ftb queue and
   *  call encode/decode.
   **/
  virtual OMX_ERRORTYPE Start();

  /** translateFormat
   *  @a_omxColorFormat: OMX color format
   *  @aformat: image format
   *  @asubsampling: image subsampling
   *
   *  This function translates the OMX color format to QIFormat
   *  and QISubsampling types to be passed to the lower layer.
   **/
  virtual OMX_ERRORTYPE translateFormat(OMX_COLOR_FORMATTYPE a_omxColorFormat,
    QIFormat *aformat, QISubsampling *asubsampling);

  /** getOmxFormat
   *  @a_omxColorFormat: OMX color format
   *  @aformat: image format
   *  @asubsampling: image subsampling
   *
   *  This function translates the QIFormat
   *  and QISubsampling types to OMX color format.
   **/
  virtual OMX_ERRORTYPE getOmxFormat(OMX_U32 *target,
      QIFormat aformat, QISubsampling asubsampling);

  /** emptyBufferDone
   *  @abufferHdr: buffer header
   *
   *  Call the Empty Buffer Done callback on the client once the
   *  input buffer has been processed.In case of the component
   *  state changing to INVALID or disabling of the input port,
   *  send back the buffers that are yet to be processed through
   *  EmptyBufferDone callback
   **/
  virtual void emptyBufferDone(OMX_BUFFERHEADERTYPE *abufferHdr);

  /** fillBufferDone
   *  @abufferHdr: buffer header
   *
   *  Call the Fill Buffer Done callback on the client once the
   *  output buffer has been filled after encoding/decoding.In
   *  case of the component state changing to INVALID or disabling
   *  of the output port, send back the buffers that are yet to be
   *  processed through FillBufferDone callback.
   **/
  virtual void fillBufferDone(OMX_BUFFERHEADERTYPE *abufferHdr);

  /** abortExecution
   *
   *  Handle the component moving to from Executing/Pause state to
   *  idle state. Abort the current encoding session and return
   *  the unprocessed buffers.
   **/
  virtual OMX_ERRORTYPE abortExecution();

  /** ComponentHandle
   *
   *  Inline function to get the component handle
   **/
  inline OMX_COMPONENTTYPE *ComponentHandle()
  {
    return &m_componentFns;
  }

  /** printPortData
   *  aPort: port info
   *
   *  Debug info for port
   **/
  static void printPortData(OMX_PARAM_PORTDEFINITIONTYPE *aPort);

  /** preloadCodecLibs
   *
   *  Preload codec libraries
   **/
  virtual OMX_ERRORTYPE preloadCodecLibs();

  /** preloadCodecLibs
   *
   *  Release codec libraries
   **/
  virtual OMX_ERRORTYPE releaseCodecLibs();
protected:

  /** m_componentFns
   *
   *  pointer to conponent functions
   **/
  OMX_COMPONENTTYPE m_componentFns;

  /** m_state
   *
   *  state of the base component
   **/
  OMX_STATETYPE m_state;

  /** m_callbacks
   *
   *  callback pointer
   **/
  OMX_CALLBACKTYPE *m_callbacks;

  /** m_compHandle
   *
   *  component handle
   **/
  OMX_HANDLETYPE m_compHandle;

  /** m_appData
   *
   *  pointer to application data
   **/
  OMX_PTR m_appData;

  /** m_queue
   *
   *  message queue
   **/
  QIQueue m_queue;

  /** m_queueLock
   *
   *  lock for the message queue
   **/
  pthread_mutex_t m_queueLock;

  /** m_etbQueue
   *
   *  input buffer queue
   **/
  QIQueue m_etbQueue;

  /** m_etbQLock
   *
   *  lock for the input buffer queue
   **/
  pthread_mutex_t m_etbQLock;

  /** m_ftbQueue
   *
   *  output buffer queue
   **/
  QIQueue m_ftbQueue;

  /** m_ftbQLock
   *
   *  lock for the output buffer queue
   **/
  pthread_mutex_t m_ftbQLock;

  /** m_imagePortParam
   *
   *  port parameters
   **/
  OMX_PORT_PARAM_TYPE *m_imagePortParam;

  /** m_inPort
   *
   *  input port definition
   **/
  OMX_PARAM_PORTDEFINITIONTYPE *m_inPort;

  /** m_outPort
   *
   *  output port definition
   **/
  OMX_PARAM_PORTDEFINITIONTYPE *m_outPort;

  /** m_inputFormatTypes
   *
   *  input format types
   **/
  OMX_IMAGE_PARAM_PORTFORMATTYPE *m_inputFormatTypes;

  /** m_outputFormatTypes
   *
   *  output format types
   **/
  OMX_IMAGE_PARAM_PORTFORMATTYPE *m_outputFormatTypes;

  /** m_compLock
   *
   *  component lock
   **/
  pthread_mutex_t m_compLock;

  /** m_cmdLock
   *
   *  command semaphore
   **/
  sem_t m_cmdLock;

  /** m_abortlock
   *
   *  abort lock
   **/
  pthread_mutex_t m_abortlock;

  /** m_queueCond
   *
   *  conditional variable for queue
   **/
  pthread_cond_t m_queueCond;

  /** m_abort_flag
   *
   *  abort flag
   **/
  OMX_U8 m_abort_flag;

  /** m_thread_exit_flag
   *
   *  indicates if thread can exit after abort
   **/
  OMX_U8 m_thread_exit_flag;

  /** m_compInitialized
   *
   *  flag to indicate if the component is initialized
   **/
  OMX_U8 m_compInitialized;

  /** m_compTransState
   *
   *  component intermediate state
   **/
  qomx_intermediate_comp_state_t m_compTransState;

  /** m_inportTransState
   *
   *  input port intermediate state
   **/
  qomx_intermediate_port_state_t m_inportTransState;

  /** m_outportTransState
   *
   *  output port intermediate state
   **/
  qomx_intermediate_port_state_t m_outportTransState;

  /** m_inputScaleInfo
   *
   *  input scale info
   **/
  OMX_CONFIG_RECTTYPE m_inputScaleInfo;

  /** m_outputScaleInfo
   *
   *  output scale info
   **/
  OMX_CONFIG_RECTTYPE m_outputScaleInfo;

  /** m_rotation
   *
   *  encoder rotation
   **/
  OMX_CONFIG_ROTATIONTYPE m_rotation;

  /** m_numOfPlanes
   *
   *  number of planes for input buffer
   **/
  OMX_U16 m_numOfPlanes;

  /** m_numOfComponents
   *
   *  number of color components for input buffer
   **/
  OMX_U16 m_numOfComponents;

  /** m_executionComplete
   *
   *  Indicates that the execution of the omx component is
   *  complete
   **/
  OMX_BOOL m_executionComplete;

  /** m_dataAllocated
   *
   *  Indicates that data for all enabled ports has been allocated
   **/
  OMX_BOOL m_dataAllocated;

  /** m_inBuffAllocCount
   *
   *  Count of the number if input buffer headers allocated during
   *  Usebuffer
   **/
  OMX_U32 m_inBuffAllocCount;

  /** m_outBuffAllocCount
   *
   *  Count of the number if output buffer headers allocated
   *  during Usebuffer
   **/
  OMX_U32 m_outBuffAllocCount;

  /** mInOMXBufferData
   *
   *  Array of all the input buffer headers created during
   *  UseBuffer
   **/
  QOMX_Buffer_Data_t *mInOMXBufferData;

  /** mOutOMXBufferData
   *
   *  Array of all the output buffer headers created during
   *  UseBuffer
   **/
  QOMX_Buffer_Data_t *mOutOMXBufferData;

  /** m_currentInBuffHdr
   *
   *  Buffer header of the current input buffer being processed
   *  passed during empty_this_buffer
   **/
  OMX_BUFFERHEADERTYPE *m_currentInBuffHdr;

  /** m_currentOutBuffHdr
   *
   *  Buffer header of the current output buffer being processed
   *  passed during fill_this_buffer
   **/
  OMX_BUFFERHEADERTYPE *m_currentOutBuffHdr;

  /** m_etbFlag
   *
   *  Flag to indicate empty this buffer has been called on all
   *  enabled i/p ports
   **/
  OMX_U8 m_etbFlag;

  /** m_ftbFlag
   *
   *  Flag to indicate empty this buffer has been called on all
   *  enabled o/p ports
   **/
  OMX_U8 m_ftbFlag;

  /** m_inputSize
   *
   *  input image size
   **/
  QISize m_inputSize;

  /** m_inputPaddedSize
   *
   *  input padded image size
   **/
  QISize m_inputPadSize;

  /** m_outputSize
   *
   *  output image size
   **/
  QISize m_outputSize[2];

  /** m_inputQIBuffer
   *
   *  input buffer
   **/
  QIBuffer *m_inputQIBuffer;

  /** m_outputQIBuffer
   *
   *  output buffer
   **/
  QIBuffer *m_outputQIBuffer;

  /** m_subsampling
   *
   *  image subsampling
   **/
  QISubsampling m_subsampling;

  /** m_format
   *
   *  image format
   **/
  QIFormat m_format;

  /** m_thumbSubsampling
   *
   *  thumb subsampling
   **/

  QISubsampling m_thumbSubsampling;

  /** m_thumbFormat
   *
   *  thumb format
   **/
  QIFormat m_thumbFormat;

  /** m_factory
   *
   *  Codec factory class
   **/
#ifdef CODEC_B
  QImageCodecFactoryB m_factory;
#else
  QImageCodecFactoryA m_factory;
#endif

  /** m_outputPadSize
   *
   *  output padded image size
   **/
  QISize m_outputPadSize;

  int use_cpu;

};

#endif
