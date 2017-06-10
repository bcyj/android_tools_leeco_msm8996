/************************************************************************* */
/**
 *
 * @brief Implements the HTTPSource MMI Extensions Event Handler
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIExtensionEventHandler.cpp#14 $
$DateTime: 2013/07/22 22:38:44 $
$Change: 4142766 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSourceMMIExtensionsEventHandler.cpp
** ======================================================================= */
#include "HTTPSourceMMIExtensionEventHandler.h"
#include "HTTPSourceMMIHelper.h"
#include "OMX_CoreExt.h"

namespace video{

/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */



/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
/*
 * Initialize MMIEventQManager - preceeded before any other call.
 *
 * @param[in] hEventQLock - Reference to event queue lock
 *
 * @return TRUE - Initialization successful, FALSE - Otherwise
 */
template<class T>
bool HTTPSourceMMIExtensionEventHandler::HTTPMMIEventQManager<T>::Initialize
(
 MM_HANDLE hEventQLock
)
{
  m_hEventQLock = hEventQLock;
  Reset();
  return true;
}

/*
 * Reset MMIEventQManager for HTTPProtocolEvent.
 */
template<>
void HTTPSourceMMIExtensionEventHandler::HTTPMMIEventQManager<HTTPSourceMMIExtensionEventHandler
                                 ::HTTPProtocolEventManager::HTTPProtocolEvent>::Reset()
{
  (void)MM_CriticalSection_Enter(m_hEventQLock);
  for (uint32 i=0; i<m_nMaxOutstandingEvents; i++)
  {
    m_eventQ[i].Reset();
  }
  m_nOutstandingEvents = 0;
  m_nEventWriteIndex = 0;
  m_nEventReadIndex = 0;
  (void)MM_CriticalSection_Leave(m_hEventQLock);
}


/*
 * Reset MMIEventQManager for HTTPProtocolHeadersEvent.
 */
template<>
void HTTPSourceMMIExtensionEventHandler::HTTPMMIEventQManager<HTTPSourceMMIExtensionEventHandler
                                 ::HTTPProtocolHeadersEventManager::HTTPProtocolHeadersEvent>::Reset()
{
  (void)MM_CriticalSection_Enter(m_hEventQLock);
  for (uint32 i=0; i<m_nMaxOutstandingEvents; i++)
  {
    m_eventQ[i].Reset();
  }
  m_nOutstandingEvents = 0;
  m_nEventWriteIndex = 0;
  m_nEventReadIndex = 0;
  (void)MM_CriticalSection_Leave(m_hEventQLock);
}


/*
 * Reset MMIEventQManager.
 */
template<class T>
void HTTPSourceMMIExtensionEventHandler::HTTPMMIEventQManager<T>::Reset()
{
  (void)MM_CriticalSection_Enter(m_hEventQLock);
  std_memset(m_eventQ, 0, sizeof(m_eventQ));
  m_nOutstandingEvents = 0;
  m_nEventWriteIndex = 0;
  m_nEventReadIndex = 0;
  (void)MM_CriticalSection_Leave(m_hEventQLock);
}

/*
 * Enqueues a new event in the MMIEventQManager event queue.
 *
 * @param[in] eventToWrite - Event to enqueue
 *
 * @return TRUE - Event successfully queued, FALSE - Otherwise
 */
template<class T>
bool HTTPSourceMMIExtensionEventHandler::HTTPMMIEventQManager<T>::Enqueue
(
 const T& eventToWrite
)
{
  bool bOk = false;

  //Check if the queue is already full, if not push the new event
  //into the queue
  if (IsQFull())
  {
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "MMIEventQManager - MMI event queue full" );
  }
  else
  {
    (void)MM_CriticalSection_Enter(m_hEventQLock);
    if (m_nEventWriteIndex < ARR_SIZE(m_eventQ))
    {
      //Update the write index and number of queued events
      bOk = true;
      m_eventQ[m_nEventWriteIndex] = eventToWrite;
      m_nOutstandingEvents++;
      QTV_MSG_PRIO2( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "MMIEventQManager - Queued event to index %u, "
                     "%u events remain", m_nEventReadIndex, m_nOutstandingEvents );
      m_nEventWriteIndex = (m_nEventWriteIndex + 1) % (uint32)ARR_SIZE(m_eventQ);
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                     "MMIEventQManager - Invalid MMI event queue write index %u",
                     m_nEventWriteIndex );
    }
    (void)MM_CriticalSection_Leave(m_hEventQLock);
  }

  return bOk;
}

