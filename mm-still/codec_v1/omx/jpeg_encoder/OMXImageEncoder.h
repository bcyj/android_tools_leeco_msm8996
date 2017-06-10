/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include <QOMXImageCodec.h>
#include <QIThread.h>
#include <OMX_Index.h>
#include <QExifComposer.h>
#include <QMobicatComposer.h>
#include "exif.h"
#include "QIONBuffer.h"

/**
 *  MACROS and CONSTANTS
 **/
#define NUM_OF_PORTS 3
#define MAX_IMAGE_WIDTH 8192
#define MAX_IMAGE_HEIGHT 8192
#define MAX_PARSED_METADATA_SIZE (4096)

/*===========================================================================
 * Class: OMXImageEncoder
 *
 * Description: This class represents base class of all OMX encoder
 *             components
 *
 * Notes: none
 *==========================================================================*/
class OMXImageEncoder : public QOMXImageCodec, public QThreadObject
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

  /** OMXImageEncoder
   *
   *
   **/
  OMXImageEncoder();

  /** ~OMXImageEncoder
   *
   *
   **/
  virtual ~OMXImageEncoder();

  /** encodeImage
   *  @a_inBuffer: input buffer header
   *  @a_outBuffer: output buffer header
   *
   *  Start Image Encoding
   **/
  virtual OMX_ERRORTYPE encodeImage(OMX_BUFFERHEADERTYPE *a_inBuffer,
    OMX_BUFFERHEADERTYPE *m_currentInTmbBuffHdr,
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
    OMX_IN OMX_PTR configData);

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
  virtual OMX_ERRORTYPE startEncode() = 0;

  /** abortMessageThread
   *
   *  This function will abort the message handler thread
   **/
  void abortMessageThread();

  /** getEstimatedExifSize
   *
   * return an estimated size of the EXIF
   **/
  OMX_U32 getEstimatedExifSize();


private:

  /** getPortDef
   *  @eIdx: Port index
   *  Get pointer to the port definition corresponding to the given index
   **/
  OMX_PARAM_PORTDEFINITIONTYPE *getPortDef(OMX_U32 eIdx);

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
    OMX_IN OMX_PTR appData, OMX_IN OMX_U32 port);

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

  virtual OMX_ERRORTYPE releaseCodecLibs() = 0;

  /** setExifData
   *
   *  Initialize the exif Info Object and set the exif Data.
   **/
  int setExifData(QOMX_EXIF_INFO *a_exifInfo);

  virtual OMX_ERRORTYPE preloadCodecLibs() = 0;

