/* =======================================================================
							  MuxQueue.cpp
DESCRIPTION
  Queue to hold the samples to process.This queue is required because we want
  MUX to run in its own thread context.


  Copyright (c) 2011  Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
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
#include "MuxQueue.h"
#include "MMDebugMsg.h"

//This value we need to fine tune on the target. On the target definetely we don't want this much.
#define MUX_DEFAULT_EVENTQUEUE_SIZE  200

/* Queue to hold the samples.
 * This queue is required because we want MUX to run in its own thread context
 * to not to block the encoder.
*/
/* constructor for muxqueue
 */
muxqueue::muxqueue():m_nSize(0), m_nWrite(0),m_nRead(0)
{
  m_pItem = new mux_item[MUX_DEFAULT_EVENTQUEUE_SIZE];
  if(!m_pItem)
  {
	MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "muxqueue: failed to allocate memory.");
  }
  if(MM_CriticalSection_Create(&hMuxQ_CS))
  {
	MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "muxqueue: MM_CriticalSection_Create failed.");
  }
}
/* Destructor for muxqueue
 */
muxqueue::~muxqueue()
{
  if(m_pItem)
  {
	delete [] m_pItem;
	m_pItem = NULL;
	m_nSize = 0;
	m_nWrite = 0;
	m_nRead = 0;
  }

  if(hMuxQ_CS)
  {
	MM_CriticalSection_Release(hMuxQ_CS);
  }
}

mux_item* muxqueue::Push(uint32 stream_number, bool bHeader, uint32 num_samples, const MUX_sample_info_type *sample_info, const uint8  *sample_data, void * ptr)
{
  mux_item* pItem = NULL;
  MM_CriticalSection_Enter(hMuxQ_CS);
  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "muxqueue::Push before write Queue size is %d  write index is %i",m_nSize, m_nWrite);
  if(m_pItem)
  {
  if(m_nSize  < MUX_DEFAULT_EVENTQUEUE_SIZE)
  {
	pItem = &m_pItem[m_nWrite];
	  if( pItem )
	  {
	pItem->Set(stream_number, bHeader, num_samples, sample_info, sample_data, ptr);
	m_nWrite++;
	m_nSize ++;
	if(m_nWrite >= MUX_DEFAULT_EVENTQUEUE_SIZE)
	{
	  m_nWrite = 0;
	}
  }
  else
  {
		MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "muxqueue::pItem is NULL.");
	  }
	}
	else
	{
	// TODO: queue is full, need to reallocate memory here...
	MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "muxqueue::event queue is full.");
  }
  }
  else
  {
	 MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "muxqueue::event queue not created");
  }

  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "muxqueue::Push after write Queue size is %d  write index is %i",m_nSize, m_nWrite);
  MM_CriticalSection_Leave(hMuxQ_CS);
  return pItem;
}

mux_item* muxqueue::Pop_Front()
{
  mux_item* pItem = NULL;

  MM_CriticalSection_Enter(hMuxQ_CS);

  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "muxqueue::Pop_Front before read Queue size is %d  read index is %i",m_nSize, m_nRead);

  if(m_nSize && (m_nRead < MUX_DEFAULT_EVENTQUEUE_SIZE) && (m_nRead >=0))
  {
	pItem = &m_pItem[m_nRead];

	m_nRead++;
	if(m_nRead >= MUX_DEFAULT_EVENTQUEUE_SIZE)
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
	MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "muxqueue: event queue is empty, size = %d.", m_nSize);
  }
  else
  {
	MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "muxqueue: m_nRead pos error = %d.", m_nRead);
  }

  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "muxqueue::Pop_Front after read Queue size is %d  read index is %i",m_nSize, m_nRead);

  MM_CriticalSection_Leave(hMuxQ_CS);
  return pItem;
}
int muxqueue::queue_size()
{
 return m_nSize;
}
