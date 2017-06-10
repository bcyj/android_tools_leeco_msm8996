/*******************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#include <QIThread.h>
#include <OMX_Index.h>
#include <QOMXImageCodec.h>
#include <QExifParser.h>

/**
 *  MACROS and CONSTANTS
 **/
#define NUM_OF_PORTS 3
#define MAX_IMAGE_WIDTH 8192
#define MAX_IMAGE_HEIGHT 8192

/*===========================================================================
 * Class: OMXImageDecoder
 *
 * Description: This class represents base class of all OMX decoder
 *             components
 *
 * Notes: none
 *==========================================================================*/
class OMXImageDecoder : public QOMXImageCodec, public QThreadObject
{

protected:

  /** port_index
   *  OMX_INPUT_PORT_INDEX: input port index
   *  OMX_OUTPUT_PORT_INDEX: output port index
   *  OMX_INPUT_THUMBNAIL_PORT_INDEX: thumbnail port index
   *
   *  Enum for OMX port indices
   **/
  enum port_index
  {
    OMX_INPUT_PORT_INDEX = 0,
    OMX_OUTPUT_PORT_INDEX = 1,
    OMX_INPUT_THUMBNAIL_PORT_INDEX = 2
  };

  /** OMXImageDecoder
   *
   *
   **/
  OMXImageDecoder();

  /** ~OMXImageDecoder
   *
   *
   **/
  virtual ~OMXImageDecoder();

 /** decodeImage
   *  @a_inBuffer: input buffer header
   *  @a_outBuffer: output buffer header
   *
   *  Start Image Encoding
   **/
  virtual OMX_ERRORTYPE decodeImage(OMX_BUFFERHEADERTYPE *a_inBuffer,
    OMX_BUFFERHEADERTYPE *a_outBuffer) = 0;

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
    OMX_INOUT OMX_PTR paramData) ;

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
    OMX_IN OMX_PTR paramData) ;

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
    OMX_INOUT OMX_PTR configData);

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
    OMX_INOUT OMX_PTR configData);

  /** omx_component_init:
   *  @hComp: component handle
   *
   *  This function is used to initialize the component
   **/
  virtual OMX_ERRORTYPE omx_component_init(OMX_IN OMX_HANDLETYPE hComp);

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
  virtual OMX_ERRORTYPE omx_component_use_buffer(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
    OMX_IN OMX_U32 port,
    OMX_IN OMX_PTR appData,
    OMX_IN OMX_U32 bytes,
    OMX_IN OMX_U8* buffer);

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
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr, OMX_IN OMX_U32 port,
    OMX_IN OMX_PTR appData,
    OMX_IN OMX_U32 bytes);

  /** omx_component_free_buffer:
   *  @hComp: component handle
   *  @port: port type
   *  @buffer: buffer header
   *
   *  This function is used to free the buffers
   **/
  virtual OMX_ERRORTYPE omx_component_free_buffer(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_U32 port,
    OMX_IN OMX_BUFFERHEADERTYPE* buffer) ;

  /** omx_component_role_enum:
   *  @hComp: component handle
   *  @role: role of the component
   *  @index: role index
   *
   *  This function is used to enumerate the roles of the
   *  component
   **/
  virtual OMX_ERRORTYPE omx_component_role_enum(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_OUT OMX_U8 *role,
    OMX_IN OMX_U32 index);

  /** initializeOutputPort
   * a_outPort: port definition
   *
   * This function is used to initialize output port
   **/
  virtual void initializeOutputPort(OMX_PARAM_PORTDEFINITIONTYPE *a_outPort);

  /** initializeInputPort
   * a_outPort: port definition
   *
   * This function is used to initialize input port
   **/
  virtual void initializeInputPort(OMX_PARAM_PORTDEFINITIONTYPE *a_inPort);

  /** Execute
   *
   *  This method is inherited from the QIThreadObject class and
   *  is called from the StartThread method of the thread class as
   *  soon as a startthread is called by the object on a new
   *  thread.
   **/
  void Execute();

  /** portEnable
   * @a_portIndex: port index
   *
   *  This method is used to enable the port
   **/
  void portEnable(OMX_U32 a_portIndex);

  /** portDisable
   * @a_portIndex: port index
   *
   *  Disable the specified port and return any buffers that the
   *  port is holding throught ETB Done or FTB Done. Wait for
   *  OMX_Free_buffer to be complete to finish disabling the port.
   **/
  void portDisable(OMX_U32 a_portIndex);

  /** Start
   *
   *  Start encode/decode of the image. This method is called when
   *  the etb and ftb are done and the component is in the execute
   *  state. Dequeue the buffers from the etb and ftb queue and
   *  call encode/decode.
   **/
  virtual OMX_ERRORTYPE Start();


  /** processMessage
   *  @a_Message: message object
   *
   *  This function is called in the context of handler thread and
   *  is used to process the message
   **/
  OMX_ERRORTYPE processMessage(QIMessage *a_Message);

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
  OMX_ERRORTYPE omx_component_fill_this_buffer(
    OMX_IN OMX_HANDLETYPE hComp,
    OMX_IN OMX_BUFFERHEADERTYPE* buffer);

  /** omx_component_deinit:
   *  @hComp: component handle
   *
   *  This function is used to uninitialize the component
   **/
  OMX_ERRORTYPE omx_component_deinit(OMX_IN OMX_HANDLETYPE hComp);
 /** startEncode
   *
   *  Get the encoder from the factory and start encoding
   **/
  virtual OMX_ERRORTYPE startDecode() = 0;

  /** abortMessageThread
   *
   *  This function will abort the message handler thread
   **/
  void abortMessageThread();

  /** decodeImageHeader
   *  @a_inBuffer: input buffer header
   *  @a_outBuffer: output buffer header
   *
   *  Extract the image header
   **/
  virtual OMX_ERRORTYPE decodeImageHeader(OMX_BUFFERHEADERTYPE
      *a_inBuffer) = 0;

  /** ProcessInputBuffer
   *
   *  Start processing the input buffer. This method is called when
   *  the etb is done and the component is in the execute
   *  state. Dequeue the buffers from the etbQ and
   *  call decodeImageHeader.
   **/
  virtual OMX_ERRORTYPE ProcessInputBuffer();

