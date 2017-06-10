/* =======================================================================
                              wfdmmsource_queue.h
DESCRIPTION
  This header defines the wfd source item class.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
   $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/inc/WFDMMSourceQueue.h
 
========================================================================== */
#ifndef WFD_MM_SOURCE_CMDQUEUE_H
#define WFD_MM_SOURCE_CMDQUEUE_H

#include "AEEStdDef.h" 
#include "MMCriticalSection.h"

struct wfdmmsource_item
{
	wfdmmsource_item(){};

  void Set(void* pBuffer)
  {
    m_bused = true;
    m_pBuffer = pBuffer;	
  }
  bool         m_bused;
  void*        m_pBuffer;
};

class wfdmmsourcequeue
{
public:
  wfdmmsourcequeue();
  virtual ~wfdmmsourcequeue();

  // push an item into Q
  wfdmmsource_item* Push(void *);
  // pop the oldest item from the Q
  wfdmmsource_item* Pop_Front();
  int queue_size();

protected:
  MM_HANDLE hwfdmmsourceQ_CS;
  wfdmmsource_item*  m_pItem;
  int        m_nSize;
  int        m_nWrite;
  int        m_nRead;
};

#endif //WFD_MM_SOURCE_CMDQUEUE_H

