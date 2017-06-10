/*******************************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/
#include "OMXImageDecoder.h"

/*==============================================================================
* Function : OMXImageDecoder
* Parameters: None
* Return Value : None
* Description: Constructor
==============================================================================*/
OMXImageDecoder::OMXImageDecoder()
{
  m_inputBufferCount = 0;
  m_outputBufferCount = 0;
  m_imageDecoding = OMX_FALSE;
}

/*==============================================================================
* Function : ~OMXImageDecoder
* Parameters: None
* Return Value : None
* Description: Destructor
==============================================================================*/
OMXImageDecoder::~OMXImageDecoder()
{
  m_inputBufferCount = 0;
  m_outputBufferCount = 0;
   m_imageDecoding = OMX_FALSE;
}

/*==============================================================================
* Function : initializeOutputPort
* Parameters: OMX_PARAM_PORTDEFINITIONTYPE
* Return Value : OMX_ERRORTYPE
* Description: Initialize the Output port with default values
==============================================================================*/
void OMXImageDecoder::initializeOutputPort(
  OMX_PARAM_PORTDEFINITIONTYPE *a_outPort)
{
  a_outPort->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  a_outPort->nVersion.nVersion = OMX_SPEC_VERSION;
  a_outPort->eDir = OMX_DirOutput;
  a_outPort->nBufferCountActual = 1;
  a_outPort->nBufferCountMin = 1;
  a_outPort->bEnabled = OMX_FALSE;
  a_outPort->bPopulated = OMX_FALSE;
  a_outPort->eDomain = OMX_PortDomainImage;
  a_outPort->bBuffersContiguous = OMX_TRUE;
  a_outPort->nBufferAlignment = 4096;
  a_outPort->format.image.cMIMEType = const_cast<char *>("JPEG");
  a_outPort->format.image.nFrameWidth = 0;
  a_outPort->format.image.nFrameHeight = 0;
  a_outPort->format.image.nStride = 0;
  a_outPort->format.image.nSliceHeight = 0;
  a_outPort->format.image.bFlagErrorConcealment = OMX_FALSE;
  a_outPort->format.image.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  a_outPort->format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
  a_outPort->nBufferSize = m_inPort->nBufferSize;

  return;
}

/*==============================================================================
* Function : initializeInputPort
* Parameters: OMX_PARAM_PORTDEFINITIONTYPE
* Return Value : None
* Description: Initialize the Input port with default values
==============================================================================*/
void OMXImageDecoder::initializeInputPort(
  OMX_PARAM_PORTDEFINITIONTYPE *a_inPort)
{
  a_inPort->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  a_inPort->nVersion.nVersion = OMX_SPEC_VERSION;
  a_inPort->eDir = OMX_DirInput;
  a_inPort->nBufferCountActual = 1;
  a_inPort->nBufferCountMin = 1;
  a_inPort->bEnabled = OMX_TRUE;
  a_inPort->bPopulated = OMX_FALSE;
  a_inPort->eDomain = OMX_PortDomainImage;
  a_inPort->bBuffersContiguous = OMX_TRUE;
  a_inPort->nBufferAlignment = 4096;
  a_inPort->format.image.cMIMEType = NULL;
  a_inPort->format.image.bFlagErrorConcealment = OMX_FALSE;
  a_inPort->format.image.eColorFormat = OMX_COLOR_FormatUnused;
  a_inPort->format.image.eCompressionFormat = OMX_IMAGE_CodingJPEG;

  return;
}


/*==============================================================================
* Function : omx_component_init
* Parameters: OMX_HANDLETYPE hComp
* Return Value : OMX_ERRORTYPE
* Description: Initialize the OMX decoder Component with relevant data
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_init(OMX_IN OMX_HANDLETYPE hComp)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  //Create and Initialize the Image Port Param Structure
  m_imagePortParam = new OMX_PORT_PARAM_TYPE();
  if (!m_imagePortParam) {
    QIDBG_ERROR("%s:%d] Failed to allocate ImagePortParam", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_imagePortParam, 0, sizeof(OMX_PORT_PARAM_TYPE));
  m_imagePortParam->nPorts = NUM_OF_PORTS;
  m_imagePortParam->nStartPortNumber = OMX_INPUT_PORT_INDEX;

  //Initilalize the Input and Output Format types.
  m_inputFormatTypes = new OMX_IMAGE_PARAM_PORTFORMATTYPE();
  if (!m_inputFormatTypes) {
    QIDBG_ERROR("%s:%d] Failed to allocate inputFormatTypes", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_inputFormatTypes, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
  m_inputFormatTypes->nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
  m_inputFormatTypes->nVersion.nVersion = OMX_SPEC_VERSION;
  m_inputFormatTypes->nIndex = 0;
  m_inputFormatTypes->eColorFormat = OMX_COLOR_FormatUnused;
  m_inputFormatTypes->eCompressionFormat = OMX_IMAGE_CodingJPEG;

  m_outputFormatTypes = new OMX_IMAGE_PARAM_PORTFORMATTYPE();
  if (!m_outputFormatTypes) {
    QIDBG_ERROR("%s:%d] Failed to allocate outputFormatTypes", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_outputFormatTypes, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
  m_outputFormatTypes->nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
  m_outputFormatTypes->nVersion.nVersion = OMX_SPEC_VERSION;
  m_outputFormatTypes->nIndex = 0;
  m_outputFormatTypes->eColorFormat = OMX_COLOR_FormatUnused;
  m_outputFormatTypes->eCompressionFormat = OMX_IMAGE_CodingUnused;

  //Initilalize the Input Port
  m_inPort = new OMX_PARAM_PORTDEFINITIONTYPE();
  if (!m_inPort) {
    QIDBG_ERROR("%s:%d] Failed to allocate inPort", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_inPort, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  m_inPort->nPortIndex = OMX_INPUT_PORT_INDEX;
  initializeInputPort(m_inPort);

  //Initilaize the Output Port
  m_outPort = new OMX_PARAM_PORTDEFINITIONTYPE();
  if (!m_outPort) {
    QIDBG_ERROR("%s:%d] Failed to allocate outPort", __func__,
      __LINE__);
    return OMX_ErrorInsufficientResources;
  }
  memset(m_outPort, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  m_outPort->nPortIndex = OMX_OUTPUT_PORT_INDEX;
  initializeOutputPort(m_outPort);

  //No data has been allocated yet
  m_dataAllocated = OMX_FALSE;

  //Spawn a new message thread
  m_messageThread = QIThread();
  m_messageThread.StartThread(this, TRUE);

  //Change state to loaded
  m_state = OMX_StateLoaded;
  m_compTransState = OMX_StateNone;
  m_inportTransState = OMX_PORT_NONE;
  m_outportTransState = OMX_PORT_NONE;

  m_compInitialized = OMX_TRUE;

  return lret;

}

/*==============================================================================
* Function : omx_component_get_parameter
* Parameters: hComp, paramIndex, paramData
* Return Value : OMX_ERRORTYPE
* Description: Get the specified parameter
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_get_parameter(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE paramIndex,
  OMX_INOUT OMX_PTR paramData)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lindex = (int)paramIndex;

  if((hComp == NULL) || (paramData == NULL)) {
    QIDBG_ERROR("%s : Bad parameter", __func__);
    return OMX_ErrorBadParameter;
  }
  if(m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s : State Invalid", __func__);
    return OMX_ErrorInvalidState;
  }
  pthread_mutex_lock(&m_compLock);
  switch(lindex) {
  case OMX_IndexParamPortDefinition: {
    OMX_PARAM_PORTDEFINITIONTYPE *ldestPort =
      reinterpret_cast<OMX_PARAM_PORTDEFINITIONTYPE *>(paramData);
    if (ldestPort->nPortIndex == (OMX_U32)OMX_INPUT_PORT_INDEX) {
      memcpy(ldestPort, m_inPort, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    } else if (ldestPort->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX)) {
      memcpy(ldestPort, m_outPort, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    } else {
      QIDBG_ERROR("%s: Invalid port Index", __func__);
      lret = OMX_ErrorNoMore;
    }
    break;
  }
  case OMX_IndexParamImageInit: {
    OMX_PORT_PARAM_TYPE *l_destPortparam =
     reinterpret_cast <OMX_PORT_PARAM_TYPE *>(paramData);
    memcpy(l_destPortparam, m_imagePortParam, sizeof(OMX_PORT_PARAM_TYPE));
    break;
  }
  case OMX_IndexParamImagePortFormat: {
    OMX_IMAGE_PARAM_PORTFORMATTYPE *l_destImagePortFormat =
     reinterpret_cast <OMX_IMAGE_PARAM_PORTFORMATTYPE *>(paramData);
    if ((l_destImagePortFormat->nPortIndex == (OMX_U32)OMX_INPUT_PORT_INDEX)) {
      memcpy(l_destImagePortFormat, m_inputFormatTypes,
        sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    } else if (l_destImagePortFormat->nPortIndex ==
      (OMX_U32)OMX_OUTPUT_PORT_INDEX) {
      memcpy(l_destImagePortFormat, m_outputFormatTypes,
        sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    } else {
      QIDBG_ERROR("Invalid Port Index");
      lret = OMX_ErrorNoMore;
    }
    break;
  }

  default: {
    QIDBG_ERROR("%s:%d] Unknown Parameter %d", __func__, __LINE__, paramIndex);
    lret = OMX_ErrorBadParameter;
    break;
  }
  }
  pthread_mutex_unlock(&m_compLock);
  return lret;
}

/*==============================================================================
* Function : portStateIsOk
* Parameters: port
* Return Value : OMX_BOOL
* Description: Returns OMX_TRUE if set parameter on port is allowed,
* OMX_FALSE otherwise.
==============================================================================*/
OMX_BOOL OMXImageDecoder::portStateIsOk(OMX_PARAM_PORTDEFINITIONTYPE *port)
{
  if ((port == NULL) || port->bEnabled) {
    if ((m_state != OMX_StateLoaded) && (m_state != OMX_StateWaitForResources)) {
      QIDBG_ERROR("%s : Component not in the right state. current state = %d",
          __func__, m_state);
      return OMX_FALSE;
    }
  }
  return OMX_TRUE;
}