protected:

  /** m_inputQTmbBuffer
   *
   *  thumbnail input buffer
   **/
  QIBuffer *m_inputQTmbBuffer;

  /** m_inBuffAllocCount
   *
   *  Count of the number if input buffer headers allocated during
   *  Usebuffer
   **/
  OMX_U32 m_inTmbBuffAllocCount;

   /** mInTmbOMXBufferData
   *
   *  Array of all the input buffer headers created during
   *  UseBuffer
   **/
  QOMX_Buffer_Data_t *mInTmbOMXBufferData;

  /** m_currentInTmbBuffHdr
   *
   *  Buffer header of the current input buffer being processed
   *  passed during empty_this_buffer
   **/
  OMX_BUFFERHEADERTYPE *m_currentInTmbBuffHdr;

  /** m_etbQueue
   *
   *  thumbnail input buffer queue
   **/
  QIQueue m_etbTmbQueue;

  /** m_etbQLock
   *
   *  lock for the input thumbnail buffer queue
   **/
  pthread_mutex_t m_etbTmbQLock;

  /** mEncodeDoneLock
   *
   *  lock for EncodeComplete function
   **/
  pthread_mutex_t mEncodeDoneLock;

  /** m_inTmbPortTransState
   *
   *  Tmb input port intermediate state
   **/
  qomx_intermediate_port_state_t m_inTmbPortTransState;

  /** m_inputTmbFormatTypes
   *
   *  Tmb input port format types
   **/
   OMX_IMAGE_PARAM_PORTFORMATTYPE *m_inputTmbFormatTypes;

   /** m_inTmbPort
   *
   *  Thumbnail input port definition
   **/
  OMX_PARAM_PORTDEFINITIONTYPE *m_inTmbPort;

  /** m_qualityfactor
   *
   * Image quality factor
   **/
  OMX_IMAGE_PARAM_QFACTORTYPE m_qualityfactor;

  /** m_huffmanTable
   *
   *  huffman table
   **/
  OMX_IMAGE_PARAM_HUFFMANTTABLETYPE m_huffmanTable;

  /** m_quantTable
   *
   *  Quantization table
   **/
  OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE m_quantTable;

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

  /** m_imageBufferOffset
   *
   *  Main image info
   **/
  QOMX_YUV_FRAME_INFO m_imageBufferOffset;

  /** m_thumbnailInfo
   *
   *  Thumbnail image info
   **/
  QOMX_THUMBNAIL_INFO m_thumbnailInfo;

  /** m_mainEncodeParams
   *
   *  Main image encoder parameters
   **/
  QIEncodeParams m_mainEncodeParams;

  /** m_thumbEncodeParams
   *
   *  Thumbnail encode parameters
   **/
  QIEncodeParams m_thumbEncodeParams;

  /** m_composer
   *
   *  Exif composer object
   **/
  QExifComposer *m_composer;

  /** m_exifParams
   *
   *  Exif composer parameters
   **/
  QExifComposerParams m_exifParams;

  /** m_mobicatComposer
   *
   *  Mobicat composer parameters
   **/
  QMobicatComposer *m_mobicatComposer;

  /** m_mainEncoder
   *
   *  Main encoder object
   **/
  QImageEncoderInterface *m_mainEncoder;

  /** m_thumbEncoder
   *
   *  Thumbnail encoder object
   **/
  QImageEncoderInterface *m_thumbEncoder;

  /** m_inputMainImage
   *
   *  Main input image
   **/
  QImage *m_inputMainImage;

  /** m_outputMainImage
   *
   *  Main output image
   **/
  QImage *m_outputMainImage;

  /** m_inThumbImage
   *
   *  Thumbnail input image
   **/
  QImage *m_inThumbImage;

  /** m_outThumbImage
   *
   *  thumbnail output image
   **/
  QImage *m_outThumbImage;

  /** m_thumbEncoding
   *
   * Flag to indicate whether thumbnail encoding is in progress
   **/
  OMX_U8 m_thumbEncoding;

  /** m_thumbEncodingComplete
   *
   * Flag to indicate whether thumbnail encoding is completed.
   * This flag will only be used in parallel encoding
   **/
  OMX_BOOL m_thumbEncodingComplete;

  /** m_mainEncodingComplete
   *
   * Flag to indicate whether main encoding is completed.
   * This flag will only be used in parallel encoding
   **/
  OMX_BOOL m_mainEncodingComplete;

  /** m_mainImageEncoding
   *
   * Flag to indicate whether main image encoding is in progress
   **/
  OMX_U8 m_mainImageEncoding;

  /** m_exifInfoObj
   *
   *  Array of Exif info structures
   **/
  exif_info_obj_t m_exifInfoObj;

  /** mExifObjInitialized
   *
   *  Flag to indicate whether the exif object is initialized
   **/
  OMX_BOOL mExifObjInitialized;

  /** mThumbBuffer
   *
   *  Store the thumbnail buffer
   **/
  QIHeapBuffer *mThumbBuffer;

  /** m_releaseFlag
   *
   * Flag to indicate whether release is in progress
   **/
  OMX_BOOL m_releaseFlag;

  /** m_encoding_mode
   *
   *  Flag to indicate if serial or parallel encoding
   **/
  QOMX_ENCODING_MODE m_encoding_mode;

  /** m_inputTmbSize
   *
   * Size of the input thumbnail image
   **/
  QISize m_inputTmbSize;

  /** m_inputTmbPaddedSize
   *
   *  input thumbnail padded image size
   **/
  QISize m_inputTmbPadSize;

  /**mIONBuffer
   *
   * Work buffer to be used for hardware encoding
   **/
  QOMX_WORK_BUFFER m_IONBuffer;

  /**m_Metadatadata
   *
   * Metadata data to be set in EXIF
   **/
  QOMX_METADATA m_Metadata;

  /**m_MetadataEncKey
   *
   * Encryption key for makernote data
   */
  QOMX_META_ENC_KEY m_MetadataEncKey;

  /** m_memOps
   *
   * Buffer memory operations
   **/
  QOMX_MEM_OPS m_memOps;

  /** mNumExifMarkers
   *
   * Number of APP markers in Exif
   **/
  int mNumExifMarkers;

  /**m_JpegSpeedMode
   *
   * Jpeg HW encoder speed setting
   */
  QOMX_JPEG_SPEED m_JpegSpeedMode;

};
