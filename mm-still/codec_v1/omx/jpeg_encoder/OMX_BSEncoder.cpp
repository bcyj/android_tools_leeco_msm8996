/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include "OMX_BSEncoder.h"



/*==============================================================================
* Function : omx_component_use_buffer
* Parameters: bufferHdr, port, appData, bytes, buffer
* Return Value : OMX_ERRORTYPE
* Description: Set the buffers passed by the OMX client
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_use_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE** bufferHdr,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN OMX_U32 bytes,
  OMX_IN OMX_U8* buffer)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  if (bufferHdr == NULL || buffer == NULL || bytes == 0 || appData == NULL) {
    QIDBG_ERROR("%s : bad param 0x%p %ld 0x%p", __func__, bufferHdr, bytes,
      buffer);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s : Invalid State", __func__);
    return OMX_ErrorInvalidState;
  }
  pthread_mutex_lock(&m_compLock);
  if (( port == (OMX_U32) OMX_INPUT_PORT_INDEX )) {
      if (m_inPort->bEnabled) {
        if (bytes != m_inPort->nBufferSize) {
          QIDBG_ERROR("%s : %d exceeds the buffer size requested previously = "
            "%d\n",__func__, (int)bytes, (int)m_inPort->nBufferSize);
           pthread_mutex_unlock(&m_compLock);
           return OMX_ErrorBadParameter;
         }
        mInOMXBufferData->mInfo = reinterpret_cast<QOMX_BUFFER_INFO *> (appData);
        mInOMXBufferData->mInfo->address = buffer;
        mInOMXBufferData->mInfo->offset = bytes;
        lret = use_input_buffer(bufferHdr, bytes, buffer, appData, port);
        if (lret == OMX_ErrorNone) {
          m_inPort->bPopulated = OMX_TRUE;
        }
      } else {
        QIDBG_ERROR("%s : Error I/p port disabled\n", __func__);
        lret = OMX_ErrorNotReady;
      }
  }

  if (port == (OMX_U32) OMX_OUTPUT_PORT_INDEX ) {
      if (m_outPort->bEnabled) {
        if (bytes != m_outPort->nBufferSize) {
          QIDBG_ERROR("%s : %d exceeds the buffer size requested previously = "
            "%d\n",__func__, (int)bytes, (int)m_outPort->nBufferSize);
           pthread_mutex_unlock(&m_compLock);
           return OMX_ErrorBadParameter;
         }
        lret = use_output_buffer(bufferHdr, bytes, buffer, appData, port);
        if (lret == OMX_ErrorNone) {
          m_outPort->bPopulated = OMX_TRUE;
        }
      } else {
        QIDBG_ERROR("%s : Error O/p port disabled\n", __func__);
        lret = OMX_ErrorNotReady;
      }
  }

  if ((port == (OMX_U32) OMX_INPUT_THUMBNAIL_PORT_INDEX)) {
    if (m_thumbnail_port->bEnabled ) {
      if (bytes != m_inPort->nBufferSize) {
        QIDBG_ERROR("%s : %d exceeds the buffer size requested previously = "
          "%d\n",__func__, (int)bytes, (int)m_thumbnail_port->nBufferSize);
        pthread_mutex_unlock(&m_compLock);
        return OMX_ErrorBadParameter;
       }
      m_thumbBufferInfo = reinterpret_cast<QOMX_BUFFER_INFO *> (appData);
      m_thumbBufferInfo->address = buffer;
      m_thumbBufferInfo->offset = bytes;
      lret =  use_input_buffer(bufferHdr, bytes, buffer, appData, port);
      if (lret == OMX_ErrorNone) {
        m_thumbnail_port->bPopulated = OMX_TRUE;
      }
    } else {
        QIDBG_ERROR("%s: Error Thumbnail Port not enabled\n", __func__);
        lret = OMX_ErrorNotReady;
    }
  }

  //Check if all the required ports are populated and set the flag
  if (m_inPort->bPopulated && m_outPort->bPopulated) {
      m_dataAllocated = OMX_TRUE;
      if ((m_thumbnail_port->bEnabled) && (!m_thumbnail_port->bPopulated)) {
        //Thumbnail port is enabled and is yet to be populated
        m_dataAllocated = OMX_FALSE;
      }
  }

  //If all the ports are populated and the component is in the idle pending
  //State transition to the Idle state
  if (m_dataAllocated && m_compTransState == OMX_StateIdle_Pending) {
      m_state = OMX_StateIdle;
      m_compTransState = OMX_StateNone;
      m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventCmdComplete,
        OMX_CommandStateSet, OMX_StateIdle, NULL);
  }
  pthread_mutex_unlock(&m_compLock);
  return lret;
 }



/*==============================================================================
* Function : port_enable
* Parameters: OMX_U32 portIndex
* Return Value : OMX_ERRORTYPE
* Description: This method processes the commands/events from the message queue
==============================================================================*/
void OMXImageEncoder:: portEnable(OMX_U32 a_portIndex)
{
    if (m_state == OMX_StateInvalid) {
      QIDBG_ERROR("%s : Error Invalid State",__func__);
      m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
        OMX_ErrorIncorrectStateOperation, a_portIndex, NULL);
      }
    if ((a_portIndex == OMX_ALL) || (a_portIndex == m_inPort->nPortIndex)) {
       m_inPort->bEnabled = OMX_TRUE;
       //If comp is in state loaded and not in transition to idle,enable the
       //port
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
    } else if ((a_portIndex == OMX_ALL) ||
        (a_portIndex == m_thumbnail_port->nPortIndex)) {
        m_thumbnail_port->bEnabled = OMX_TRUE;
        if (((m_state == OMX_StateLoaded) &&
         (!(m_compTransState == OMX_StateIdle_Pending))) ||
         (m_state == OMX_StateWaitForResources)) {
          m_callbacks->EventHandler(m_compHandle, m_appData,
          OMX_EventCmdComplete, OMX_CommandPortEnable,
          m_thumbnail_port->nPortIndex, NULL);
        } else {
            m_thumbPortTransState = OMX_PORT_ENABLE_PENDING;
        }
    } else {
        QIDBG("%s: ERROR Invalid Port Index\n", __func__);
        m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
         OMX_CommandPortEnable, a_portIndex, NULL);
    }
    //ToDo:Handle port enable on OMX_StateIdle and OMX_StateExecuting
    //in the OMX_StateExecuting state, then that port shall begin
    //transferring buffers
    if (m_inPort->bEnabled && m_outPort->bEnabled) {
        if (m_state == OMX_StateExecuting) {
          //StartEncode?
        }
        if (m_state == OMX_StateIdle && (!m_state == OMX_StateIdle_Pending)) {
          //ToDo
        }
    }
}