/*==============================================================================
* Function : omx_component_set_parameter
* Parameters: hComp, paramIndex, paramData
* Return Value : OMX_ERRORTYPE
* Description: Set the specified parameter with the value
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_set_parameter(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE paramIndex,
  OMX_IN OMX_PTR paramData)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lindex = (int)paramIndex;

  QIDBG_MED("%s %d:] ParamIndex: %d", __func__, __LINE__,
   (int)paramIndex);
  if ((hComp == NULL) || (paramData == NULL)) {
    QIDBG_ERROR("%s : Bad parameter", __func__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s : Component in Invalid State. current state = %d",
      __func__, m_state);
    return OMX_ErrorInvalidState;
  }

  pthread_mutex_lock(&m_compLock);

  switch(lindex) {
  case OMX_IndexParamPortDefinition: {
    OMX_PARAM_PORTDEFINITIONTYPE *ldestPort =
      reinterpret_cast <OMX_PARAM_PORTDEFINITIONTYPE *> (paramData);
    if (ldestPort->nPortIndex == (OMX_U32)OMX_INPUT_PORT_INDEX) {
      if (!portStateIsOk(m_inPort)) {
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorIncorrectStateOperation;
      }
      if ((ldestPort->format.image.nFrameWidth > MAX_IMAGE_WIDTH) ||
        (ldestPort->format.image.nFrameHeight > MAX_IMAGE_HEIGHT)) {
        QIDBG_ERROR("%s: Width/Height exceeds max width = %d height =%d",
          __func__, (int)ldestPort->format.image.nFrameWidth,
          (int)ldestPort->format.image.nFrameHeight);
        lret  = OMX_ErrorUnsupportedSetting;
      }
      memcpy(m_inPort, ldestPort, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
      m_inPort->format.image.nStride = ldestPort->format.image.nFrameWidth;
      QIDBG_MED("%s:%d]: buffer size id %d", __func__, __LINE__,
        (int)m_inPort->nBufferSize);
    } else if (ldestPort->nPortIndex == (OMX_U32)(OMX_OUTPUT_PORT_INDEX)) {
      if (!portStateIsOk(m_outPort)) {
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorIncorrectStateOperation;
      }
      memcpy(m_outPort, ldestPort, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    } else {
      QIDBG_ERROR("%s: Invalid port Index", __func__);
      lret = OMX_ErrorNoMore;
    }
    break;
  }
  case OMX_IndexParamImageInit: {
    OMX_PORT_PARAM_TYPE *l_destPortparam =
      reinterpret_cast <OMX_PORT_PARAM_TYPE *>(paramData);
    if (!portStateIsOk(NULL)) {
      pthread_mutex_unlock(&m_compLock);
      return OMX_ErrorIncorrectStateOperation;
    }
    memcpy(m_imagePortParam, l_destPortparam, sizeof(OMX_PORT_PARAM_TYPE));
    break;
  }
  case OMX_IndexParamImagePortFormat: {
    OMX_IMAGE_PARAM_PORTFORMATTYPE *l_destImagePortFormat =
     reinterpret_cast <OMX_IMAGE_PARAM_PORTFORMATTYPE *>(paramData);
    if ((l_destImagePortFormat->nPortIndex == (OMX_U32)OMX_INPUT_PORT_INDEX)) {
      if (!portStateIsOk(m_inPort)) {
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorIncorrectStateOperation;
      }
      memcpy(m_inputFormatTypes, l_destImagePortFormat,
        sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    } else if (l_destImagePortFormat->nPortIndex ==
      (OMX_U32)OMX_OUTPUT_PORT_INDEX) {
      if (!portStateIsOk(m_outPort)) {
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorIncorrectStateOperation;
      }
      memcpy(m_outputFormatTypes, l_destImagePortFormat,
        sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    } else {
      QIDBG_ERROR("Invalid Port Index");
      lret = OMX_ErrorNoMore;
    }
  break;
  }
  default: {
    QIDBG_ERROR("%s: Unknown Parameter %d", __func__, paramIndex);
    lret = OMX_ErrorBadParameter;
    break;
  }
  }
  pthread_mutex_unlock(&m_compLock);
  return lret;
}

/*==============================================================================
* Function : use_input_buffer
* Parameters: bufferHdr, bytes, buffer
* Return Value : OMX_ERRORTYPE
* Description: Set the Input buffers passed by the OMX client
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::use_input_buffer(
  OMX_INOUT OMX_BUFFERHEADERTYPE** abufferHdr,
  OMX_U32 abytes,
  OMX_U8* abuffer,
  OMX_IN OMX_PTR appData,
  OMX_U32 aport)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int i = 0;
  QIDBG_MED("%s: Port Index = %d", __func__, (int) aport);

  if (m_outBuffAllocCount == 0) {
    mInOMXBufferData = new QOMX_Buffer_Data_t[m_inPort->nBufferCountActual];
    if (!mInOMXBufferData) {
      QIDBG_ERROR("%s %d: Error: Allocation failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
    memset(mInOMXBufferData, 0, sizeof(QOMX_Buffer_Data_t));
  }

  i = m_inBuffAllocCount;
  mInOMXBufferData[i].mHeader.nAllocLen = abytes;
  mInOMXBufferData[i].mHeader.nInputPortIndex = (OMX_U32)aport;
  mInOMXBufferData[i].mHeader.nSize = sizeof(OMX_BUFFERHEADERTYPE);
  mInOMXBufferData[i].mHeader.nVersion.nVersion = OMX_SPEC_VERSION;
  mInOMXBufferData[i].mHeader.pBuffer = abuffer;
  mInOMXBufferData[i].mHeader.pAppPrivate = appData;
  mInOMXBufferData[i].mHeader.nOffset = 0;
  mInOMXBufferData[i].mHeader.pPlatformPrivate = &mInOMXBufferData->mInfo;
  mInOMXBufferData[i].mHeader.pInputPortPrivate = m_inPort;

  mInOMXBufferData[i].mInfo.offset = 0;
  mInOMXBufferData[i].mInfo.fd = -1;

  if (appData) {
    QOMX_BUFFER_INFO *lbufferInfo = (QOMX_BUFFER_INFO *)appData;
    mInOMXBufferData[i].mInfo.fd =  lbufferInfo->fd;
    mInOMXBufferData[i].mInfo.offset = lbufferInfo->offset;
  }
  mInOMXBufferData[i].mHeader.pPlatformPrivate = &mInOMXBufferData->mInfo;
  mInOMXBufferData[i].mHeader.pOutputPortPrivate = &mInOMXBufferData->mInfo;

  *abufferHdr = &mInOMXBufferData[i].mHeader;
  mInOMXBufferData[i].valid = OMX_TRUE;
   m_inBuffAllocCount++;

  if (m_inPort->nBufferCountActual == m_inBuffAllocCount) {
    m_inPort->bPopulated = OMX_TRUE;
  }
  QIDBG_MED("%s:%d] BufferCountActual = %d, inBuffAllocCount = %d", __func__,
    __LINE__, (int)m_inPort->nBufferCountActual, (int)m_inBuffAllocCount);
  return lret;
}

/*==============================================================================
* Function : use_output_buffer
* Parameters: bufferHdr, bytes, buffer
* Return Value : OMX_ERRORTYPE
* Description: Set the Input buffers passed by the OMX client
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::use_output_buffer(
  OMX_INOUT OMX_BUFFERHEADERTYPE** abufferHdr,
  OMX_U32 abytes,
  OMX_U8* abuffer,
  OMX_IN OMX_PTR appData,
  OMX_U32 aport)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int i = 0;
  QIDBG_MED("%s: Port Index = %d", __func__, (int) aport);

  if (m_outBuffAllocCount == 0) {
    mOutOMXBufferData = new QOMX_Buffer_Data_t[m_outPort->nBufferCountActual];
    if (NULL == mOutOMXBufferData) {
      QIDBG_ERROR("%s:%d] Error: Allocation failed", __func__, __LINE__);
      return OMX_ErrorInsufficientResources;
    }
    memset(mOutOMXBufferData, 0, sizeof(QOMX_Buffer_Data_t));
  }
  i = m_outBuffAllocCount;
  mOutOMXBufferData[i].mHeader.nAllocLen = abytes;
  mOutOMXBufferData[i].mHeader.nInputPortIndex = (OMX_U32)aport;
  mOutOMXBufferData[i].mHeader.pOutputPortPrivate = m_outPort;
  mOutOMXBufferData[i].mHeader.nSize = sizeof(OMX_BUFFERHEADERTYPE);
  mOutOMXBufferData[i].mHeader.nVersion.nVersion = OMX_SPEC_VERSION;
  mOutOMXBufferData[i].mHeader.pBuffer = abuffer;
  mOutOMXBufferData[i].mHeader.nOffset = 0;
  mOutOMXBufferData[i].mHeader.nFilledLen = 0;

  mOutOMXBufferData[i].mInfo.offset = 0;
  mOutOMXBufferData[i].mInfo.fd = -1;

  if (appData) {
    QOMX_BUFFER_INFO *lbufferInfo = (QOMX_BUFFER_INFO *)appData;
    mOutOMXBufferData[i].mInfo.fd =  lbufferInfo->fd;
    mOutOMXBufferData[i].mInfo.offset = lbufferInfo->offset;
  }
  mOutOMXBufferData[i].mHeader.pAppPrivate =
    &(mOutOMXBufferData[i].mInfo);
  mOutOMXBufferData[i].mHeader.pPlatformPrivate = &mOutOMXBufferData->mInfo;
  mOutOMXBufferData[i].mHeader.pOutputPortPrivate = &mOutOMXBufferData->mInfo;
  mOutOMXBufferData[i].valid = OMX_TRUE;
  *abufferHdr = &mOutOMXBufferData[i].mHeader;

  m_outBuffAllocCount++;

  if (m_outPort->nBufferCountActual == m_outBuffAllocCount) {
    m_outPort->bPopulated = OMX_TRUE;
  }
  QIDBG_MED("%s %d: BufferCountActual = %d, outBuffAllocCount = %d", __func__,
    __LINE__, (int)m_outPort->nBufferCountActual, (int)m_outBuffAllocCount);
  return lret;

}

/*==============================================================================
* Function : omx_component_use_buffer
* Parameters: hComp, bufferHdr - Buffer header needs to allocated by the
* component, port - Port associated with the buffer, appData, bytes -
* size of the buffer, buffer - address of the buffer
* Return Value : OMX_ERRORTYPE
* Description: Create the bufferheader and set the buffers and any relevant data
* and return the header to the OMX client
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_use_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 bytes,
  OMX_IN OMX_U8* buffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  OMX_PARAM_PORTDEFINITIONTYPE *lPort;
  qomx_intermediate_port_state_t lPortTransState;

  if (bufferHdr == NULL || buffer == NULL || bytes == 0 ) {
    QIDBG_ERROR("%s %d]: bad param 0x%p %ld 0x%p", __func__, __LINE__,
      bufferHdr, bytes, buffer);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s %d] : Invalid State", __func__, __LINE__);
    return OMX_ErrorInvalidState;
  }

  if ((port == (OMX_U32)OMX_INPUT_PORT_INDEX)) {
    lPort = m_inPort;
    lPortTransState = m_inportTransState;
  } else if ((port == (OMX_U32)OMX_OUTPUT_PORT_INDEX)) {
    lPort = m_outPort;
    lPortTransState = m_outportTransState;
  } else {
    QIDBG_ERROR("%s : Error Invalid port!", __func__);
    return OMX_ErrorBadPortIndex;
  }

  if (!lPort->bEnabled) {
    QIDBG_ERROR("%s : Error  port index disabled", __func__);
    lret = OMX_ErrorNotReady;
  }

  if ((lPort->bEnabled) && (lPortTransState != OMX_PORT_ENABLE_PENDING)) {
    if (((m_state == OMX_StateLoaded) &&
        (m_compTransState != OMX_StateIdle_Pending)) &&
        (m_state != OMX_StateIdle) &&
        (m_state != OMX_StateWaitForResources)) {
        QIDBG_ERROR("%s %d]: Not allowed in current state %d", __func__,
          __LINE__, m_state);
        return OMX_ErrorIncorrectStateOperation;
      }
  }

  pthread_mutex_lock(&m_compLock);
  if ((port == (OMX_U32)OMX_INPUT_PORT_INDEX)) {
    if (m_inPort->bEnabled) {
      if (bytes != m_inPort->nBufferSize) {
        QIDBG_ERROR("%s:%d] exceeds the buffer size requested previously = "
          "%d", __func__, (int)bytes, (int)m_inPort->nBufferSize);
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorBadParameter;
      }
      if (m_inBuffAllocCount == m_inPort->nBufferCountActual) {
        QIDBG_ERROR("%s:%d] Error: exceeds actual number of buffers "
          " requested %d %d",
          __func__, __LINE__, (int)m_inBuffAllocCount,
          (int)m_inPort->nBufferCountActual);
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorInsufficientResources;
      }
      lret = use_input_buffer(bufferHdr, bytes, buffer, appData, port);
    } else {
      QIDBG_ERROR("%s:%d] Error I/p port disabled", __func__, __LINE__);
      lret = OMX_ErrorNotReady;
    }
  }

  if (port == (OMX_U32) OMX_OUTPUT_PORT_INDEX) {
    if (m_outPort->bEnabled) {
      if (bytes != m_outPort->nBufferSize) {
        QIDBG_ERROR("%s %d: %d In o/p port exceeds the buffer size "
          "requested previously = %d",__func__, __LINE__, (int)bytes,
          (int)m_outPort->nBufferSize);
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorBadParameter;
      }
      if (m_outBuffAllocCount == m_inPort->nBufferCountActual) {
        QIDBG_ERROR("%s: Error:exceeds actual number of buffers requested",
          __func__);
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorInsufficientResources;
      }
      lret = use_output_buffer(bufferHdr, bytes, buffer, appData, port);
    } else {
      QIDBG_ERROR("%s : Error O/p port disabled", __func__);
      lret = OMX_ErrorNotReady;
    }
  }
  QIDBG_MED("%s %d: Inport populated = %d, Outport populated = %d", __func__,
    __LINE__, (int)m_inPort->bPopulated, (int)m_outPort->bPopulated);
  //Check if all the required ports are populated and set the flag
  if (m_inPort->bPopulated &&
      (m_outPort->bPopulated || !m_outPort->bEnabled)) {
    m_dataAllocated = OMX_TRUE;
  }
  QIDBG_MED("%s:%d] All Data Allocated = %d", __func__, __LINE__,
    m_dataAllocated);
  //If all the ports are populated and the component is in the idle pending
  //State transition to the Idle state
  if (m_dataAllocated && m_compTransState == OMX_StateIdle_Pending) {
    QIMessage *lMessage = new QIMessage();
    if (lMessage == NULL) {
      QIDBG_ERROR("%s:%d] cannot create message", __func__, __LINE__);
      lret = OMX_ErrorInsufficientResources;
    } else {
      m_state = OMX_StateIdle;
      m_compTransState = OMX_StateNone;
      lMessage->iData = m_state;
      lMessage->m_qMessage = OMX_MESSAGE_CHANGE_STATE_DONE;
      lret = postMessage(lMessage);
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] cannot post message", __func__, __LINE__);
        delete lMessage;
      }
    }
  }
  if (lPortTransState == OMX_PORT_ENABLE_PENDING) {
    QIMessage *lMessage = new QIMessage();
    if (lMessage == NULL) {
      QIDBG_ERROR("%s:%d] cannot create message", __func__, __LINE__);
      lret = OMX_ErrorInsufficientResources;
    } else {
      lPortTransState = OMX_PORT_NONE;
      lMessage->iData = port;
      lMessage->m_qMessage = OMX_MESSAGE_PORT_ENABLE_DONE;
      lret = postMessage(lMessage);
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] cannot post message", __func__, __LINE__);
        delete lMessage;
      }
    }
  }

  pthread_mutex_unlock(&m_compLock);
  return lret;
 }

/*==============================================================================
* Function : omx_component_get_config
* Parameters: hComp, configIndex, configData
* Return Value : OMX_ERRORTYPE
* Description: Get the requested configuration
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_get_config(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE configIndex,
  OMX_INOUT OMX_PTR configData)
{
  //TODO: Do.
  return OMX_ErrorBadParameter;
}

/*==============================================================================
* Function : omx_component_set_config
* Parameters: hComp, configIndex, configData
* Return Value : OMX_ERRORTYPE
* Description: Get the requested configuration
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_set_config(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_INDEXTYPE configIndex,
  OMX_INOUT OMX_PTR configData)
{
  //TODO: Ditto.
  return OMX_ErrorBadParameter;
}

/*==============================================================================
* Function : Execute
* Parameters: void
* Return Value : void
* Description: This method is inherited from the QIThreadObject class and
* is called from the StartThread method of the thread class as soon as a *
* startthread is called by the object on a new thread.
==============================================================================*/
void OMXImageDecoder::Execute()
{
  if (m_messageThread.IsSelf()) {
    handleMessage();
  }
}

