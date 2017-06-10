/*******************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include "QOMXImageCodec.h"

/*==============================================================================
* Function : QOMXImageCodec
* Parameters: None
* Return Value : None
* Description: Constructor
==============================================================================*/
QOMXImageCodec::QOMXImageCodec()
{
  pthread_mutex_init(&m_compLock, NULL);
  pthread_mutex_init(&m_queueLock, NULL);
  pthread_mutex_init(&m_etbQLock, NULL);
  pthread_mutex_init(&m_ftbQLock, NULL);
  pthread_mutex_init(&m_abortlock, NULL);
  sem_init(&m_cmdLock, 0, 0);
  m_abort_flag = FALSE;
  m_thread_exit_flag = OMX_FALSE;
  m_etbFlag = OMX_FALSE;
  m_ftbFlag = OMX_FALSE;
  m_executionComplete = OMX_FALSE;
  m_numOfPlanes = 2;
  m_numOfComponents = 3;
  m_outputQIBuffer = NULL;
  m_inputQIBuffer = NULL;
  mInOMXBufferData = NULL;
  mOutOMXBufferData = NULL;
  m_currentOutBuffHdr = NULL;
  m_currentInBuffHdr = NULL;
  m_outBuffAllocCount = 0;
  m_inBuffAllocCount = 0;
  m_dataAllocated = OMX_FALSE;
  m_compInitialized = OMX_FALSE;
  m_numOfPlanes = 0;
  m_numOfComponents = 0;
  m_outputFormatTypes = NULL;
  m_inputFormatTypes = NULL;
  m_outPort = NULL;
  m_inPort = NULL;
  m_imagePortParam = NULL;
  m_callbacks = NULL;

}

/*==============================================================================
* Function : ~QOMXImageCodec
* Parameters: None
* Return Value : None
* Description: Destructor
==============================================================================*/
QOMXImageCodec::~QOMXImageCodec()
{
  pthread_mutex_destroy(&m_compLock);
  pthread_mutex_destroy(&m_queueLock);
  pthread_mutex_destroy(&m_etbQLock);
  pthread_mutex_destroy(&m_ftbQLock);
  pthread_mutex_destroy(&m_abortlock);
  sem_destroy(&m_cmdLock);
  if (m_imagePortParam) {
    delete(m_imagePortParam);
  }
  if (m_inPort) {
    delete(m_inPort);
  }
  if (m_outPort) {
    delete(m_outPort);
  }
  if (m_inputFormatTypes) {
    delete(m_inputFormatTypes);
  }
  if (m_outputFormatTypes) {
    delete(m_outputFormatTypes);
  }
}