private:

  /** use_input_buffer
   *  @bufferHdr: buffer header
   *  @bytes: buffer length
   *  @buffer: buffer address
   *  @appData: application private data
   *  @port: port type
   *
   *  Set the Input buffers passed by the OMX client
   **/
  virtual OMX_ERRORTYPE use_input_buffer(
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
    OMX_U32 bytes, OMX_U8* buffer,
    OMX_IN OMX_PTR appData, OMX_U32 port);

  /** use_output_buffer
   *  @bufferHdr: buffer header
   *  @bytes: buffer length
   *  @buffer: buffer address
   *  @appData: application private data
   *  @port: port type
   *
   *  Set the Input buffers passed by the OMX client
   **/
  virtual OMX_ERRORTYPE use_output_buffer(
    OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
    OMX_U32 bytes, OMX_U8* buffer,
    OMX_IN OMX_PTR appData, OMX_U32 port);

  /** abortExecution
   *
   *  Handle the component moving to from Executing/Pause state to
   *  idle state. Abort the current encoding session and return
   *  the unprocessed buffers.
   **/
  OMX_ERRORTYPE abortExecution();

  /** CanFreeBuffers
   *  @aPort: port info
   *  @aBuffer: buffer header
   *  @pBufferData: buffer data
   *
   *  This function is used to check if all the buffers are passed
   *  to the component.
   **/
  OMX_BOOL CanFreeBuffers(OMX_PARAM_PORTDEFINITIONTYPE *aPort,
    OMX_BUFFERHEADERTYPE *aBuffer, QOMX_Buffer_Data_t *pBufferData);

  /** flushBufferQueues
   *  @a_portIndex: port index
   *
   *  Return the unprocessed buffers associated with the port.
   *  This can be called when the client calls command flush,
   *  during transition to OMX_StateIdle from Executing/Pause or
   *  transition to OMX_StateInvalid or when the port is disabled.
   **/
  virtual OMX_ERRORTYPE flushBufferQueues(OMX_U32 a_portIndex);

  /** handleCommandFlush
   *  @a_portIndex: port index
   *
   *  Handle the Flush command from the omx client. Calls
   *  flush_Buffer_Queues to flush out unprocessed buffers
   *  associated with a_portIndex
   **/
  virtual void handleCommandFlush(OMX_U32 a_portIndex);

  /** releaseCurrentSession
   *
   *  Release all buffers associated with the current snapshot.
   *  Encoding shold be stopped before calling this function. If
   *  not it will lead to a crash.
   **/
  virtual OMX_ERRORTYPE releaseCurrentSession() = 0;

  /** portStateIsOk
   * @port: port to check
   *
   * Returns OMX_TRUE if set parameter on port is allowed,
   * OMX_FALSE otherwise.
   *
   */
  OMX_BOOL portStateIsOk(OMX_PARAM_PORTDEFINITIONTYPE *port);
protected:

  /** m_inputBufferCount
   *
   *  Input buffer count
   **/
  OMX_U32 m_inputBufferCount;

  /** m_outputBufferCount
   *
   *  output buffer count
   **/
  OMX_U32 m_outputBufferCount;

  /** m_messageThread
   *
   *  Message thread
   **/
  QIThread m_messageThread;

  /** m_imageDecoding
   *
   *  Flag indicating if decoding is in progress
   **/

  OMX_BOOL m_imageDecoding;

  /** m_mainEncoder
   *
   *  Main encoder object
   **/
  QImageDecoderInterface *m_mainDecoder;

  /** m_mainEncodeParams
   *
   *  Main image encoder parameters
   **/
  QIDecodeParams m_decodeParams;

  /** m_composer
   *
   *  Exif composer object
   **/
  QExifParser *m_parser;

  /** m_inputImage
   *
   *  Main input image
   **/
  QImage *m_inputImage;

  /** m_outputMainImage
   *
   *  Main output image
   **/
  QImage *m_outputImage;

  /** m_outImgFormat
   *
   *  Output image format as extracted from header
   **/
  QIFormat m_outImgFormat;

  /** m_outImgFormat
   *
   *  Output image subsampling as extracted from header
   **/
  QISubsampling m_outImgSubsampling;

  /** m_outImgSize
   *
   *  Output image size as extracted from header
   **/
  QISize m_outImgSize;

  /** m_outImgLength
   *
   *  Output image length as extracted from header
   **/
  OMX_U32 m_outImgLength;

};