/*
 * Peeks an event from the MMIEventQManager event queue.
 *
 * @param[out] eventToRead - Event to peek
 *
 * @return TRUE - Event successfully peeked, FALSE - Otherwise
 */
template<class T>
bool HTTPSourceMMIExtensionEventHandler::HTTPMMIEventQManager<T>::Peek
(
 T& eventToRead
)
{
  bool bOk = false;

  //Check if the queue is already empty, if not get the event from
  //the queue
  if (IsQEmpty())
  {
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "MMIEventQManager - MMI event queue empty" );
  }
  else
  {
    (void)MM_CriticalSection_Enter(m_hEventQLock);
    if (m_nEventReadIndex < ARR_SIZE(m_eventQ))
    {
      bOk = true;
      eventToRead = m_eventQ[m_nEventReadIndex];

      QTV_MSG_PRIO2( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "MMIEventQManager - Peeked event from index %u, "
                     "%u events remain", m_nEventReadIndex, m_nOutstandingEvents );
    }
    (void)MM_CriticalSection_Leave(m_hEventQLock);
  }

  return bOk;
}

/*
 * Dequeues an event from the MMIEventQManager event queue.
 *
 * @param[out] eventToRead - Event to dequeue
 *
 * @return TRUE - Event successfully dequeued, FALSE - Otherwise
 */
template<class T>
bool HTTPSourceMMIExtensionEventHandler::HTTPMMIEventQManager<T>::Dequeue
(
 T& eventToRead
)
{
  bool bOk = false;

  //Check if the queue is already empty, if not get the event from
  //the queue
  if (IsQEmpty())
  {
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "MMIEventQManager - MMI event queue empty" );
  }
  else
  {
    (void)MM_CriticalSection_Enter(m_hEventQLock);
    if (m_nEventReadIndex < ARR_SIZE(m_eventQ))
    {
      //Dequeue the event and update the read index and number of queued events
      bOk = true;
      eventToRead = m_eventQ[m_nEventReadIndex];
      m_nOutstandingEvents--;
      QTV_MSG_PRIO2( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "MMIEventQManager - Dequeued event from index %u, "
                     "%u events remain", m_nEventReadIndex, m_nOutstandingEvents );
      m_nEventReadIndex = (m_nEventReadIndex + 1) % (uint32)ARR_SIZE(m_eventQ);
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                     "MMIEventQManager - Invalid MMI event queue read index %u",
                     m_nEventReadIndex );
    }
    (void)MM_CriticalSection_Leave(m_hEventQLock);
  }
  return bOk;
}

/*
 * Checks if the Event Queue is empty
 *
 * @param T
 *
 * @return bool
 */
template<class T>
bool HTTPSourceMMIExtensionEventHandler::HTTPMMIEventQManager<T>::IsQEmpty() const
{
  bool bEmpty = false;

  //Declare the queue as empty if the number of queued events is 0
  (void)MM_CriticalSection_Enter(m_hEventQLock);
  bEmpty = (m_nOutstandingEvents == 0);
  (void)MM_CriticalSection_Leave(m_hEventQLock);

  return bEmpty;
}

/*
 * Checks if the event Queue is full
 *
 * @param T
 *
 * @return bool
 */
template<class T>
bool HTTPSourceMMIExtensionEventHandler::HTTPMMIEventQManager<T>::IsQFull() const
{
  bool bFull = false;

  //Declare the queue as full if the number of queued events is equal to
  //the queue size
  (void)MM_CriticalSection_Enter(m_hEventQLock);
  bFull = (m_nOutstandingEvents == ARR_SIZE(m_eventQ));
  (void)MM_CriticalSection_Leave(m_hEventQLock);

  return bFull;
}

/**
 * constructor
 *
 */
HTTPSourceMMIExtensionEventHandler::HTTPMMIEventManager::HTTPMMIEventManager()
: m_hHTTPEventMgrLock(NULL),
  m_bNotify(false)
{
}

/**
 *
 * HTTPMMIEventManager Destructor
 *
 */
HTTPSourceMMIExtensionEventHandler::HTTPMMIEventManager::~HTTPMMIEventManager()
{
}

/*
 * Initialize MMIEventManager - preceeded before any other call.
 *
 * @return TRUE - Initialization successful, FALSE - Otherwise
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPMMIEventManager::Initialize
(
  MM_HANDLE hHTTPEventMgrLock
)
{
  m_hHTTPEventMgrLock = hHTTPEventMgrLock;
  return true;
}

/*
 * Set the event notify flag in MMIEventManager.
 *
 * @param[in] bNotify - Event notify flag
 */