/*==============================================================================
* Function : omx_component_tunnel_request
* Parameters: hComp, port, peerComponent, peerPort, tunnelSetup
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_tunnel_request(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_HANDLETYPE peerComponent,
  OMX_IN OMX_U32 peerPort,
  OMX_INOUT OMX_TUNNELSETUPTYPE *tunnelSetup)
{
  if((hComp == NULL) || (peerComponent == NULL) || (tunnelSetup == NULL) ) {
    port = 0;
    peerPort = 0;
    return OMX_ErrorBadParameter;
  }
  return OMX_ErrorNotImplemented;
}

/*==============================================================================
* Function : omx_component_use_EGL_image
* Parameters: hComp, bufferHdr, port, appData, eglImage
* Return Value : OMX_ERRORTYPE
* Description:
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_use_EGL_image(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_INOUT OMX_BUFFERHEADERTYPE **bufferHdr,
  OMX_IN OMX_U32 port,
  OMX_IN OMX_PTR appData,
  OMX_IN void *eglImage)
{
  if((hComp == NULL) || (appData == NULL) || (eglImage == NULL)) {
    bufferHdr = NULL;
    port = 0;
    return OMX_ErrorBadParameter;
  }
  return OMX_ErrorNotImplemented;
}


/*==============================================================================
* Function : omx_component_set_callbacks
* Parameters: hComp, buffer
* Return Value : OMX_ERRORTYPE
* Description: Set the callback function from the OMX client. Also saves the
* appData and comphandle for future use.
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_set_callbacks(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_CALLBACKTYPE *callbacks,
  OMX_IN OMX_PTR appData)
{
  if((hComp == NULL) || (callbacks == NULL) ||
    (callbacks->EmptyBufferDone == NULL) ||
    (callbacks->FillBufferDone == NULL)
    || (callbacks->EventHandler == NULL)) {
    QIDBG_ERROR("%s: Bad Parameter",__func__);
    return OMX_ErrorBadParameter;
  }
  m_callbacks = callbacks;
  m_appData = appData;
  m_compHandle = hComp;
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : omx_component_get_version
* Parameters: hComp, componentName, componentVersion, specVersion,
*             componentUUID
* Return Value : OMX_ERRORTYPE
* Description: Get the OMX Specification version being used
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_get_version(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_OUT OMX_STRING componentName,
  OMX_OUT OMX_VERSIONTYPE *componentVersion,
  OMX_OUT OMX_VERSIONTYPE *specVersion,
  OMX_OUT OMX_UUIDTYPE *componentUUID)
{
  if((hComp == NULL) || (componentName == NULL) || (specVersion == NULL) ||
    (componentUUID == NULL)) {
    componentVersion == NULL;
    return OMX_ErrorBadParameter;
  }
  if(m_state == OMX_StateInvalid) {
    return OMX_ErrorInvalidState;
  }
  componentVersion->nVersion = OMX_SPEC_VERSION;
  specVersion->nVersion = OMX_SPEC_VERSION;
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : changeState
* Parameters: OMX_STATETYPE
* Return Value : OMX_ERRORTYPE
* Description: Change the current state of the component to the specified state
* if state transition is valid.
==============================================================================*/
void QOMXImageCodec::changeState(OMX_STATETYPE aState)
{
  QIDBG_HIGH("%s:%d] From Current State: %d To New state: %d",
    __func__, __LINE__, m_state, aState);

  if (m_state == aState) {
    QIDBG_ERROR("%s:%d] Error: Cannot change to same state", __func__,
      __LINE__);
    sem_post(&m_cmdLock);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      OMX_ErrorSameState, 0, NULL);
    return;
  }
  switch (aState) {
  case OMX_StateLoaded: {
    if (OMX_StateIdle == m_state) {
      /*Conditions to transit from IDLE to LOADED
       1.All the ports are disabled
       2.Buffers all released through calls to FreeBuffer _ if not the state
        transition is still pending */
      if (!m_inPort->bEnabled && !m_outPort->bEnabled) {
        // Release codec libraries
        OMX_ERRORTYPE rc = releaseCodecLibs();
        if (rc != OMX_ErrorNone) {
          QIDBG_ERROR("%s: Failed to release codec libraries",
              __func__);
        }
        m_state = aState;
        QIDBG_HIGH("%s: State change from OMX_StateIdle to OMX_StateLoaded",
          __func__);
        sem_post(&m_cmdLock);
        m_callbacks->EventHandler(m_compHandle, m_appData,
          OMX_EventCmdComplete, OMX_CommandStateSet, aState, NULL);
      } else {
        //Wait for freeBuffer to be called to complete transition
        QIDBG_HIGH("%s:%d] State change from OMX_StateIdle to OMX_StateLoaded "
          "Pending", __func__, __LINE__);
        m_compTransState = OMX_StateLoaded_Pending;
        sem_post(&m_cmdLock);
      }
    } else if (OMX_StateWaitForResources == m_state) {
      m_state = aState;
      QIDBG_HIGH("%s:%d] State change from OMX_StateWaitForResources to "
        "OMX_StateLoaded", __func__, __LINE__);
      sem_post(&m_cmdLock);
      m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventCmdComplete,
        OMX_CommandStateSet, aState, NULL);
    } else {
      QIDBG_ERROR("%s:%d] Invalid state transition from %d to OMX_StateLoaded",
        __func__, __LINE__, m_state);
      sem_post(&m_cmdLock);
      m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
        (OMX_U32)OMX_ErrorIncorrectStateTransition, 0, NULL);
    }
    break;
  }
  case OMX_StateIdle: {
    if (OMX_StateLoaded == m_state) {
      /*Conditions to transit from LOADED to IDLE
      1.All the ports are disabled
      2.The buffers for all the enabed ports are allocated either through
      the UseBuffer or AllocatedBuffer function calls.If not the state
      transition is still pending */
      if (!m_inPort->bEnabled && !m_outPort->bEnabled) {
        m_state = aState;
        QIDBG_HIGH("%s:%d] State change from OMX_StateLoaded to "
          "OMX_StateIdle", __func__, __LINE__);
        sem_post(&m_cmdLock);
        OMX_ERRORTYPE rc = preloadCodecLibs();
        if (rc != OMX_ErrorNone) {
          QIDBG_ERROR("%s: Failed to preload codec libraries",
              __func__);
        }
        m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventCmdComplete,
          OMX_CommandStateSet, aState, NULL);
      } else {
        if (m_dataAllocated) {
          m_state = aState;
          QIDBG_HIGH("%s:%d] State change from OMX_StateLoaded to "
            "OMX_StateIdle", __func__, __LINE__);
          sem_post(&m_cmdLock);
          m_callbacks->EventHandler(m_compHandle, m_appData,
            OMX_EventCmdComplete, OMX_CommandStateSet, aState, NULL);
        } else {
          QIDBG_HIGH("%s:%d] State  change incomplete.. Moving to "
            "OMX_StateIdle_Pending", __func__, __LINE__);
          m_compTransState = OMX_StateIdle_Pending;
          //ToDo : call function to allocate?
          sem_post(&m_cmdLock);
        }
      }
    } else if ((OMX_StateExecuting == m_state) ||
      (OMX_StatePause == m_state)) {
      /* Conditions to transit from Execution/Pause to IDLE
       * 1.All the ports are disabled
       * 2.If execution is complete, the buffers are already flushed.
       * 3.If the client wants to abort execution, move to pendig state, abort
       * execution and flush all buffers before moving to stateIdle
       */
      if (!m_inPort->bEnabled && !m_outPort->bEnabled) {
        m_state = aState;
        QIDBG_HIGH("%s:%d] Ports disabled: State change from "
           "OMX_StateExecuting to OMX_StateIdle", __func__, __LINE__);
        sem_post(&m_cmdLock);
        m_callbacks->EventHandler(m_compHandle, m_appData,
          OMX_EventCmdComplete, OMX_CommandStateSet, aState, NULL);
      } else {
        //Abort the execution
        m_compTransState = OMX_StateIdle_Pending;
        pthread_mutex_lock(&m_abortlock);
        m_abort_flag = OMX_TRUE;
        pthread_mutex_unlock(&m_abortlock);
        abortExecution();
        m_state = aState;
        m_compTransState = OMX_StateNone;
        QIDBG_HIGH("%s: %d] State change from OMX_StateExecuting to "
            "OMX_StateIdle", __func__, __LINE__);
        sem_post(&m_cmdLock);
        m_callbacks->EventHandler(m_compHandle, m_appData,
            OMX_EventCmdComplete, OMX_CommandStateSet, aState, NULL);
      }
    } else if (OMX_StateWaitForResources == m_state) {
      m_state = aState;
      QIDBG_HIGH("%s:%d] State change from OMX_StateWaitForResources to"
       "OMX_StateIdle", __func__, __LINE__);
      sem_post(&m_cmdLock);
      m_callbacks->EventHandler(m_compHandle, m_appData,
        OMX_EventCmdComplete, OMX_CommandStateSet, aState, NULL);
    } else {
      QIDBG_ERROR("%s:%d] Invalid state transition from %d to OMX_StateIdle",
        __func__, __LINE__, m_state);
      sem_post(&m_cmdLock);
      m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
        (OMX_U32)OMX_ErrorIncorrectStateTransition, 0, NULL);
    }
    break;
  }

  case OMX_StateExecuting: {
    if (OMX_StateIdle == m_state) {
      /*Conditions to transit from idle to Executing
      1.All the ports are disabled
      2.All the ports are enabled, start execution (since they are in Idle
        the buffers will be allocated)
      */
      if (!m_inPort->bEnabled && !m_outPort->bEnabled) {
        m_state = aState;
        QIDBG_HIGH("%s:%d] State change from OMX_StateIdle to "
          "OMX_StateExecuting", __func__, __LINE__);
        sem_post(&m_cmdLock);
        m_callbacks->EventHandler(m_compHandle, m_appData,
          OMX_EventCmdComplete, OMX_CommandStateSet, aState, NULL);
      } else if (m_inPort->bEnabled) {
       //Change state to Executing
        m_state = aState;
        QIDBG_HIGH("%s:%d] State change from OMX_StateIdle to "
        "OMX_StateExecuting", __func__, __LINE__);
        sem_post(&m_cmdLock);
        m_callbacks->EventHandler(m_compHandle, m_appData,
        OMX_EventCmdComplete, OMX_CommandStateSet, aState, NULL);
      } else {
        QIDBG_ERROR("%s:%d] Cannot transition from Idle to execution state",
          __func__, __LINE__);
        m_compTransState = OMX_StateExecuting_Pending;
      }
    } else if (OMX_StatePause == m_state) {
      //To do : Start encode/Decode again and change state
    } else {
      QIDBG_ERROR("%s:%d] Invalid state transition from %d to "
        "OMX_StateExecuting", __func__, __LINE__, m_state);
      sem_post(&m_cmdLock);
      m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
        (OMX_U32)OMX_ErrorIncorrectStateTransition, 0, NULL);
    }
    break;
  }

  case OMX_StatePause:{
    if (OMX_StateExecuting == m_state) {
      //ToDo
    } else if (OMX_StateIdle == m_state) {
      //Todo
    } else {
      QIDBG_ERROR("%s:%d] Invalid state transition from %d to "
        "OMX_StatePause", __func__, __LINE__, m_state);
      sem_post(&m_cmdLock);
      m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
        (OMX_U32)OMX_ErrorIncorrectStateTransition, 0, NULL);
    }
    break;
  }

  case OMX_StateWaitForResources: {
    if (OMX_StateLoaded == m_state) {
      m_state = aState;
      QIDBG_HIGH("%s:%d] transition from OMX_StateLoaded to "
        "OMX_StateWaitForResources", __func__, __LINE__);
      sem_post(&m_cmdLock);
      m_callbacks->EventHandler(m_compHandle, m_appData,
        OMX_EventCmdComplete, OMX_CommandStateSet, aState, NULL);
    } else {
      QIDBG_ERROR("%s:%d] Invalid state transition from %d to "
      "OMX_StateWaitForResources", __func__, __LINE__, m_state);
      sem_post(&m_cmdLock);
      m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
        (OMX_U32)OMX_ErrorIncorrectStateTransition, 0, NULL);
    }
    break;
  }

  case OMX_StateInvalid:{
    //ToDo: Need to Stop irrespective of what the component state is
    break;
  }

  default: {
    QIDBG_ERROR("%s:%d] Invalid state %d ", __func__, __LINE__, aState);
    sem_post(&m_cmdLock);
    m_callbacks->EventHandler(m_compHandle, m_appData, OMX_EventError,
      (OMX_U32)OMX_ErrorBadParameter, 0, NULL);
    break;
  }
  } /*end of switch case*/
}

