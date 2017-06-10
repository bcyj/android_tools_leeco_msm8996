#ifndef _EVENT_NOTIFIER_REGISTRY_H_
#define _EVENT_NOTIFIER_REGISTRY_H_

/************************************************************************* */
/**
 *
 * @brief Implements the EventNotifierRegistry for the streamer library
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/EventNotifierRegistry.h#6 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "StreamQueue.h"
#include "qtv_msg.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* Forward declarations for class referenced */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

namespace video {

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

/**
 * struct _EventNotifierCallbackStruct stores the callback information
 */
typedef struct _EventNotifierCallbackStruct
{
  StreamQ_link_type link;

  // privateData: is callback specific. Callback knows how to interpret
  //              it. Caller code does not know about it.
  // eventData:   is the payload associated with the event. It is caller
  //              specific. Callback code must ensure it is casted to the
  //              correct type to extract event data.
  typedef void (*CallbackFunc)(void *privateData, void *eventData);

  CallbackFunc m_CallbackFunc;
  void *m_PrivateData;

} EventNotifierCallbackStruct;

/**
 * class EventNotifier stores all callback registered for an event.
 */
class EventNotifier
{
private:
  /**
   * Internal structure to maintain a list of callbacks for event
   */
  StreamQ_type m_EventHandlerList;

public:
  EventNotifier();
  ~EventNotifier();

  bool AddCallback(EventNotifierCallbackStruct::CallbackFunc cb, void *pvData);
  void DeleteCallback(EventNotifierCallbackStruct::CallbackFunc cb);

  /**
   * eventData is the payload associated with the event. The callback code must
   * ensure that the eventData is casted to the correct type.
   */
  void Notify(void *eventData);

  /**
   * return true if there is at least one callback for eventType
   *
   * @return bool
   */
  bool CallbacksRegistered();

};

/**
 * class EventNotifierRegistry is a generic event registry.
 * It maintains callbacks for 'numEvents' number of events.
 */
class EventNotifierRegistry
{
public:

  EventNotifierRegistry(int& result,
                        int numEvents);

  ~EventNotifierRegistry();

  /**
  * Register callback for an event.
  *
  *
  *  @param cb - Callback function. Address 'cb' is used
  *            toidentify callback for registering and
  *          unregistering with EventNotifierRegistry. This means
  *          the same callback function cannot be used by more
  *          than one thread to register a callback. If this is a
  *          limitation, CompareCallbackAddress() can be modified
  *          to identify queue element by 'cb' and 'pvData'.
 * @param pvData
 * @param eventType
 *
 * @return bool
 */
  bool Register(EventNotifierCallbackStruct::CallbackFunc cb,
                void *pvData,
                int eventType);

  /**
   * Deregister a callback from an event
   */
  bool Deregister(EventNotifierCallbackStruct::CallbackFunc cb,
                  int eventType);

  /**
   * Notify all callbacks registered for event
   * Care should be taken in callback to cast 'eventData' to the correct type.
   */
  bool Notify(void *eventData, int eventType);

  /**
   * Returns true if there is at least one callback registered for event.
   * Returns false otherwise.
   */
  bool CallbacksRegistered(int eventType);

private:
  // Disallow
  EventNotifierRegistry();
  EventNotifierRegistry& operator=(const EventNotifierRegistry &);

  /**
   * Array of EventNotifiers. This is one EventNotifier for
   * each event
   */
  EventNotifier *m_EventHandlerArray;

  /**
   * Size of array m_EventHandlerArray
   */
  int m_NumEvents;

};

};

#endif
