#ifndef HTTPSOURCEMMIEXTENSIONEVENTHANDLER_H
#define HTTPSOURCEMMIEXTENSIONEVENTHANDLER_H
/************************************************************************* */
/**
 * HTTPSourceMMIExtensionEventHandler.h
 * @brief Defines the HTTPSource MMI Extensions Event Handler
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIExtensionEventHandler.h#10 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "MMCriticalSection.h"
#include "oscl_string.h"
#include "QOMX_StreamingExtensions.h"
#include "HTTPSourceMMITrackHandler.h"



namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
class HTTPSourceMMIHelper;

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

/**
* @brief   HTTPSourceMMIExtensionEventHandler
*
* @detail This module handles OEM extension Events for the Qualcomm HTTP Source .
*/
class HTTPSourceMMIExtensionEventHandler
{
public:
  HTTPSourceMMIExtensionEventHandler(){ };
  virtual ~HTTPSourceMMIExtensionEventHandler(){ };

  //MMI event queue manager - manages a simple generic circular event queue.
  //Needed to hold on to the asynchronous events after notification to client
  //is sent out and before the client queries for the event
  template <class T>
  class HTTPMMIEventQManager
  {
  public:
    HTTPMMIEventQManager() : m_hEventQLock(NULL){ };
    virtual ~HTTPMMIEventQManager(){ };

    bool Initialize(MM_HANDLE hEventQLock);
    void Reset();
    bool Enqueue(const T& eventToWrite);
    bool Peek(T& eventToRead);
    bool Dequeue(T& eventToRead);

    //Max number of outstanding events waiting to be read
    static const uint32 m_nMaxOutstandingEvents = 0x0000000A;

  private:
    bool IsQEmpty() const;
    bool IsQFull() const;

    //Event queue lock (to protect concurrent access of certain data
    //members like m_eventQ from streamer thread and OMX thread contexts)
    MM_HANDLE m_hEventQLock;

    //Buffering event queue
    T m_eventQ[m_nMaxOutstandingEvents];

    //Number of outstanding events
    uint32 m_nOutstandingEvents;

    //Event write and read indices (rear and front if you will!!)
    uint32 m_nEventWriteIndex;
    uint32 m_nEventReadIndex;
  };

  //Abstract MMI event manager
  class HTTPMMIEventManager
  {
  public:
    HTTPMMIEventManager();
    virtual ~HTTPMMIEventManager();

    virtual bool Initialize(MM_HANDLE hHTTPEventMgrLock);
    virtual void SetNotify(const bool bNotify);
    virtual bool GetNotify() const;

  protected:
    //Event manager lock (to protect concurrent access of certain data
    //members like m_bNotify from streamer thread and OMX thread contexts)
    MM_HANDLE m_hHTTPEventMgrLock;

  private:
    //Event notification control
    bool m_bNotify;
  };

  //Protocol error event manager
  class HTTPProtocolEventManager : public HTTPMMIEventManager
  {
  public:
    HTTPProtocolEventManager();
    virtual ~HTTPProtocolEventManager();

     //Protocol error event
    class HTTPProtocolEvent
    {
     public:
      uint32 nStatusCode;
      OSCL_STRING reasonPhrase;
      OSCL_STRING entityBody;

      void Reset(){nStatusCode = 0;};
    };

    virtual bool Initialize(MM_HANDLE hHTTPEventMgrLock = NULL);
    virtual void Reset();
    bool SetHTTPProtocolEvent(HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
                              void *protocolInfo,
                              void *pClientData);

    bool GetHTTPProtcolEvent(HTTPProtocolEvent& protocolEvent);
    bool PeekHTTPProtocolEvent(HTTPProtocolEvent& protocolEvent);
  private:
    //Protocol error event queue manager
    HTTPMMIEventQManager<HTTPProtocolEvent> m_eventQManager;
    MM_HANDLE m_httpProtocolEventMgrLock;
  };

  // ProtocolHeaders Event Manager.
  class HTTPProtocolHeadersEventManager : public HTTPMMIEventManager
  {
  public:
    HTTPProtocolHeadersEventManager();
    virtual ~HTTPProtocolHeadersEventManager();