void HTTPSourceMMIExtensionEventHandler::HTTPMMIEventManager::SetNotify
(
 const bool bNotify
)
{
  //Set the event notify flag
  (void)MM_CriticalSection_Enter(m_hHTTPEventMgrLock);
  m_bNotify = bNotify;
  (void)MM_CriticalSection_Leave(m_hHTTPEventMgrLock);
}

/*
 * Get the event notify flag from MMIEventManager.
 *
 * @return TRUE - Notify the client, FALSE - Otherwise
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPMMIEventManager::GetNotify() const
{
  bool bNotify = false;

  //Get the event notify flag
  (void)MM_CriticalSection_Enter(m_hHTTPEventMgrLock);
  bNotify = m_bNotify;
  (void)MM_CriticalSection_Leave(m_hHTTPEventMgrLock);

  return bNotify;
}

/*
 *
 * HTTP ProtocolEvent Manager Constructor
 *
 */
HTTPSourceMMIExtensionEventHandler::HTTPProtocolEventManager::HTTPProtocolEventManager()
 :m_httpProtocolEventMgrLock(NULL)
{

}

/*
 * HTTP ProtocolEvent Manager Destructor
 *
 */
HTTPSourceMMIExtensionEventHandler::HTTPProtocolEventManager::~HTTPProtocolEventManager()
{
  if(m_httpProtocolEventMgrLock)
  {
    MM_CriticalSection_Release(m_httpProtocolEventMgrLock);
    m_httpProtocolEventMgrLock  = NULL;
  }
}


/*
 * Initializes HTTP ProtocolEvent Manager
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPProtocolEventManager::Initialize(MM_HANDLE /*hHTTPEventMgrLock*/)
{
  bool bOk = false;
  bOk = (MM_CriticalSection_Create(&m_httpProtocolEventMgrLock) == 0);

  if(bOk && m_httpProtocolEventMgrLock)
  {
    HTTPMMIEventManager::Initialize(m_httpProtocolEventMgrLock);
    bOk = m_eventQManager.Initialize(m_httpProtocolEventMgrLock);
  }
  return bOk;
}


/*
 * Resets HTTP ProtocolEvent Manager
 *
 */
void HTTPSourceMMIExtensionEventHandler::HTTPProtocolEventManager::Reset()
{
  m_eventQManager.Reset();
}

/*
 * Creates a new HTTP Protocol event and enqueues it in HTTPProtocolEventQ
 *
 * @param nserverCode
 * @param pReasonString
 * @param pEntityBody
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPProtocolEventManager::SetHTTPProtocolEvent
(
  HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
  void *protocolInfo,
  void *pClientData
)
{
  bool rsltCode = false;
  HTTPStackNotificationCbData *protocolData = (HTTPStackNotificationCbData*)protocolInfo;

  if (GetNotify())
  {
    HTTPProtocolEvent protocolEvent = { (uint32)protocolData->m_serverCode,
                                          protocolData->m_serverMessage,
                                          protocolData->m_entityBody };

    rsltCode = m_eventQManager.Enqueue(protocolEvent);
    if (rsltCode)
    {
       rsltCode = HTTPSourceMMIExtensionEventHandler::NotifyEvent(pHTTPSourceMMIHelper,
                                                                 OMX_ALL,
                                                          QOMX_HTTP_IndexConfigStreamingProtocolEvent,
                                                                 pClientData);
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "HTTP Protocol Event Queued: serverCode:%d", protocolData->m_serverCode );
    }
  }
  else
  {
    QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_LOW,
                  "HTTPProtocolEventManager: Dropping the Protocol Event with Code:%d",
       protocolData->m_serverCode);
  }
  return rsltCode;
}

/*
 * Dequeues and returns the First HTTPProtcolEvent from the HTTPProtocolEventQ
 *
 * @param errorEvent
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPProtocolEventManager::GetHTTPProtcolEvent
(
  HTTPProtocolEvent& protocolEvent
)
{
  return m_eventQManager.Dequeue(protocolEvent);
}

/**
 * Peek at the first outstanding event without dequeing it.
 *
 * @param protocolEvent  Reference to peeked event.
 *
 * @return bool
 *  true - Event successfully peeked, false - otherwise
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPProtocolEventManager::PeekHTTPProtocolEvent
(
  HTTPProtocolEvent& protocolEvent
)
{
  return m_eventQManager.Peek(protocolEvent);
}

/*
 *
 * HTTP ProtocolHeadersEvent Manager Constructor
 *
 */