/*==============================================================================
* Function : port_disable
* Parameters: OMX_U32 portIndex
* Return Value : OMX_ERRORTYPE
* Description: Disable the specified port and return any buffers
* that the port is holding throught ETB Done or FTB Done. Wait for
* OMX_Free_buffer to be complete to finish disabling the port.
==============================================================================*/
void OMXImageDecoder::portDisable(OMX_U32 a_portIndex)
{
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s : Error Invalid State",__func__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      OMX_ErrorIncorrectStateOperation, a_portIndex, NULL);
    sem_post(&m_cmdLock);
    return;
  }
  if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inPort->nPortIndex)) {
  //If comp is in state loaded and not in transition to idle,enable the port
    if ((m_state == OMX_StateLoaded) &&
      (!(m_compTransState == OMX_StateIdle_Pending))) {
      if (m_inBuffAllocCount > 0) {
        //Flush out through empty this buffer done
        m_inportTransState = OMX_PORT_DISABLE_PENDING;
      }
    } else if((m_state == OMX_StateWaitForResources) ||
      (m_state == OMX_StateLoaded)) {
      m_inPort->bEnabled = OMX_FALSE;
      m_callbacks->EventHandler(m_compHandle, m_appData,
      OMX_EventCmdComplete, OMX_CommandPortDisable, m_inPort->nPortIndex, NULL);
    } else {
        //Flush out input buffers
        m_inportTransState = OMX_PORT_DISABLE_PENDING;
    }
  }
  if ((a_portIndex == OMX_ALL) || (a_portIndex == m_outPort->nPortIndex)) {
    if (!m_outPort->bEnabled) {
      QIDBG("%s : Before EventHandler",__func__);
      m_callbacks->EventHandler(m_compHandle, m_appData,
                OMX_EventCmdComplete, OMX_CommandPortDisable, m_outPort->nPortIndex, NULL);
      QIDBG("%s : After EventHandler",__func__);
    }
    //If comp is in state loaded and not in transition to idle,enable the port
    if ((m_state == OMX_StateLoaded) &&
        (!(m_compTransState == OMX_StateIdle_Pending))) {
      if (m_inBuffAllocCount > 0) {
        //Flush out through empty this buffer done
        m_outportTransState = OMX_PORT_DISABLE_PENDING;
      }
    } else if((m_state == OMX_StateWaitForResources) ||
        (m_state == OMX_StateLoaded)) {
      m_outPort->bEnabled = OMX_FALSE;
      m_callbacks->EventHandler(m_compHandle, m_appData,
          OMX_EventCmdComplete, OMX_CommandPortDisable, m_outPort->nPortIndex, NULL);
    } else {
      //Flush out input buffers
      m_outportTransState = OMX_PORT_DISABLE_PENDING;
    }
   }

  sem_post(&m_cmdLock);
}