    //Protocol headers event
    struct HTTPProtocolHeadersEvent
    {
      bool msgType;               // false for request, true for response
      OSCL_STRING messageClass;
      OSCL_STRING protocolHeaders;
      void Reset() { msgType = true; }
    };

    virtual bool Initialize(MM_HANDLE hEventLock=NULL);
    virtual void Reset();
    bool SetHTTPProtocolHeadersEvent(HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
                                     void *protocolInfo,
                                     void *pClientData);
    bool PeekHTTPProtocolHeadersEvent(HTTPProtocolHeadersEvent& protocolHeadersEvent);
    bool GetHTTPProtocolHeadersEvent(HTTPProtocolHeadersEvent& protocolHeadersEvent);

  private:
    //Protocol error event queue manager
    HTTPMMIEventQManager<HTTPProtocolHeadersEvent> m_eventQManager;
    //Event manager lock (to protect concurrent access of certain data)
    MM_HANDLE m_httpProtocolHeadersEventMgrLock;
  };

  class HTTPBufferingEventManager
  {
  public:
      HTTPBufferingEventManager() ;
      virtual ~HTTPBufferingEventManager();
      typedef struct
      {
        QOMX_WATERMARKTYPE eWaterMarkType;
        OMX_U32 nLevel;
        QOMX_WATERMARKUNITSTYPE eUnitsType;
      }HTTPBufferingEvent;

      virtual bool Initialize();
      virtual void Reset();

      void SetBufferingNotifcation(QOMX_WATERMARKTYPE eWaterMarkType, const bool bNotify);
      bool GetBufferingNotification(QOMX_WATERMARKTYPE eWaterMarkType);
      bool ProcessBufferingStatus(HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
                                  const uint32 portID,
                                  bool bDataAvailable,
                                  void *pClientData);
    bool NotifyBufferingEvent(HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
                              const uint32 portID,
                              void *pClientData);

    QOMX_WATERMARKTYPE GetCurrentWatermarkType() const;
    void SetCurrentWatermarkType(const QOMX_WATERMARKTYPE);


  private:

    // Store previous buffering status to stop sending duplicate events to IL
    MM_HANDLE m_httpBufferingEventMgrLock;
    QOMX_WATERMARKTYPE m_ePreviousBufferingEvent;


    HTTPMMIEventQManager <HTTPBufferingEvent> m_HTTPBufferingEventQMgr;
    HTTPMMIEventManager m_HTTPBufferingEventMgr[QOMX_WATERMARK_NORMAL+1];

  };
  class HTTPQOEEventManager{
  public:
      HTTPQOEEventManager() ;
      virtual ~HTTPQOEEventManager();
      void SetQOENotifcation(const bool bNotify);
      bool GetQOENotification();
      void FillQOEData(const uint32 eventID,
                       void* dataPtr,
                       uint32& size);
      void UpdateQOEData(HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
                         const uint32 eventID);
      bool NotifyQOEEvent(HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
                              const uint32 portID,
                              const uint32 eventID,
                              void *pClientData);
  private:
      bool m_bQOENotify;
      QOMX_QOE_DATA_PLAY     *dataPlay;
      QOMX_QOE_DATA_SWITCH   *dataSwitch;
      QOMX_QOE_DATA_PERIODIC *dataPeriodic;
      QOMX_QOE_DATA_STOP     *dataStop;
  };
  HTTPProtocolEventManager m_HTTPProtocolEventMgr;
  HTTPProtocolHeadersEventManager m_HTTPProtocolHeadersEventMgr;
  HTTPBufferingEventManager m_HTTPBufferingEventMgr[MMI_HTTP_NUM_MAX_PORTS +1];
  HTTPQOEEventManager m_HTTPQOEEventMgr;
private:

 static bool NotifyEvent(HTTPSourceMMIHelper *pHTTPSourceMMIHelper,
                         const uint32 nPortID,
                         const uint32 eventID,
                         void *pClientData  );

};

}
#endif /* HTTPSOURCEMMIEXTENSIONEVENTHANDLER_H*/