HTTPSourceMMIExtensionEventHandler::HTTPProtocolHeadersEventManager::HTTPProtocolHeadersEventManager()
 :m_httpProtocolHeadersEventMgrLock(NULL)
{

}

/*
 * HTTP ProtocolHeadersEvent Manager Destructor
 *
 */
HTTPSourceMMIExtensionEventHandler::HTTPProtocolHeadersEventManager::~HTTPProtocolHeadersEventManager()
{
  if(m_httpProtocolHeadersEventMgrLock)
  {
    MM_CriticalSection_Release(m_httpProtocolHeadersEventMgrLock);
    m_httpProtocolHeadersEventMgrLock  = NULL;
  }
}


/*
 * Initializes HTTP ProtocolHeadersEvent Manager
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPProtocolHeadersEventManager::Initialize(MM_HANDLE /*hHTTPEventMgrLock*/)
{
  bool bOk = false;
  bOk = (MM_CriticalSection_Create(&m_httpProtocolHeadersEventMgrLock) == 0);

  if(bOk && m_httpProtocolHeadersEventMgrLock)
  {
    HTTPMMIEventManager::Initialize(m_httpProtocolHeadersEventMgrLock);
    bOk = m_eventQManager.Initialize(m_httpProtocolHeadersEventMgrLock);
  }
  return bOk;
}


/*
 * Resets HTTP ProtocolHeadersEvent Manager
 *
 */
void HTTPSourceMMIExtensionEventHandler::HTTPProtocolHeadersEventManager::Reset()
{
  m_eventQManager.Reset();
}

/*
 * Creates a new HTTP ProtocolHeaders event and enqueues it in HTTPProtocolHeadersEventQ
 *
 * @param protocolInfo
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPProtocolHeadersEventManager::SetHTTPProtocolHeadersEvent
(
  HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
  void *protocolHeadersEventInfo,
  void *pClientData
)
{
  bool rsltCode = false;
  HTTPStackNotificationCbData *protocolData = (HTTPStackNotificationCbData*)protocolHeadersEventInfo;

  if (GetNotify())
  {
    HTTPProtocolHeadersEvent protocolHeaderEvent = { protocolData->m_msgType,
                                                     protocolData->m_method,
                                                     protocolData->m_protocolHeaders};

    rsltCode = m_eventQManager.Enqueue(protocolHeaderEvent);
    if (rsltCode)
    {
       rsltCode = HTTPSourceMMIExtensionEventHandler::NotifyEvent(pHTTPSourceMMIHelper,
                                                                 OMX_ALL,
                                                                 QOMX_HTTP_IndexConfigStreamingProtocolHeadersEvent,
                                                                 pClientData);
      QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "HTTP ProtocolHeader Event Queued:" );
    }
    else
    {
      if (pHTTPSourceMMIHelper)
      {
        pHTTPSourceMMIHelper->ProcessAuthHandlingDiscarded();
        QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                   "HTTPProtocolHeadersEventManager: Auth handling would be discarded");
      }
    }
  }
  else
  {
    if (pHTTPSourceMMIHelper)
    {
      pHTTPSourceMMIHelper->ProcessAuthHandlingDiscarded();
      QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_LOW,
                  "HTTPProtocolHeadersEventManager: Dropping the Protocol HeadersEvent");
    }
  }
  return rsltCode;
}

/*
 * Dequeues and returns the First HTTPProtcolHeadersEvent from the HTTPProtocolHedersEventQ
 *
 * @param errorEvent
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPProtocolHeadersEventManager::GetHTTPProtocolHeadersEvent
(
  HTTPProtocolHeadersEvent& protocolHeaderEvent
)
{
  return m_eventQManager.Dequeue(protocolHeaderEvent);
}

/**
 * Peek at the first outstanding event without dequeing it.
 *
 * @param protocolHeadersEvent  Reference to peeked event.
 *
 * @return bool
 *  true - Event successfully peeked, false - otherwise
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPProtocolHeadersEventManager::PeekHTTPProtocolHeadersEvent
(
  HTTPProtocolHeadersEvent& protocolHeaderEvent
)
{
  return m_eventQManager.Peek(protocolHeaderEvent);
}



/**
 *  Ctor
 */
HTTPSourceMMIExtensionEventHandler::HTTPBufferingEventManager::HTTPBufferingEventManager()
 :m_httpBufferingEventMgrLock(NULL),
  m_ePreviousBufferingEvent(QOMX_WATERMARK_UNDERRUN)
{

}