/*==============================================================================
* Function : port_enable
* Parameters: OMX_U32 portIndex
* Return Value : OMX_ERRORTYPE
* Description: This method is used to enable the port
==============================================================================*/
void OMXImageDecoder::portEnable(OMX_U32 a_portIndex)
{
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s : Error Invalid State",__func__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      OMX_ErrorIncorrectStateOperation, a_portIndex, NULL);
    sem_post(&m_cmdLock);
    return;
  }
  if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inPort->nPortIndex)) {
    m_inPort->bEnabled = OMX_TRUE;
    //If comp is in state loaded and not in transition to idle,enable the port
    if (((m_state == OMX_StateLoaded) &&
      (!(m_compTransState == OMX_StateIdle_Pending))) ||
      (m_state == OMX_StateWaitForResources)) {
        m_callbacks->EventHandler(m_compHandle, m_appData,
        OMX_EventCmdComplete, OMX_CommandPortEnable, m_inPort->nPortIndex,
        NULL);
    } else {
        m_inportTransState = OMX_PORT_ENABLE_PENDING;
    }
  } else if ((a_portIndex == OMX_ALL) ||
    (a_portIndex == m_outPort->nPortIndex)) {
      m_outPort->bEnabled = OMX_TRUE;
      if (((m_state == OMX_StateLoaded) &&
        (!(m_compTransState == OMX_StateIdle_Pending))) ||
        (m_state == OMX_StateWaitForResources)) {
           m_callbacks->EventHandler(m_compHandle, m_appData,
           OMX_EventCmdComplete,OMX_CommandPortEnable, m_outPort->nPortIndex,
           NULL);
      } else {
        m_outportTransState = OMX_PORT_ENABLE_PENDING;
      }
  } else {
    QIDBG("%s: ERROR Invalid Port Index", __func__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      OMX_CommandPortEnable, a_portIndex, NULL);
  }

  sem_post(&m_cmdLock);
}