/*==============================================================================
* Function : post_Message
* Parameters: Queue of type QIBase
* Return Value : OMX_ERRORTYPE
* Description: This method posts a Message to the queue and invokes the message
* thread
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::postMessage(QIBase *a_Message)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  if (!a_Message) {
    return OMX_ErrorBadParameter;
  }
  pthread_mutex_lock(&m_queueLock);
  if (QI_ERROR(m_queue.Enqueue(a_Message))) {
    QIDBG_MED("%s:%d] cannot enqueue message", __func__, __LINE__);
    pthread_mutex_unlock(&m_queueLock);
    return OMX_ErrorInsufficientResources;
  }
  QIDBG_MED("%s:%d] Message addr = %p", __func__, __LINE__, a_Message);
  pthread_cond_signal(&m_queueCond);
  pthread_mutex_unlock(&m_queueLock);

  return OMX_ErrorNone;
}
/*==============================================================================
* Function : handle_Message
* Parameters: Queue of type QIBase
* Return Value : OMX_ERRORTYPE
* Description: This method handles the messages posted to the queue. The message
* thread waits for the signal from post_message to handle the next message in
* the message queue.
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::handleMessage()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  QIMessage *l_message;

  QIDBG_MED("%s: E",__func__);

  while (true) {
    pthread_mutex_lock(&m_queueLock);
    //Wait till it has been signalled by post_message
    QIDBG_MED("%s:%d] before wait", __func__, __LINE__);

    while ((m_queue.Count() == 0) && (!m_thread_exit_flag)) {
      pthread_cond_wait(&m_queueCond, &m_queueLock);
      QIDBG_MED("%s:%d] after cond wait", __func__, __LINE__);
    }

    QIDBG_MED("%s:%d] after wait", __func__, __LINE__);
    if (m_thread_exit_flag) {
      pthread_mutex_unlock(&m_queueLock);
      break;
    }
    l_message = reinterpret_cast <QIMessage *> (m_queue.Dequeue());
    if (!l_message) {
      QIDBG_ERROR("%s:%d] Message is Null", __func__, __LINE__);
      pthread_mutex_unlock(&m_queueLock);
      return OMX_ErrorBadParameter;
    }
    QIDBG_MED("%s:%d] Message addr = %p", __func__, __LINE__, l_message);
    pthread_mutex_unlock(&m_queueLock);
    lret = processMessage(l_message);
  }
  return lret;
}

/*==============================================================================
* Function : omx_component_get_state
* Parameters: hComp, state
* Return Value : OMX_ERRORTYPE
* Description: Get the current state of the OMX component
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_get_state(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_OUT OMX_STATETYPE* state)
{
  if(hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
  *state = m_state;
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : omx_component_send_command
* Parameters: hComp, cmd, param1, cmdData
* Return Value : OMX_ERRORTYPE
* Description: This method will invoke a command on the component.
* This is a non-blocking
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_send_command(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_COMMANDTYPE cmd,
  OMX_IN OMX_U32 param1,
  OMX_IN OMX_PTR cmdData)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;

  QIDBG_LOW("%s:%d] E", __func__, __LINE__);
  if (hComp == NULL) {
    QIDBG_ERROR("%s: Invalid Parameter", __func__);
    return OMX_ErrorBadParameter;
  }

  QIMessage *l_message = new QIMessage();
  if (l_message == NULL) {
    QIDBG_ERROR("%s: cannot create message", __func__);
    return OMX_ErrorInsufficientResources;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s:%d] Invalid State", __func__, __LINE__);
    if (l_message) {
      delete l_message;
      l_message = NULL;
    }
    return OMX_ErrorInvalidState;
  }

  QIDBG_LOW("%s:%d] E", __func__, __LINE__);

  pthread_mutex_lock(&m_compLock);
  switch (cmd) {
  case OMX_CommandStateSet: {
    l_message->m_qMessage = OMX_MESSAGE_CHANGE_STATE;
    l_message->iData = param1;
    break;
  }
  case OMX_CommandPortEnable: {
    l_message->m_qMessage = OMX_MESSAGE_PORT_ENABLE;
    l_message->iData = param1;
    break;
  }
  case OMX_CommandPortDisable: {
    l_message->m_qMessage = OMX_MESSAGE_PORT_DISABLE;
    l_message->iData = param1;
    break;
  }
  case OMX_CommandFlush: {
    l_message->m_qMessage = OMX_MESSAGE_FLUSH;
    l_message->iData = param1;
    break;
  }
  default: {
    QIDBG_ERROR("%s: Invalid Command %d",__func__, cmd);
    if (l_message != NULL) {
      delete l_message;
      l_message = NULL;
    }
    pthread_mutex_unlock(&m_compLock);
    return OMX_ErrorBadParameter;
  }
  }
  lret = postMessage(l_message);

  QIDBG_LOW("%s:%d] E", __func__, __LINE__);

  /* wait for message thread to process the command*/
  sem_wait(&m_cmdLock);

  QIDBG_LOW("%s:%d] E", __func__, __LINE__);

  pthread_mutex_unlock(&m_compLock);
  return lret;
}