/**
 * Dtor
 */
HTTPSourceMMIExtensionEventHandler::HTTPBufferingEventManager::~HTTPBufferingEventManager()
{
  if(m_httpBufferingEventMgrLock)
  {
    MM_CriticalSection_Release(m_httpBufferingEventMgrLock);
    m_httpBufferingEventMgrLock = NULL;
  }
}

/**
 * @brief Initialize HTTP Buffering Event Manager
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPBufferingEventManager::Initialize()
{
  bool bOk;
  bOk = (MM_CriticalSection_Create(&m_httpBufferingEventMgrLock) == 0);
  if (bOk && m_httpBufferingEventMgrLock)
  {
    bOk = m_HTTPBufferingEventQMgr.Initialize(m_httpBufferingEventMgrLock);
    for (int i = 0; i <= QOMX_WATERMARK_NORMAL; i++)
    {
      m_HTTPBufferingEventMgr[i].Initialize(m_httpBufferingEventMgrLock);
    }
  }
  return bOk;
}
void HTTPSourceMMIExtensionEventHandler::HTTPBufferingEventManager::SetCurrentWatermarkType
(
  QOMX_WATERMARKTYPE eCurWaterMark
)
{
  MM_CriticalSection_Enter(m_httpBufferingEventMgrLock);
  m_ePreviousBufferingEvent = eCurWaterMark;
  MM_CriticalSection_Leave(m_httpBufferingEventMgrLock);

}
/**
 * @brief Reset HTTP Buffering Event Manager
 */
void HTTPSourceMMIExtensionEventHandler::HTTPBufferingEventManager::Reset()
{
  SetCurrentWatermarkType(QOMX_WATERMARK_UNDERRUN);
  m_HTTPBufferingEventQMgr.Reset();
}

/**
 * @brief Set MMI layer to send Buffering notification event
 *        back to Client
 *
 * @param eWaterMarkType
 * @param bNotify
 *
 * @return None
 */
void HTTPSourceMMIExtensionEventHandler::HTTPBufferingEventManager::SetBufferingNotifcation
(
  QOMX_WATERMARKTYPE eWaterMarkType,
  const bool bNotify
)
{
  m_HTTPBufferingEventMgr[eWaterMarkType].SetNotify(bNotify);
}

/**
 *
 * @brief Get the buffering notification enabled value for a
 *        particular watermark type
 *
 * @param eWaterMarkType
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPBufferingEventManager::GetBufferingNotification
(
  QOMX_WATERMARKTYPE eWaterMarkType
)
{
  return m_HTTPBufferingEventMgr[eWaterMarkType].GetNotify();
}


/**
 * @brief Process the Buffering Status and enqueu the status
 *         and Notify clinet if required
 *
 * @param pHTTPSourceMMIHelper
 * @param portID
 * @param bDataAvailable
 * @param pClientData
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPBufferingEventManager::ProcessBufferingStatus
(
  HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
  const uint32 portID,
  bool bDataAvailable,
  void *pClientData
)
{
  bool bOk = false;

  QOMX_WATERMARKTYPE eCurrentBufferingEventType =
    (bDataAvailable == true)? QOMX_WATERMARK_NORMAL: QOMX_WATERMARK_UNDERRUN;

  if(eCurrentBufferingEventType != GetCurrentWatermarkType())
  {
    SetCurrentWatermarkType(eCurrentBufferingEventType);

    bool bNotify = m_HTTPBufferingEventMgr[eCurrentBufferingEventType].GetNotify();

    if(bNotify)
    {
      if(pHTTPSourceMMIHelper)
      {
        bOk = NotifyBufferingEvent(pHTTPSourceMMIHelper,portID,pClientData);
        if(eCurrentBufferingEventType == QOMX_WATERMARK_UNDERRUN)
        {
          pHTTPSourceMMIHelper->UpdateRebufCount(1);
        }
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
          "ProcessBufferingStatus: Notify watermark event on port %u as reached watermark type %d",
          portID, (int)eCurrentBufferingEventType);
      }
    }
  }

  return bOk;
}

/**
 * @brief
 *  Notify function to invoke watermark event on portID if
 *  watermark type in http component is the same as the
 *  watermark type registered for.
 *
 * @param pHTTPSourceMMIHelper
 * @param portID
 * @param pClientData
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPBufferingEventManager::NotifyBufferingEvent(
  HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
  const uint32 portID,
  void *pClientData)
{
  return NotifyEvent(
    pHTTPSourceMMIHelper,
    portID,
    QOMX_HTTP_IndexStreamingConfigWaterMarkStatus,
    pClientData);
}

/**
 * @brief
 *  Get the watermark type associated with e the http component
 *  for the port.
 *
 * @return QOMX_WATERMARKTYPE
 */