/*==============================================================================
* Function : process_Message
* Parameters: Message of type QIBase
* Return Value : OMX_ERRORTYPE
* Description: This method processes the commands/events from the message queue
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::processMessage(QIMessage *a_Message)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  QIDBG_MED("%s:%d] E", __func__, __LINE__);

  if(!a_Message) {
    QIDBG_ERROR("%s:%d] bad parameter", __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }

  if(m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s:%d] Invalid state", __func__, __LINE__);
    return OMX_ErrorInvalidState;
  }

  switch(a_Message->m_qMessage) {
  case OMX_MESSAGE_CHANGE_STATE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_CHANGE_STATE", __func__, __LINE__);
    changeState((OMX_STATETYPE)a_Message->iData);
    break;
  }

  case OMX_MESSAGE_PORT_ENABLE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_PORT_ENABLE", __func__, __LINE__);
    portEnable((OMX_U32)a_Message->iData);
    break;
  }

  case OMX_MESSAGE_PORT_DISABLE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_PORT_DISABLE", __func__, __LINE__);
    portDisable((OMX_U32)a_Message->iData);
    break;
  }

  case OMX_MESSAGE_FLUSH: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_FLUSH", __func__, __LINE__);
    handleCommandFlush((OMX_U32)a_Message->iData);
    break;
  }

  case OMX_MESSAGE_START_MAIN_DECODE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_START_MAIN_DECODE", __func__, __LINE__);
    if (!m_abort_flag) {
      lret = startDecode();
    }
    pthread_mutex_unlock(&m_abortlock);
    if (lret != OMX_ErrorNone) {
      //Todo: change state to Invalid
    }
    break;
  }

  case OMX_MESSAGE_ETB_DONE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_ETB_DONE", __func__, __LINE__);
    emptyBufferDone((OMX_BUFFERHEADERTYPE *)a_Message->pData);
    break;
  }

  case OMX_MESSAGE_FTB_DONE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_FTB_DONE", __func__, __LINE__);
    fillBufferDone((OMX_BUFFERHEADERTYPE *)a_Message->pData);
    if (m_ftbQueue.Count() == 0) {
      //Finished flushing all the buffers.
      m_executionComplete = OMX_TRUE;
    }
    break;
  }

  case OMX_MESSAGE_EVENT_ERROR: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_EVENT_ERROR", __func__, __LINE__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      a_Message->iData, 0, NULL);
    break;
  }

  case OMX_MESSAGE_CHANGE_STATE_DONE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_CHANGE_STATE_DONE", __func__, __LINE__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventCmdComplete,
      OMX_CommandStateSet, a_Message->iData, NULL);
    break;
  }

  case OMX_MESSAGE_PORT_SETTINGS_CHANGED: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_PORT_SETTINGS_CHANGED", __func__, __LINE__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventPortSettingsChanged,
        a_Message->iData, OMX_IndexParamPortDefinition, NULL);
    break;
  }


  case OMX_MESSAGE_PORT_ENABLE_DONE: {
    QIDBG_MED("%s:%d] OMX_MESSAGE_PORT_ENABLE", __func__, __LINE__);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventCmdComplete,
          OMX_CommandPortEnable, a_Message->iData, NULL);
    break;
  }

  default: {
    QIDBG_ERROR("%s:%d] Invalid Message = %d", __func__, __LINE__,
      a_Message->m_qMessage);
    break;
  }
  } /* end of switch*/

  if (a_Message) {
    delete a_Message;
    a_Message = NULL;
  }
  return lret;
}