/*==============================================================================
* Function : omx_component_send_command
* Parameters: hComp, cmd, param1, cmdData
* Return Value : OMX_ERRORTYPE
* Description: This method will invoke a command on the component.
* This is a non-blocking
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_get_extension_index(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_STRING paramName,
  OMX_OUT OMX_INDEXTYPE *indexType)
{
  if (paramName == NULL) {
    QIDBG_ERROR("%s: Param Name is NULL", __func__);
    return OMX_ErrorBadParameter;
  }
  if(indexType == NULL) {
    QIDBG_ERROR("%s: Index Name is NULL", __func__);
    return OMX_ErrorBadParameter;
  }
  if (m_state == OMX_StateInvalid) {
    QIDBG_ERROR("%s: Inavlid state", __func__);
    return OMX_ErrorInvalidState;
  }
  QIDBG_MED("%s: %d] Extension Name: %s", __func__, __LINE__, paramName);
  pthread_mutex_lock(&m_compLock);
  if (strncmp("OMX.QCOM.image.exttype.exif", paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE)QOMX_IMAGE_EXT_EXIF;
  } else if (strncmp("OMX.QCOM.image.exttype.thumbnail", paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE) QOMX_IMAGE_EXT_THUMBNAIL;
  } else if (strncmp("OMX.QCOM.image.exttype.bufferOffset", paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE) QOMX_IMAGE_EXT_BUFFER_OFFSET;
  } else if(strncmp("OMX.QCOM.image.exttype.mobicat", paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE) QOMX_IMAGE_EXT_MOBICAT;
  } else if(strncmp(QOMX_IMAGE_EXT_ENCODING_MODE_NAME, paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE) QOMX_IMAGE_EXT_ENCODING_MODE;
  } else if (strncmp(QOMX_IMAGE_EXT_WORK_BUFFER_NAME, paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE) QOMX_IMAGE_EXT_WORK_BUFFER;
  } else if (strncmp(QOMX_IMAGE_EXT_METADATA_NAME, paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE) QOMX_IMAGE_EXT_METADATA;
  } else if (strncmp(QOMX_IMAGE_EXT_META_ENC_KEY_NAME, paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE) QOMX_IMAGE_EXT_META_ENC_KEY;
  } else if (strncmp(QOMX_IMAGE_EXT_MEM_OPS_NAME, paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE) QOMX_IMAGE_EXT_MEM_OPS;
  } else if (strncmp(QOMX_IMAGE_EXT_JPEG_SPEED_NAME, paramName,
    strlen(paramName)) == 0) {
    *indexType = (OMX_INDEXTYPE) QOMX_IMAGE_EXT_JPEG_SPEED;
  } else {
    QIDBG_ERROR("%s:%d] Unsupported extention type", __func__, __LINE__);
    pthread_mutex_unlock(&m_compLock);
    return OMX_ErrorBadParameter;
  }
  pthread_mutex_unlock(&m_compLock);
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : omx_component_empty_this_buffer
* Parameters: OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE buffer
* Return Value : OMX_ERRORTYPE
* Description: The client makes this call with the input buffer indicating the
* component to start processing the input buffer
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_empty_this_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  if(hComp == NULL  || buffer == NULL) {
    return OMX_ErrorBadParameter;
  }
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : omx_component_fill_this_buffer
* Parameters: OMX_HANDLETYPE hComp, OMX_BUFFERHEADERTYPE buffer
* Return Value : OMX_ERRORTYPE
* Description: The client makes this call with the out buffer indicating the
* component to start processing the output buffer
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_fill_this_buffer(
  OMX_IN OMX_HANDLETYPE hComp,
  OMX_IN OMX_BUFFERHEADERTYPE* buffer)
{
  if(hComp == NULL  || buffer ==NULL) {
    return OMX_ErrorBadParameter;
  }
  return OMX_ErrorNone;
}
/*==============================================================================
* Function : empty_this_buffer_done
* Parameters: none
* Return Value : none
* Description: Call the Empty Buffer Done callback on the client once the
* input buffer has been processed.In case of the component
* state changing to INVALID or disabling of the input port, send back the
* buffers that are yet to be processed through EmptyBufferDone callback
==============================================================================*/
void QOMXImageCodec::emptyBufferDone(OMX_BUFFERHEADERTYPE *abufferHdr)
{
  QIDBG_MED("%s %d]", __func__, __LINE__);
  pthread_mutex_lock(&m_etbQLock);
  m_etbFlag = OMX_FALSE;
  pthread_mutex_unlock(&m_etbQLock);
  m_callbacks->EmptyBufferDone(m_compHandle, m_appData, abufferHdr);
}

