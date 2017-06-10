/************************************************************************* */
/**
 * EventNotifierRegistry.cpp
 * @brief implementation of the EventNotifierRegistry helper classes.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/EventNotifierRegistry.cpp#11 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "EventNotifierRegistry.h"
#include "qtv_msg.h"
#include "SourceMemDebug.h"

namespace video {

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
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
**                        Macro Definitions
** ======================================================================= */

/**
 * Comparison function needed by queue.h,cpp routines to search for element
 * in queue.
 */
static int CompareCallbackAddress(void *itemPtr, void *compareVal)
{
  int result = 0;

  EventNotifierCallbackStruct *node = (EventNotifierCallbackStruct *)itemPtr;

  if (node)
  {
    result =  ((void *)node->m_CallbackFunc == compareVal ? 1 : 0);
  }

  return result;
}

/**
 * Constructor
 */
EventNotifier::EventNotifier()
{
  StreamQ_init(&m_EventHandlerList);
}

/**
 * Destructor
 */
EventNotifier::~EventNotifier()
{
  // Clean up in case there are callbacks that were not deregistered
  while (StreamQ_cnt(&m_EventHandlerList) > 0)
  {
    EventNotifierCallbackStruct *cbStruct =
      (EventNotifierCallbackStruct *) StreamQ_get(&m_EventHandlerList);

    if (cbStruct)
    {
      QTV_Delete(cbStruct);
    }
  }
}

/** @brief Register the callback with event if currently not registered
  *
  * @param[in] cb     - The callback function to be invoked
  * @param[in] pvData - Pointer to private data for callback
  *
  * @result true if callback successfully registered with event
  *         false otherwise
  */
bool EventNotifier::AddCallback(EventNotifierCallbackStruct::CallbackFunc cb, void *pvData)
{
  bool result = true;

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                "EventNotifier::AddCallback: Adding callback '%p'",
                cb);

  EventNotifierCallbackStruct *cbStruct =
    (EventNotifierCallbackStruct *)StreamQ_linear_search(&m_EventHandlerList,
                                                    CompareCallbackAddress,
                                                    (void*)cb);

  if (cbStruct)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                  "EventNotifier::AddCallback: Callback '%p' already registered. "
                  "Nothing to do", cb);
  }
  else
  {
    cbStruct = QTV_New(EventNotifierCallbackStruct);

    if (NULL == cbStruct)
    {
      result = false;

      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "EventNotifier::AddCallback: Failed to allocated cbStruct");
    }
    else
    {
      StreamQ_link(cbStruct, &(cbStruct->link));
      cbStruct->m_CallbackFunc = cb;
      cbStruct->m_PrivateData = pvData;
      StreamQ_put(&m_EventHandlerList, &(cbStruct->link));

      QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_ERROR,
                   "EventNotifier::AddCallback: Callback added");
    }
  }

  return result;
}

/** @brief Delete the callback identified by 'pvData' from the set of
  * registered callbacks if it exists.
  *
  * @param[in] pvData - Pointer to private data associated with callback
  */
void EventNotifier::DeleteCallback(EventNotifierCallbackStruct::CallbackFunc cb)
{
  EventNotifierCallbackStruct *cbStruct =
    (EventNotifierCallbackStruct *)StreamQ_linear_search(&m_EventHandlerList,
                                                   CompareCallbackAddress,
                                                   (void *)cb);

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                "EventNotifier::DeleteCallback: Deleting callback '%p'",
                 cb);

  if (cbStruct)
  {
    StreamQ_delete(&(cbStruct->link));
    QTV_Delete(cbStruct);

    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                 "EventNotifier::DeleteCallback: Callback Deregistered"
                 "Nothing to do");
  }
  else
  {
    // Callback was not registered for event
    QTV_MSG_PRIO(QTVDIAG_HTTP_STACK, QTVDIAG_PRIO_MED,
                 "EventNotifier::DeleteCallback: Callback already unregistered. "
                 "Nothing to do");
  }
}

/**
 * @brief Invoke all callbacks registered for event
 *
 * @param[in] eventData - Event data that is passed to callback to
 *            interpret. Callback must cast it to correct type.
 */
void EventNotifier::Notify(void *eventData)
{
  // Get head of queue
  EventNotifierCallbackStruct *cbStruct =
    (EventNotifierCallbackStruct *)StreamQ_check(&m_EventHandlerList);

  while (cbStruct)
  {
    // Invoke the callbacks
    cbStruct->m_CallbackFunc(cbStruct->m_PrivateData, eventData);
    cbStruct = (EventNotifierCallbackStruct *)StreamQ_next(&m_EventHandlerList, &(cbStruct->link));
  }
}