/*==============================================================================
* Function : flushBufferQueues
* Parameters: OMX_U32 a_portIndex - Port Index of the port who's buffers
* need to be flushed
* Return Value : OMX_ERRORTYPE
* Description: Return the unprocessed buffers associated with the port. This can
* be called when the client calls command flush, during transition to
* OMX_StateIdle from Executing/Pause or transition to OMX_StateInvalid or
* when the port is disabled.
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::flushBufferQueues(OMX_U32 a_portIndex)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_Buffer *l_buffer;

  if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inPort->nPortIndex)) {
    for (int i = 0; i < m_etbQueue.Count(); i++) {
      l_buffer = (QOMX_Buffer *)m_etbQueue.Dequeue();
      if(l_buffer) {
        emptyBufferDone((OMX_BUFFERHEADERTYPE *)l_buffer->getBuffer());
        delete l_buffer;
      }
    }
  } else if ((a_portIndex == OMX_ALL) ||
    (a_portIndex == m_outPort->nPortIndex)) {
    for (int i = 0; i < m_ftbQueue.Count(); i++) {
      l_buffer = (QOMX_Buffer *)m_ftbQueue.Dequeue();
      if(l_buffer) {
        fillBufferDone((OMX_BUFFERHEADERTYPE *)l_buffer->getBuffer());
        delete l_buffer;
      }
    }
  } else {
    QIDBG_ERROR("%s %d]: Bad Port Index", __func__, __LINE__);
    lret = OMX_ErrorBadPortIndex;
  }
  return lret;
}

/*==============================================================================
* Function : handleCommandFlush
* Parameters: OMX_U32 a_portIndex - port Index of the port who's buffers
* need to be flushed
* Return Value : void
* Description: Handle the Flush command from the omx client. Calls
* flush_Buffer_Queues to flush out unprocessed buffers associated with
* a_portIndex
==============================================================================*/
void OMXImageDecoder::handleCommandFlush(OMX_U32 a_portIndex)
{
   OMX_ERRORTYPE lret = OMX_ErrorNone;
   lret = flushBufferQueues(a_portIndex);
   if (lret == OMX_ErrorNone ) {
     if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inPort->nPortIndex)) {
       m_callbacks->EventHandler(m_compHandle, m_appData,
         OMX_EventCmdComplete, OMX_CommandFlush, m_inPort->nPortIndex, NULL);
     } else if ((a_portIndex == OMX_ALL) ||
       (a_portIndex == m_outPort->nPortIndex)) {
       m_callbacks->EventHandler(m_compHandle, m_appData,
         OMX_EventCmdComplete, OMX_CommandFlush, m_outPort->nPortIndex,
         NULL);
     }
   } else {
     m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError, lret,
       a_portIndex, NULL);
   }
  sem_post(&m_cmdLock);
}

/*==============================================================================
* Function : omx_component_empty_this_buffer
* Parameters: OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE buffer
* Return Value : OMX_ERRORTYPE
* Description: The client makes this call with the input buffer indicating the
* component to start processing the input buffer
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_empty_this_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  QOMX_Buffer *lInputBuffer;

  if (hComp == NULL || buffer == NULL ||
    (buffer->nSize != sizeof(OMX_BUFFERHEADERTYPE))) {
    QIDBG_ERROR("%s: Bad Parameter", __func__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Invalid State", __func__);
    return OMX_ErrorInvalidState;
  }
  if ((m_state != OMX_StateExecuting) && (m_state != OMX_StatePause)) {
    QIDBG_ERROR("%s: Current State is %d. Operation Not allowed",
      __func__, m_state);
    return OMX_ErrorInvalidState;
  }
  if ((buffer->nVersion.nVersion != OMX_SPEC_VERSION)) {
    QIDBG_ERROR("%s: Error - Invalid version ", __func__);
    return OMX_ErrorVersionMismatch;
  }
  if (!(buffer->nInputPortIndex == m_inPort->nPortIndex)) {
      QIDBG_ERROR("%s : bad Port index %d",
        __func__,(int)buffer->nInputPortIndex);
      return OMX_ErrorBadPortIndex;
  }
  if((buffer->nInputPortIndex == m_inPort->nPortIndex) &&
    (!m_inPort->bEnabled)) {
    QIDBG_ERROR("%s: Error Port not enabled %d",
      __func__, (int)buffer->nInputPortIndex);
    return OMX_ErrorIncorrectStateOperation;
  }
  pthread_mutex_lock(&m_etbQLock);

  lInputBuffer = new QOMX_Buffer(buffer);
  if (NULL == lInputBuffer) {
    QIDBG_ERROR("%s:%d] cannot allocate OMX buffer",
      __func__, __LINE__);
    pthread_mutex_unlock(&m_etbQLock);
    return OMX_ErrorInsufficientResources;
  }
  if (QI_ERROR(m_etbQueue.Enqueue(lInputBuffer))) {
    QIDBG_ERROR("%s:%d] cannot enqueue OMX buffer",
      __func__, __LINE__);
    delete lInputBuffer;
    lInputBuffer = NULL;
    pthread_mutex_unlock(&m_etbQLock);
    return OMX_ErrorInsufficientResources;
  }

  QIDBG_MED("%s %d: ETB Queue count = %d, Expected number of buffers = %d ",
    __func__, __LINE__, m_etbQueue.Count(), (int)m_inPort->nBufferCountActual);

  m_etbFlag = OMX_TRUE;
  pthread_mutex_unlock(&m_etbQLock);

  //Start decode if in execute state already and etb and ftb flags are set
  QIDBG_MED("%s:%d] state %d etb_flag %d ftb_flag %d",
    __func__, __LINE__, m_state, m_etbFlag, m_ftbFlag);

  if (m_state == OMX_StateExecuting) {
    // Start processing the image
    // Extract the image dims
    ProcessInputBuffer();

    // After genereting PortSettingsChanged, component should stop processing
    // For that reason execution is triggered on FTB.

    //TODO: Redesign the processing of buffers
  }

  return lrc;
}

/*==============================================================================
* Function : omx_component_fill_this_buffer
* Parameters: OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE buffer
* Return Value : OMX_ERRORTYPE
* Description: The client makes this call with the out buffer indicating the
* component to start processing the output buffer
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_fill_this_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  QOMX_Buffer *lOutputBuffer;

  if (hComp == NULL || buffer == NULL ||
    (buffer->nSize != sizeof(OMX_BUFFERHEADERTYPE))) {
    QIDBG_ERROR("%s: Bad Parameter", __func__);
    return OMX_ErrorBadParameter;
  }

  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Invalid State", __func__);
    return OMX_ErrorInvalidState;
  }

  if ((m_state != OMX_StateExecuting) && (m_state != OMX_StatePause)) {
    QIDBG_ERROR("%s: Current State is %d. Operation Not allowed",
      __func__, m_state);
    return OMX_ErrorInvalidState;
  }

  if ((buffer->nVersion.nVersion != OMX_SPEC_VERSION)) {
    QIDBG_ERROR("%s: Error - Invalid version ", __func__);
    return OMX_ErrorVersionMismatch;
  }

  if((buffer->nOutputPortIndex == m_outPort->nPortIndex) &&
    (!m_outPort->bEnabled)) {
    QIDBG_ERROR("%s: Error Port not enabled %d",
      __func__, (int)buffer->nOutputPortIndex);
    return OMX_ErrorIncorrectStateOperation;
  }
  pthread_mutex_lock(&m_ftbQLock);

  lOutputBuffer = new QOMX_Buffer(buffer);
  if (NULL == lOutputBuffer) {
    QIDBG_ERROR("%s:%d] cannot allocate OMX buffer",
      __func__, __LINE__);
    pthread_mutex_unlock(&m_ftbQLock);
    return OMX_ErrorInsufficientResources;
  }

  if (QI_ERROR(m_ftbQueue.Enqueue(lOutputBuffer))) {
    QIDBG_ERROR("%s:%d] cannot enqueue buffer",
      __func__, __LINE__);
    pthread_mutex_unlock(&m_ftbQLock);
    return OMX_ErrorInsufficientResources;
  }

  QIDBG_MED("%s %d: FTB Queue count = %d, Expected number of buffers = %d ",
    __func__, __LINE__, m_ftbQueue.Count(),
    (int)m_outPort->nBufferCountActual);

  m_ftbFlag = OMX_TRUE;
  pthread_mutex_unlock(&m_ftbQLock);
  //Start decode if in execute state already and etb and ftb flags are set

  QIDBG_MED("%s:%d] state %d etb_flag %d ftb_flag %d",
    __func__, __LINE__, m_state, m_etbFlag, m_ftbFlag);
  if ((m_state == OMX_StateExecuting) && m_etbFlag && m_ftbFlag) {
    QIDBG_MED("%s %d: Going to Start decoding. Etb and ftb flags are set",
      __func__, __LINE__);
    lrc = Start();
    /* reset the flags */
    m_etbFlag = OMX_FALSE;
    m_ftbFlag = OMX_FALSE;
  }
  return lrc;
}