/*==============================================================================
* Function : fill_this_buffer_done
* Parameters: none
* Return Value : none
* Description: Call the Fill Buffer Done callback on the client once the
* output buffer has been filled after encoding/decoding.In case of the component
* state changing to INVALID or disabling of the output port, send back the
* buffers that are yet to be processed through FillBufferDone callback.
==============================================================================*/
void QOMXImageCodec::fillBufferDone(OMX_BUFFERHEADERTYPE *abufferHdr)
{
  QIDBG_MED("%s %d]", __func__, __LINE__);
  pthread_mutex_lock(&m_ftbQLock);
  m_ftbFlag = OMX_FALSE;
  pthread_mutex_unlock(&m_ftbQLock);
  m_callbacks->FillBufferDone(m_compHandle, m_appData, m_currentOutBuffHdr);
}

/*==============================================================================
* Function : getOmxFormat
* Parameters: atarget, aformat, asub
* Return Value : none
* Description: Given QIFormat and subsampling, derive the equivalent OMX format
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::getOmxFormat(OMX_U32 *atarget,
    QIFormat aformat, QISubsampling asub)
{
  switch (aformat) {
  case QI_YCRCB_SP:
    switch (asub) {
    case QI_H2V2:
      *atarget = OMX_QCOM_IMG_COLOR_FormatYVU420SemiPlanar;
      break;
    case QI_H2V1:
      *atarget = OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar;
      break;
    case QI_H1V2:
      *atarget = OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar_h1v2;
      break;
    case QI_H1V1:
      *atarget = OMX_QCOM_IMG_COLOR_FormatYVU444SemiPlanar;
      break;
    default:
      *atarget = OMX_QCOM_IMG_COLOR_FormatYVU420SemiPlanar;
      return OMX_ErrorFormatNotDetected;
    }
    break;
  case QI_YCBCR_SP:
    switch (asub) {
    case QI_H2V2:
      *atarget = OMX_COLOR_FormatYUV420SemiPlanar;
      break;
    case QI_H2V1:
      *atarget = OMX_COLOR_FormatYUV422SemiPlanar;
      break;
    case QI_H1V2:
      *atarget = OMX_QCOM_IMG_COLOR_FormatYUV422SemiPlanar_h1v2;
      break;
    case QI_H1V1:
      *atarget = OMX_QCOM_IMG_COLOR_FormatYUV444SemiPlanar;
      break;
    default:
      *atarget = OMX_COLOR_FormatYUV420SemiPlanar;
      return OMX_ErrorFormatNotDetected;
    }
    break;
    default:
      *atarget = OMX_COLOR_FormatYUV420SemiPlanar;
      return OMX_ErrorFormatNotDetected;
  }
  return OMX_ErrorNone;
}


/*==============================================================================
* Function : translateFormat
* Parameters: a_omxColorFormat, QIFormat aformat, QISubsampling asubsampling
* Return Value : OMX_ERRORTYPE
* Description: This function translates the OMX color format to QIFormat and
* QISubsampling types to be passed to the lower layer.
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::translateFormat(
  OMX_COLOR_FORMATTYPE a_omxColorFormat,
  QIFormat *aformat,
  QISubsampling *asubsampling)
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  int lColorFormat = (int)a_omxColorFormat;

  switch (lColorFormat) {
  case OMX_COLOR_FormatMonochrome:
    *aformat = QI_MONOCHROME;
    *asubsampling = QI_H2V2;
    m_numOfPlanes = 1;
    m_numOfComponents = 1;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU420SemiPlanar:
    *aformat = QI_YCRCB_SP;
    *asubsampling = QI_H2V2;
    m_numOfPlanes = 2;
    m_numOfComponents = 3;
    break;
  case OMX_COLOR_FormatYUV420SemiPlanar:
    *aformat = QI_YCBCR_SP;
    *asubsampling = QI_H2V2;
    m_numOfPlanes = 2;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar:
    *aformat = QI_YCRCB_SP;
    *asubsampling = QI_H2V1;
    m_numOfPlanes = 2;
    m_numOfComponents = 3;
    break;
  case OMX_COLOR_FormatYUV422SemiPlanar:
    *aformat = QI_YCBCR_SP;
    *asubsampling = QI_H2V1;
    m_numOfPlanes = 2;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU422SemiPlanar_h1v2:
    *aformat = QI_YCRCB_SP;
    *asubsampling = QI_H1V2;
    m_numOfPlanes = 2;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYUV422SemiPlanar_h1v2:
    *aformat = QI_YCBCR_SP;
    *asubsampling = QI_H1V2;
    m_numOfPlanes = 2;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU444SemiPlanar:
    *aformat = QI_YCRCB_SP;
    *asubsampling = QI_H1V1;
    m_numOfPlanes = 2;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYUV444SemiPlanar:
    *aformat = QI_YCBCR_SP;
    *asubsampling = QI_H1V1;
    m_numOfPlanes = 2;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU420Planar:
    *aformat = QI_IYUV;
    *asubsampling = QI_H2V2;
    m_numOfPlanes = 3;
    m_numOfComponents = 3;
    break;
  case OMX_COLOR_FormatYUV420Planar:
    *aformat = QI_YUV2;
    *asubsampling = QI_H2V2;
    m_numOfPlanes = 3;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU422Planar:
    *aformat = QI_IYUV;
    *asubsampling = QI_H2V1;
    m_numOfPlanes = 3;
    m_numOfComponents = 3;
    break;
  case OMX_COLOR_FormatYUV422Planar:
    *aformat = QI_YUV2;
    *asubsampling = QI_H2V1;
    m_numOfPlanes = 3;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU422Planar_h1v2:
    *aformat = QI_IYUV;
    *asubsampling = QI_H1V2;
    m_numOfPlanes = 3;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYUV422Planar_h1v2:
    *aformat = QI_YUV2;
    *asubsampling = QI_H1V2;
    m_numOfPlanes = 3;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYVU444Planar:
    *aformat = QI_IYUV;
    *asubsampling = QI_H1V1;
    m_numOfPlanes = 3;
    m_numOfComponents = 3;
    break;
  case OMX_QCOM_IMG_COLOR_FormatYUV444Planar:
    *aformat = QI_YUV2;
    *asubsampling = QI_H1V1;
    m_numOfPlanes = 3;
    m_numOfComponents = 3;
    break;
  default:
    QIDBG_ERROR("%s:%d] Invalid OMX color format %x",
      __func__, __LINE__, lColorFormat);
    return OMX_ErrorBadParameter;
  }
  return lret;
}

/*==============================================================================
* Function : start
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Start encode/decode of the image. This method is called when the
* etb and ftb are done and the component is in the execute state. Dequeue the
* buffers from the etb and ftb queue and call encode/decode.
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::Start()
{
  QIDBG_ERROR("%s:%d] needs to be overridden by derived class", __func__,
    __LINE__);
  return OMX_ErrorNone;
}

/*==============================================================================
* Function : omx_component_deinit
* Parameters: hComp
* Return Value : OMX_ERRORTYPE
* Description: This function will deinit the component.It is a virtual function
* and has to be overridden by the
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::omx_component_deinit(OMX_IN OMX_HANDLETYPE hComp)
{
  if(hComp == NULL) {
    return OMX_ErrorBadParameter;
  }
 return OMX_ErrorNone;

}

/*==============================================================================
* Function : abortExecution
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: Handle the transition to the invalid state
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::abortExecution()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  return lret;
}

/*==============================================================================
* Function : printPortData
* Parameters: None
* Return Value : void
* Description: prints the port information
==============================================================================*/
void QOMXImageCodec::printPortData(OMX_PARAM_PORTDEFINITIONTYPE *aPort)
{
  if (NULL == aPort) {
    QIDBG_ERROR("%s:%d] Port is NULL", __func__, __LINE__);
    return;
  }
  QIDBG_MED("%s:%d] bBuffersContiguous: %d", __func__, __LINE__,
    (int)aPort->bBuffersContiguous);
  QIDBG_MED("%s:%d] bEnabled %d", __func__, __LINE__, (int)aPort->bEnabled);
  QIDBG_MED("%s:%d] bPopulated %d", __func__, __LINE__, (int)aPort->bPopulated);
  QIDBG_MED("%s:%d] eDir: %d", __func__, __LINE__, (int)aPort->eDir);
  QIDBG_MED("%s:%d] eDomain: %d", __func__, __LINE__, (int)aPort->eDomain);
  QIDBG_MED("%s:%d] nBufferAlignment: %d", __func__, __LINE__,
    (int)aPort->nBufferAlignment);
  QIDBG_MED("%s:%d] nBufferCountActual: %d", __func__, __LINE__,
    (int)aPort->nBufferCountActual);
  QIDBG_MED("%s:%d] nBufferCountMin: %d", __func__, __LINE__,
    (int)aPort->nBufferCountMin);
  QIDBG_MED("%s:%d] nBufferSize: %d", __func__, __LINE__,
    (int)aPort->nBufferSize);
  QIDBG_MED("%s:%d] nPortIndex: %d", __func__, __LINE__,
    (int)aPort->nPortIndex);
  QIDBG_MED("%s:%d] nSize: %d", __func__, __LINE__, (int)aPort->nSize);
  QIDBG_MED("%s:%d] nVersion: %d", __func__, __LINE__,
    (int)aPort->nVersion.nVersion);
  QIDBG_MED("%s:%d] format.image.eColorFormat: %d", __func__, __LINE__,
    (int)aPort->format.image.eColorFormat);
  QIDBG_MED("%s:%d] format.image.eCompressionFormat: %d", __func__, __LINE__,
    (int)aPort->format.image.eCompressionFormat);
  QIDBG_MED("%s:%d] format.image.nFrameHeight: %d", __func__, __LINE__,
    (int)aPort->format.image.nFrameHeight);
  QIDBG_MED("%s:%d] format.image.nFrameWidth: %d", __func__, __LINE__,
    (int)aPort->format.image.nFrameWidth);
  QIDBG_MED("%s:%d] format.image.nSliceHeight: %d", __func__, __LINE__,
    (int)aPort->format.image.nSliceHeight);
  QIDBG_MED("%s:%d] format.image.nStride: %d", __func__, __LINE__,
    (int)aPort->format.image.nStride);
}

/*==============================================================================
* Function : preloadCodecLibs
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: preload codec libraries
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::preloadCodecLibs()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  return lret;
}

/*==============================================================================
* Function : releaseCodecLibs
* Parameters: None
* Return Value : OMX_ERRORTYPE
* Description: release codec libraries
==============================================================================*/
OMX_ERRORTYPE QOMXImageCodec::releaseCodecLibs()
{
  OMX_ERRORTYPE lret = OMX_ErrorNone;
  return lret;
}