QOMX_WATERMARKTYPE HTTPSourceMMIExtensionEventHandler::
                   HTTPBufferingEventManager::GetCurrentWatermarkType() const
{
  QOMX_WATERMARKTYPE eCurrentWaterMark;
  MM_CriticalSection_Enter(m_httpBufferingEventMgrLock);
  eCurrentWaterMark = m_ePreviousBufferingEvent;
  MM_CriticalSection_Leave(m_httpBufferingEventMgrLock);
  return eCurrentWaterMark;
}

/**
 * @brief
 *  HTTPQOEEventManager constructor
 *
 */
HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::HTTPQOEEventManager()
{
  m_bQOENotify = false;
  dataPlay = NULL;
  dataPeriodic = NULL;
  dataSwitch = NULL;
  dataStop = NULL;
  QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
    "HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::HTTPQOEEventManager Created");
}

/**
 * @brief
 *  Sets value for qoe notification. Based on which notification will be sent/not sent
 *
 * @param bNotify
 */
void HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::SetQOENotifcation(const bool bNotify)
{
  m_bQOENotify = bNotify;
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::SetQOENotifcation m_bQOENotify = %d",m_bQOENotify);
}

/**
 * @brief
 *  Returns value for qoe notification. "m_bQOENotify"
 *
 * @return m_bQOENotify
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::GetQOENotification()
{
  return m_bQOENotify;
}

/**
 * @brief
 *  Fill QOE data for the event id passed..
 *
 * @param eventID
 * @param dataPtr  (to be populated with data)
 * @param size  (size of data populated)
 *
 * @return None
 */
void HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::
     FillQOEData(const uint32 eventID,void* dataPtr,uint32 &size)
{
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::FillQOEData eventID = %u",eventID);

  switch(eventID)
  {
    case QOMX_HTTP_IndexParamQOEPlay:
      if(!dataPtr)
      {
        size = (uint32)sizeof(QOMX_QOE_DATA_PLAY);
      }else if(dataPlay)
      {
        memcpy(dataPtr,dataPlay,size);
      }else
      {
        size = 0;
      }
      break;
    case QOMX_HTTP_IndexParamQOESwitch:
      if(!dataPtr)
      {
        size = (uint32) sizeof(QOMX_QOE_DATA_SWITCH);
      }else if(dataSwitch)
      {
        memcpy(dataPtr,dataSwitch,size);
      }else
      {
        size = 0;
      }
      break;
    case QOMX_HTTP_IndexParamQOEPeriodic:
      if(!dataPtr)
      {
        size = (dataPeriodic ? dataPeriodic->size : 0);
      }else if(dataPeriodic && (size >= dataPeriodic->size))
      {
        memcpy(dataPtr,dataPeriodic,size);
      }else if(dataPeriodic && (size < dataPeriodic->size))
      {
        size = dataPeriodic->size;
      }else
      {
        size = 0;
      }
      break;
    case QOMX_HTTP_IndexParamQOEStop:
      if(!dataPtr)
      {
        size = (dataStop ? dataStop->size : 0);
      }else if(dataStop && (size >= dataStop->size))
      {
        memcpy(dataPtr,dataStop,size);
      }else if(dataStop && (size < dataStop->size))
      {
        size = dataStop->size;
      }else
      {
        size = 0;
      }
      break;
    default:
      break;
  }
}

/**
 * @brief
 *  Update data structures related to event id passed.for QOE data.
 *
 * @param pHTTPSourceMMIHelper
 * @param eventID
 *
 * @return None
 */
void HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::
     UpdateQOEData(HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
                   const uint32 eventID)
{
  if(m_bQOENotify)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
      "HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::UpdateQOEData eventID = %u",eventID);
    char* pVideoURL = NULL;
    uint32 nBandwidth = 0;
    uint32 nBufCount = 0;
    size_t nURLSize = 0;
    size_t nIPAddrSize = 0;
    size_t    nStopPhraseSize = 0;
    bool   bUpdateRequired = false;
    uint64 currMSeconds = 0;
    (void)MM_Time_GetCurrentTimeInMilliSecsFromEpoch(&currMSeconds);
    switch(eventID)
    {
      case QOMX_HTTP_IndexParamQOEPlay:
        if(!dataPlay)
        {
          dataPlay = (QOMX_QOE_DATA_PLAY *)QTV_Malloc(sizeof(QOMX_QOE_DATA_PLAY));
        }
        if(dataPlay)
        {
          memset(dataPlay,0,sizeof(QOMX_QOE_DATA_PLAY));
          dataPlay->size = (OMX_U32)sizeof(QOMX_QOE_DATA_PLAY);
          dataPlay->timeOfDay = currMSeconds;
        }
        break;
      case QOMX_HTTP_IndexParamQOESwitch:
        {
          if(pHTTPSourceMMIHelper)
          {
            pHTTPSourceMMIHelper->GetQOEData(nBandwidth,nBufCount,NULL,nURLSize,NULL,nIPAddrSize,NULL,nStopPhraseSize);
          }
          if(!dataSwitch)
          {
            dataSwitch = (QOMX_QOE_DATA_SWITCH *)QTV_Malloc(sizeof(QOMX_QOE_DATA_SWITCH));
          }
          if(dataSwitch)
          {
            memset(dataSwitch,0,sizeof(QOMX_QOE_DATA_SWITCH));
            dataSwitch->bandwidth  = nBandwidth;
            dataSwitch->reBufCount = nBufCount;
            dataSwitch->timeOfDay  = currMSeconds;
          }
        }
        break;
      case QOMX_HTTP_IndexParamQOEPeriodic:
        {
          char* pIPAddress = NULL;
          if(pHTTPSourceMMIHelper)
          {
            pHTTPSourceMMIHelper->GetQOEData(nBandwidth,nBufCount,pVideoURL,nURLSize,pIPAddress,nIPAddrSize,NULL,nStopPhraseSize);
            if(pVideoURL == NULL && nURLSize)
            {
              pVideoURL = (char*)QTV_Malloc(nURLSize+1);
              if(pVideoURL)
              {
                bUpdateRequired = true;
              }
            }
            if(pIPAddress == NULL && nIPAddrSize)
            {
              pIPAddress = (char*)QTV_Malloc(nIPAddrSize+1);
              if(pIPAddress)
              {
                bUpdateRequired = true;
              }
            }
            if(bUpdateRequired)
            {
              pHTTPSourceMMIHelper->GetQOEData(nBandwidth,nBufCount,pVideoURL,nURLSize,pIPAddress,nIPAddrSize,NULL,nStopPhraseSize);
              bUpdateRequired = false;
            }
          }
          size_t nSize = sizeof(QOMX_QOE_DATA_PERIODIC)+nURLSize+1+nIPAddrSize+1;
          if(dataPeriodic)
          {
            QTV_Free(dataPeriodic);
          }
          dataPeriodic = (QOMX_QOE_DATA_PERIODIC *)QTV_Malloc(nSize);
          if(dataPeriodic)
          {
            memset(dataPeriodic,0,nSize);
            dataPeriodic->size = (uint32)nSize;
            dataPeriodic->bandwidth = nBandwidth;
            dataPeriodic->timeOfDay = currMSeconds;
            dataPeriodic->nInfoPeriodicLen = (OMX_U32)(nURLSize+1+nIPAddrSize+1);
            if(pIPAddress)
            {
              std_memmove(dataPeriodic->infoPeriodic,pIPAddress,std_strlen(pIPAddress)+1);
              dataPeriodic->nIpAddressLen = (OMX_U32)std_strlen(pIPAddress)+1;
            }
            if(pVideoURL)
            {
              std_memmove(dataPeriodic->infoPeriodic+dataPeriodic->nIpAddressLen,pVideoURL,std_strlen(pVideoURL)+1);
              dataPeriodic->nVideoURLLen =  (OMX_U32)std_strlen(pVideoURL)+1;
            }
          }
          if(pIPAddress)
          {
            QTV_Free(pIPAddress);
            pIPAddress = NULL;
          }
        }
        break;
      case QOMX_HTTP_IndexParamQOEStop:
        {
          char* pStopPhrase = NULL;
          if(pHTTPSourceMMIHelper)
          {
            pHTTPSourceMMIHelper->GetQOEData(nBandwidth,nBufCount,pVideoURL,nURLSize,NULL,nIPAddrSize,pStopPhrase,nStopPhraseSize);
            if(pVideoURL == NULL && nURLSize)
            {
              pVideoURL = (char*)QTV_Malloc(nURLSize+1);
              if(pVideoURL)
              {
                bUpdateRequired = true;
              }
            }
            if(pStopPhrase == NULL && nStopPhraseSize)
            {
              pStopPhrase = (char*)QTV_Malloc(nStopPhraseSize+1);
              if(pStopPhrase)
              {
                bUpdateRequired = true;
              }
            }
            if(bUpdateRequired)
            {
              pHTTPSourceMMIHelper->GetQOEData(nBandwidth,nBufCount,pVideoURL,nURLSize,NULL,nIPAddrSize,pStopPhrase,nStopPhraseSize);
              bUpdateRequired = false;
            }
          }
          size_t nSize = sizeof(QOMX_QOE_DATA_STOP)+nURLSize+1+nStopPhraseSize+1;
          if(dataStop)
          {
            QTV_Free(dataStop);
          }
          dataStop = (QOMX_QOE_DATA_STOP *)QTV_Malloc(nSize);
          if(dataStop)
          {
            memset(dataStop,0,nSize);
            dataStop->size = (OMX_U32)nSize;
            dataStop->bandwidth = (OMX_U32)nBandwidth;
            dataStop->reBufCount = nBufCount;
            dataStop->timeOfDay = currMSeconds;
            dataStop->nInfoStopLen = (OMX_U32)(nURLSize+1+nStopPhraseSize+1);
            if(pStopPhrase)
            {
              std_memmove(dataStop->infoStop,pStopPhrase,std_strlen(pStopPhrase)+1);
              dataStop->nStopPhraseLen = (OMX_U32)(std_strlen(pStopPhrase)+1);
            }

            if(pVideoURL)
            {
              std_memmove(dataStop->infoStop+dataStop->nStopPhraseLen,pVideoURL,std_strlen(pVideoURL)+1);
              dataStop->nVideoURLLen = (OMX_U32)(std_strlen(pVideoURL)+1);
            }
          }
          if(pStopPhrase)
          {
            QTV_Free(pStopPhrase);
            pStopPhrase = NULL;
          }
        }
        break;
      default:
        break;
    }
    if(pVideoURL)
    {
      QTV_Free(pVideoURL);
      pVideoURL = NULL;
    }
  }
}