/*==============================================================================
* Function : start
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Start decode of the image. This method is called when the
* etb and ftb are done and the component is in the execute state. Dequeue the
* buffers from the etb and ftb queue and call decode.
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::ProcessInputBuffer()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_Buffer *lInputBuffer, *lOutputBuffer;
  OMX_U32 lomxOutFormat;
  QIMessage *lMessage;

  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Inavlid state", __func__);
    return OMX_ErrorInvalidState;
  }
  if (m_state != OMX_StateExecuting) {
    QIDBG_ERROR("%s %d]: Incorrect state %d", __func__, __LINE__,
      (int)m_state);
    return OMX_ErrorIncorrectStateOperation;
  }
  pthread_mutex_lock(&m_etbQLock);
  lInputBuffer = (QOMX_Buffer *)m_etbQueue.Dequeue();
  pthread_mutex_unlock(&m_etbQLock);
  if ((NULL == lInputBuffer) || (NULL == lInputBuffer->getBuffer())) {
    QIDBG_ERROR("%s %d]: Input Buffer is NULL", __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }

  //Assign the current i/p buffer
  m_currentInBuffHdr = lInputBuffer->getBuffer();
  delete lInputBuffer;
  lInputBuffer = NULL;
  //Call Decode Image header
  lret = decodeImageHeader(m_currentInBuffHdr);
  if (lret != OMX_ErrorNone) {
   QIDBG_ERROR("%s %d]: Failure.", __func__, __LINE__);
   return lret;
  }
  lret = getOmxFormat(&lomxOutFormat, m_outImgFormat, m_outImgSubsampling);
  if (lret != OMX_ErrorNone) {
    QIDBG_ERROR("%s %d]: Failure.", __func__, __LINE__);
    return lret;
  }
  // We should now have the buffer dimensions
  // Update output port definition

  pthread_mutex_lock(&m_compLock);
  m_outPort->format.image.nFrameWidth = m_outImgSize.Width();
  m_outPort->format.image.nFrameHeight = m_outImgSize.Height();
  m_outPort->format.image.nStride = CEILING16(m_outPort->format.image.nFrameWidth);
  m_outPort->format.image.nSliceHeight = CEILING16(m_outPort->format.image.nFrameHeight);
  m_outPort->format.image.eColorFormat = (OMX_COLOR_FORMATTYPE)lomxOutFormat;
  m_outPort->nBufferSize = m_outImgLength;
  pthread_mutex_unlock(&m_compLock);

  // Sent event that port definition has changed
  lMessage = new QIMessage();
  if (lMessage == NULL) {
    QIDBG_ERROR("%s:%d] cannot create message", __func__, __LINE__);
    lret = OMX_ErrorInsufficientResources;
  } else {
    lMessage->iData = m_outPort->nPortIndex;
    lMessage->m_qMessage = OMX_MESSAGE_PORT_SETTINGS_CHANGED;
    lret = postMessage(lMessage);
    QIDBG_HIGH("%s:%d: Signaling output port settings changed.",
        __func__, __LINE__);

    if (lret != OMX_ErrorNone) {
      QIDBG_ERROR("%s:%d] cannot post message", __func__, __LINE__);
      delete lMessage;
    }
  }

 return lret;
}

/*==============================================================================
* Function : start
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Start decode of the image. This method is called when the
* etb and ftb are done and the component is in the execute state. Dequeue the
* buffers from the etb and ftb queue and call decode.
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::Start()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QOMX_Buffer *lInputBuffer, *lOutputBuffer;

  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Inavlid state", __func__);
    return OMX_ErrorInvalidState;
  }
  if (m_state != OMX_StateExecuting) {
    QIDBG_ERROR("%s %d]: Incorrect state %d", __func__, __LINE__,
      (int)m_state);
    return OMX_ErrorIncorrectStateOperation;
  }

  pthread_mutex_lock(&m_ftbQLock);
  lOutputBuffer = (QOMX_Buffer *)m_ftbQueue.Dequeue();
  pthread_mutex_unlock(&m_ftbQLock);
  if ((NULL == lOutputBuffer) || (NULL == lOutputBuffer->getBuffer())) {
    QIDBG_ERROR("%s %d]: output Buffer is NULL", __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }
  //Assign the current o/p buffer
  m_currentOutBuffHdr = lOutputBuffer->getBuffer();
  delete lOutputBuffer;
  lOutputBuffer = NULL;
  //Call Decode Image
  lret = decodeImage(m_currentInBuffHdr, m_currentOutBuffHdr);
  if (lret != OMX_ErrorNone) {
   //Transition to InvalidState?
  }
 return lret;
}

/*==============================================================================
* Function : abortExecution
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Handle the component moving to from Executing/Pause state to idle
* state. Abort the current decoding session and return the unprocessed buffers.
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::abortExecution()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lrc = 0;


  //Release Buffers associated with current decode
  lret = releaseCurrentSession();
  //Flush out the message queue
  if (m_queue.Count() > 0) {
    m_queue.DeleteAll();
  }
  //Flush all the ports to return any unprocessed buffers they are holding
  flushBufferQueues(OMX_ALL);

  return lret;
}

/*==============================================================================
* Function : omx_component_allocate_buffer
* Parameters: hComp, bufferHdr, port, appData, bytes
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_allocate_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr, OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 bytes)
{
  return OMX_ErrorNotImplemented;
}

/*==============================================================================
* Function : CanFreeBuffers
* Parameters: aPort, aBuffer, pBufferData
* Return Value : OMX_BOOL
* Description: This function is used to check if all the buffers are passed to
*              the component.
==============================================================================*/
OMX_BOOL OMXImageDecoder::CanFreeBuffers(OMX_PARAM_PORTDEFINITIONTYPE *aPort,
  OMX_BUFFERHEADERTYPE *aBuffer, QOMX_Buffer_Data_t *pBufferData)
{
  int i = 0;
  OMX_BOOL lCanFreeBuffers = OMX_TRUE;

  for (i = 0; i < (int)aPort->nBufferCountActual; i++) {
    if (aBuffer == &pBufferData[i].mHeader) {
      pBufferData[i].valid = OMX_FALSE;
      break;
    }
  }

  for (i = 0; i < (int)aPort->nBufferCountActual; i++) {
    if (pBufferData[i].valid == OMX_TRUE) {
      lCanFreeBuffers = OMX_FALSE;
      break;
    }
  }
  return lCanFreeBuffers;
}