/*==============================================================================
* Function : omx_component_empty_this_buffer
* Parameters: OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE buffer
* Return Value : OMX_ERRORTYPE
* Description: The client makes this call with the input buffer indicating the
* component to start processing the input buffer
==============================================================================*/
OMX_ERRORTYPE OMXImageEncoder::omx_component_empty_this_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  OMX_ERRORTYPE lrc = OMX_ErrorNone;
  if (hComp == NULL || buffer == NULL ||
    (buffer->nSize != sizeof(OMX_BUFFERHEADERTYPE))) {
    QIDBG_ERROR("%s: Bad Parameter", __func__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Invalid State\n", __func__);
    return OMX_ErrorInvalidState;
  }
  if ((m_state != OMX_StateExecuting) || (m_state != OMX_StatePause)) {
    QIDBG_ERROR("%s: Current State is %d. Operation Not allowed",
      __func__, m_state);
    return OMX_ErrorInvalidState;
  }
  if ((buffer->nVersion.nVersion != OMX_SPEC_VERSION)) {
    QIDBG_ERROR("%s: Error - Invalid version\n ", __func__);
    return OMX_ErrorVersionMismatch;
  }
  if (!(buffer->nInputPortIndex == m_inPort->nPortIndex) ||
    !(buffer->nInputPortIndex == m_thumbnail_port->nPortIndex)) {
      QIDBG_ERROR("%s : bad Port index %d\n",
        __func__,(int)buffer->nInputPortIndex);
      return OMX_ErrorBadPortIndex;
  }
  if(((buffer->nInputPortIndex == m_inPort->nPortIndex) &&
    (!m_inPort->bEnabled) ||
    ((buffer->nInputPortIndex == m_thumbnail_port->nPortIndex) &&
    (!m_thumbnail_port->bEnabled)))){
      QIDBG_ERROR("%s: Error Port not enabled %d\n",
        __func__, (int)buffer->nInputPortIndex);
      return OMX_ErrorIncorrectStateOperation;
    }
  if (!(buffer->pBuffer == m_inBuffHeaders->pBuffer) ||)
       (!(buffer->pBuffer == m_thumb{
  }

  return lrc;
}


/*==============================================================================
* Function : omx_component_init
* Parameters: hComp
* Return Value : OMX_ERRORTYPE
* Description: Initialize the OMX Encoder Component with relevant data
==============================================================================*/
OMX_ERRORTYPE omx_bsencoder::omx_component_init(OMX_IN OMX_HANDLETYPE hComp)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  if (hComp == NULL) {
     QIDBG_ERROR("%s: Bad paramerter\n", __func__);
     return OMX_ErrorBadParameter;
  }

  lret = OMXImageEncoder::omx_component_init(hComp);
  if (lret != OMX_ErrorNone) {
     return lret;
  }

  //Initialize the Thumbnail Port
  memset(m_thumbnail_port, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
  m_thumbnail_port->nPortIndex = OMX_INPUT_THUMBNAIL_PORT_INDEX;
  initialize_thumbnail_port(m_thumbnail_port);

  m_thumbPortTransState = OMX_PORT_NONE;

  return lret;

}

/*==============================================================================
* Function : initialize_thumbnail_port
* Parameters: OMX_PARAM_PORTDEFINITIONTYPE
* Return Value : OMX_ERRORTYPE
* Description: Initialize the thumbnail with relevant data
==============================================================================*/
void OMXImageEncoder::initialize_thumbnail_port(
  OMX_PARAM_PORTDEFINITIONTYPE *a_thumbPort)
{
  a_thumbPort->nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
  a_thumbPort->nVersion.nVersion = OMX_SPEC_VERSION;
  a_thumbPort->eDir = OMX_DirInput;
  a_thumbPort->nBufferCountActual = 1;
  a_thumbPort->nBufferCountMin = 1;
  a_thumbPort->bEnabled = OMX_TRUE;
  a_thumbPort->bPopulated = OMX_FALSE;
  a_thumbPort->eDomain = OMX_PortDomainImage;
  a_thumbPort->bBuffersContiguous = OMX_TRUE;
  a_thumbPort->nBufferAlignment = 4096;
  a_thumbPort->format.image.cMIMEType = NULL;
  a_thumbPort->format.image.bFlagErrorConcealment = OMX_FALSE;
  a_thumbPort->format.image.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  a_thumbPort->format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;

  return;
}

