/* =======================================================================
                              wfdmmsourcequeue.cpp
DESCRIPTION
  Queue to hold the buffers to process.


  Copyright (c) 2011 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
   $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/src/WFDMMSourceQueue.cpp

========================================================================== */

/* =======================================================================
                             Edit History


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "wfdmmsource_queue.h"
#include "MMDebugMsg.h"

//This value we need to fine tune on the target. On the target definetely we don't want this much.
#define WFD_MM_SOURCE_DEFAULT_EVENTQUEUE_SIZE  200

/* Queue to hold the samples.
 *
*/
/* constructor for wfdmmsourcequeue
 */
wfdmmsourcequeue::wfdmmsourcequeue():m_nSize(0), m_nWrite(0),m_nRead(0)
{
  m_pItem = new wfdmmsource_item[WFD_MM_SOURCE_DEFAULT_EVENTQUEUE_SIZE];
  if(!m_pItem)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "wfdmmsourcequeue: failed to allocate memory.");
  }
  if(MM_CriticalSection_Create(&hwfdmmsourceQ_CS))
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "wfdmmsourcequeue: MM_CriticalSection_Create failed.");
  }
}
/* Destructor for wfdmmsourcequeue
 */
wfdmmsourcequeue::~wfdmmsourcequeue()
{
  if(m_pItem)
  {
    delete [] m_pItem;
    m_pItem = NULL;
    m_nSize = 0;
    m_nWrite = 0;
    m_nRead = 0;
  }

  if(hwfdmmsourceQ_CS)
  {
    MM_CriticalSection_Release(hwfdmmsourceQ_CS);
  }
}

wfdmmsource_item* wfdmmsourcequeue::Push(void * buffer)
{
  wfdmmsource_item* pItem = NULL;
  MM_CriticalSection_Enter(hwfdmmsourceQ_CS);
  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "wfdmmsourcequeue::Push before write Queue size is %d  write index is %i",m_nSize, m_nWrite);
  if(m_nSize  < WFD_MM_SOURCE_DEFAULT_EVENTQUEUE_SIZE)
  {
    pItem = &m_pItem[m_nWrite];
    pItem->Set(buffer);
    m_nWrite++;
    m_nSize ++;
    if(m_nWrite >= WFD_MM_SOURCE_DEFAULT_EVENTQUEUE_SIZE)
    {
      m_nWrite = 0;
    }
  }
  else
  {
    // TODO: queue is full, need to reallocate memory here...
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "wfdmmsourcequeue::event queue is full.");
  }

  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "wfdmmsourcequeue::Push after write Queue size is %d  write index is %i",m_nSize, m_nWrite);
   MM_CriticalSection_Leave(hwfdmmsourceQ_CS);
  return pItem;
}

wfdmmsource_item* wfdmmsourcequeue::Pop_Front()
{
  wfdmmsource_item* pItem = NULL;

  MM_CriticalSection_Enter(hwfdmmsourceQ_CS);

  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "wfdmmsourcequeue::Pop_Front before read Queue size is %d  read index is %i",m_nSize, m_nRead);

  if(m_nSize && (m_nRead < WFD_MM_SOURCE_DEFAULT_EVENTQUEUE_SIZE) && (m_nRead >=0))
  {
    pItem = &m_pItem[m_nRead];

    m_nRead++;
    if(m_nRead >= WFD_MM_SOURCE_DEFAULT_EVENTQUEUE_SIZE)
    {
      m_nRead = 0;
    }

    m_nSize--;
    if(m_nSize < 0)
    {
      m_nSize = 0;
    }
  }
  else if(m_nSize <= 0)
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "wfdmmsourcequeue: event queue is empty, size = %d.", m_nSize);
  }
  else
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wfdmmsourcequeue: m_nRead pos error = %d.", m_nRead);
  }

  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "wfdmmsourcequeue::Pop_Front after read Queue size is %d  read index is %i",m_nSize, m_nRead);

  MM_CriticalSection_Leave(hwfdmmsourceQ_CS);
  return pItem;
}
int wfdmmsourcequeue::queue_size()
{
 return m_nSize;
}