/**
 * @brief Checks if there is least one callback is registered for event
 *
 * @result - true if at least one callback registered for event
 *         - false otherwise.
 */
bool EventNotifier::CallbacksRegistered()
{
  return StreamQ_cnt(&m_EventHandlerList) > 0 ? true : false;
}


/**
 * Notify all callbacks registered for event
 * Care should be taken in callback to cast 'eventData' to the correct type.
 */
bool
EventNotifierRegistry::Notify(void *eventData, int eventType)
{
  bool result = true;

   if ((eventType < 0) || (eventType >= m_NumEvents))
  {
    result = false;

    QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "EventNotifierRegistry::Notify: Failed. eventType '%d' "
                  "invalid. Should between 0 and '%d' inclusive",
                  eventType, m_NumEvents);
  }
  else
  {
    m_EventHandlerArray[eventType].Notify(eventData);
  }

  return result;
}

////////////////////////////////////////////////////////////////
//          Function definitions for EventNotifierRegistry
////////////////////////////////////////////////////////////////

/**
 * constructor
 */
EventNotifierRegistry::EventNotifierRegistry(
  int& result, int numEvents) :
    m_EventHandlerArray(NULL),
    m_NumEvents(numEvents)
{
  result = 1;

  if (numEvents > 0)
  {
    m_EventHandlerArray = QTV_New_Array(EventNotifier, m_NumEvents);

    if (NULL != m_EventHandlerArray)
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED,
                    "EventNotifierRegistry::ctor: Created event registry "
                    "with '%d' events", m_NumEvents);
      result = 0;
    }
  }

}

/**
 * Destructor
 */
EventNotifierRegistry::~EventNotifierRegistry()
{
  QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED,
    "EventNotifierRegistry::dtor");

  if (NULL != m_EventHandlerArray)
  {
    QTV_Delete_Array(m_EventHandlerArray);
    m_EventHandlerArray = NULL;
  }
}

/**
 * Register callback for an event.
 *
 * @param cb - Callback function. Address 'cb' is used to
 *           identify callback for registering and unregistering
 *           with EventNotifierRegistry. This means the same
 *           callback function cannot be used by more than one
 *           thread to register a callback. If this is a
 *           limitation, CompareCallbackAddress() can be
 *           modified to identify queue element by 'cb' and
 *           'pvData'.
 * @param pvData
 * @param eventType
 *
 * @return bool
 */
bool
EventNotifierRegistry::Register(
  EventNotifierCallbackStruct::CallbackFunc cb,
  void *pvData,
  int eventType)
{
  bool result = false;

  if ((eventType < 0) || (eventType >= m_NumEvents))
  {
    result = false;

    QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "EventNotifierRegistry::Register: Failed. eventType '%d' invalid. "
                  " Should between 0 and '%d' inclusive", eventType, m_NumEvents);
  }
  else
  {
    result = m_EventHandlerArray[eventType].AddCallback(cb, pvData);
  }

  return result;
}

/**
 * Deregister a callback from an event
 *
 * @param pvData
 * @param eventType
 *
 * @return bool
 */
bool
EventNotifierRegistry::Deregister(EventNotifierCallbackStruct::CallbackFunc cb,
                                  int eventType)
{
  bool result = true;
  if ((eventType < 0) || (eventType >= m_NumEvents))
  {
    result = false;

    QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "EventNotifierRegistry::Deregister:  Failed. eventType '%d' invalid. "
                  "Should between 0 and '%d' inclusive", eventType, m_NumEvents);
  }
  else
  {
    m_EventHandlerArray[eventType].DeleteCallback(cb);
  }

  return result;
}

/**
 * Returns true if there is at least one callback registered for event.
 * Returns false otherwise.
 */
bool
EventNotifierRegistry::CallbacksRegistered(int eventType)
{
  bool result = true;

  if ((eventType < 0) || (eventType >= m_NumEvents))
  {
    result = false;

    QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "EventNotifierRegistry::CallbacksRegistered: Failed. eventType '%d' invalid. "
      "Should between 0 and '%d' inclusive",
      eventType, m_NumEvents);
  }
  else
  {
    result = m_EventHandlerArray[eventType].CallbacksRegistered();
  }

  return result;
}

};