/**
 * @brief
 *  Notify QOE Event.
 *
 * @param pHTTPSourceMMIHelper
 * @param portID
 * @param eventID
 * @param pClientData
 *
 * @return status
 */
bool HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::
      NotifyQOEEvent(HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
                              const uint32 portID,
                              const uint32 eventID,
                              void *pClientData)
{
  bool status = false;
  if(m_bQOENotify)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::NotifyQOEEvent eventID = %u",eventID);
    status = NotifyEvent(
              pHTTPSourceMMIHelper,
              portID,
              eventID,
              pClientData);
  }
  return status;
}

/**
 * @brief
 *  HTTPQOEEventManager Destructor.
 */
HTTPSourceMMIExtensionEventHandler::HTTPQOEEventManager::~HTTPQOEEventManager()
{
  m_bQOENotify = false;
  if(this->dataPlay)
  {
    QTV_Free(this->dataPlay);
    this->dataPlay = NULL;
  }
  if(this->dataSwitch)
  {
    QTV_Free(this->dataSwitch);
    this->dataSwitch = NULL;
  }
  if(this->dataStop)
  {
    QTV_Free(this->dataStop);
    this->dataStop = NULL;
  }
  if(this->dataPeriodic)
  {
    QTV_Free(this->dataPeriodic);
    this->dataPeriodic = NULL;
  }
}

/**
 * @brief Notify Extension Specific event to IL client
 *
 * @param pHTTPSourceMMIHelper
 * @param nPortID
 * @param eventID
 * @param pClientData
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionEventHandler::NotifyEvent
(
  HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
  const uint32 nPortID,
  const uint32 eventID,
  void *pClientData
)
{
  bool bOk = false;

  if (pHTTPSourceMMIHelper)
  {
    bOk = true;

    //Send the event out to the client
    MMI_ExtSpecificMsgType mmiExtMsg;
    mmiExtMsg.eEvent = (OMX_EVENTTYPE)OMX_EventIndexSettingChanged;
    mmiExtMsg.nData1 = (OMX_U32)nPortID;
    mmiExtMsg.nData2 = (OMX_U32)eventID;
    mmiExtMsg.pEventData = (OMX_PTR)NULL;
    pHTTPSourceMMIHelper->NotifyMmi(MMI_EVT_QOMX_EXT_SPECIFIC,
                             MMI_S_COMPLETE, sizeof(mmiExtMsg),
                             &mmiExtMsg, pClientData);
  }
  return bOk;
}

}