/*==============================================================================
* Function : qomx_component_free_buffer
* Parameters: hComp, port, buffer
* Return Value : OMX_ERRORTYPE
* Description: This function is called by the client to free the buffer headers
* allocated during UseBuffer/Allocate Buffer functions
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_free_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_BUFFERHEADERTYPE *buffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int i = 0;

  QIDBG_MED("%s:%d] state %d", __func__, __LINE__, m_state);
  if (m_state == OMX_StateIdle) {
    if (m_compTransState != OMX_StateLoaded_Pending) {
      QIDBG_ERROR("%s:%d: Component needs to be in transition to Loaded state "
        "to free buffers", __func__, __LINE__);
      return OMX_ErrorIncorrectStateOperation;
    }
  } else {
    QIDBG_ERROR("%s:%d: Invalid State operation. Current state is %d", __func__,
      __LINE__, m_state);
    return OMX_ErrorIncorrectStateOperation;
  }

  if (port == m_inPort->nPortIndex) {
    if (CanFreeBuffers(m_inPort, buffer, mInOMXBufferData)) {
      QIDBG_HIGH("%s:%d]: Releasing %d input bufferheaders",
        __func__, __LINE__, (int)m_inPort->nBufferCountActual);
      if (m_inBuffAllocCount > 0) {
        if (mInOMXBufferData) {
          delete[] mInOMXBufferData;
          mInOMXBufferData = NULL;
        }
        m_inBuffAllocCount = 0;
      }
    }
  } else if (port == m_outPort->nPortIndex) {
    if (CanFreeBuffers(m_outPort, buffer, mOutOMXBufferData)) {
      QIDBG_HIGH("%s:%d]: Releasing %d output bufferheaders",
        __func__, __LINE__, (int)m_outPort->nBufferCountActual);
      if (m_outBuffAllocCount > 0) {
        if (mOutOMXBufferData) {
          delete[] mOutOMXBufferData;
          mOutOMXBufferData = NULL;
        }
        m_outBuffAllocCount = 0;
      }
    }
  } else {
    QIDBG_ERROR("%s:%d]: Invalid Port Index %d", __func__, __LINE__,
      (int)port);
    lret = OMX_ErrorBadPortIndex;
  }
  if ((m_inBuffAllocCount == 0) && (m_outBuffAllocCount == 0) &&
    (m_compTransState == OMX_StateLoaded_Pending)) {
    QIMessage *lMessage = new QIMessage();
    if (lMessage == NULL) {
      QIDBG_ERROR("%s:%d] cannot create message", __func__, __LINE__);
      lret = OMX_ErrorInsufficientResources;
    } else {
      m_state = OMX_StateLoaded;
      m_compTransState = OMX_StateNone;
      lMessage->iData = m_state;
      lMessage->m_qMessage = OMX_MESSAGE_CHANGE_STATE_DONE;
      lret = postMessage(lMessage);
      QIDBG_HIGH("%s:%d: Free Buffer complete: State change from"
        "OMX_StateLoadedPending to OMX_StateLoaded", __func__, __LINE__);
      if (lret != OMX_ErrorNone) {
        QIDBG_ERROR("%s:%d] cannot post message", __func__, __LINE__);
        delete lMessage;
      }
    }
  }
  return lret;
}


/*==============================================================================
* Function : omx_component_role_enum
* Parameters: OMX_HANDLETYPE hComp, OMX_U8 *role, OMX_U32 index
* Return Value : OMX_ERRORTYPE
* Description: Handle the transition to the invalid state
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_role_enum(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_OUT OMX_U8 *role,
  OMX_IN OMX_U32 index)
{
  return OMX_ErrorNotImplemented;
}

/*==============================================================================
* Function : abortMessageThread
* Parameters: none
* Return Value : none
* Description: This function will abort the message handler thread
==============================================================================*/
void OMXImageDecoder::abortMessageThread()
{
  QIDBG_MED("%s %d]: mState %d", __func__, __LINE__, m_state);

  pthread_mutex_lock(&m_queueLock);
  m_thread_exit_flag = OMX_TRUE;
  pthread_cond_signal(&m_queueCond);
  pthread_mutex_unlock(&m_queueLock);
  m_messageThread.JoinThread();
}

/*==============================================================================
* Function : omx_component_deinit
* Parameters: hComp
* Return Value : OMX_ERRORTYPE
* Description: This function will deinitiale the component.
==============================================================================*/
OMX_ERRORTYPE OMXImageDecoder::omx_component_deinit(OMX_IN OMX_HANDLETYPE hComp)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int i = 0;

  QIDBG_MED("%s %d]: ", __func__, __LINE__);

  if (hComp == NULL) {
    QIDBG_ERROR("%s %d]: Bad parameter", __func__, __LINE__);
    return OMX_ErrorBadParameter;
  }

  if (m_messageThread.IsSelf()) {
    QIDBG_ERROR("%s %d]: Deinit called from the message thread.. Not allowed",
      __func__, __LINE__);
    return OMX_ErrorUndefined;
  }

  pthread_mutex_lock(&m_compLock);
  if (m_compInitialized) {
    if ((m_state == OMX_StateLoaded) || m_state == OMX_StateInvalid) {
      QIDBG_MED("%s %d]: ", __func__, __LINE__);
      abortMessageThread();
    }
    QIDBG_MED("%s %d]: m_state %d", __func__, __LINE__, m_state);
    if (m_state == OMX_StateExecuting) {
      abortExecution();
      if (m_dataAllocated) {
        for (i = 0; i < (int)m_inPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_inPort->nPortIndex,
            &mInOMXBufferData[i].mHeader);
        }
        for (i = 0; i < (int)m_outPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_outPort->nPortIndex,
            &mOutOMXBufferData[i].mHeader);
        }
      }
      m_state = OMX_StateInvalid;
      abortMessageThread();
    }
    if (m_state == OMX_StateIdle) {
      if (m_dataAllocated) {
        for (i = 0; i < (int)m_inPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_inPort->nPortIndex,
            &mInOMXBufferData[i].mHeader);
        }
        for (i = 0; i < (int)m_outPort->nBufferCountActual; i++) {
          omx_component_free_buffer(m_compHandle, m_outPort->nPortIndex,
            &mOutOMXBufferData[i].mHeader);
        }
      }
      m_state = OMX_StateInvalid;
      abortMessageThread();
    }
  }
  m_compInitialized = OMX_FALSE;
  pthread_mutex_unlock(&m_compLock);
  return OMX_ErrorNone;
}

