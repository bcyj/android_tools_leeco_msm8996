/* =======================================================================
                              MuxQueue.h
DESCRIPTION
  This header defines OMX item Q class.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-mux/main/FileMuxLib/src/MuxQueue.h#2 $
========================================================================== */
#ifndef MUX_CMDQUEUE_H
#define MUX_CMDQUEUE_H

#include "AEEStdDef.h"
#include "MMCriticalSection.h"
#include "filemuxtypes.h"

struct mux_item
{
	mux_item(){};//:m_nId(0), m_bused(false), m_nP1(0),m_nP2(0), m_cb(0){};

  void Set(uint32 stream_number, bool bHeader, uint32 num_samples, const MUX_sample_info_type *sample_info, const uint8  *sample_data, void *ptr)
  {
    m_bused = true;
    m_bheader = bHeader;
    m_stream_number = stream_number;
    m_num_samples = num_samples;
    m_sample_info = sample_info;
    m_sample_data = sample_data;
	m_ptr = ptr;
  }


  bool         m_bused;
  bool         m_bheader;
  uint32       m_stream_number;
  uint32       m_num_samples;
  const MUX_sample_info_type *m_sample_info;
  const uint8  *m_sample_data;
  void *m_ptr;
};

class muxqueue
{
public:
  muxqueue();
  virtual ~muxqueue();

  // push an item into Q
  mux_item* Push(uint32 stream_number,bool bHeader, uint32 num_samples, const MUX_sample_info_type *sample_info, const uint8  *sample_data, void *);
  // pop the oldest item from the Q
  mux_item* Pop_Front();
  //mux_item* Pop_Front(int* id, OMX_HANDLETYPE handle, OMX_PTR buffer);

  int queue_size();

protected:
  VIDEO_HANDLE hMuxQ_CS;
  mux_item*  m_pItem;
  int        m_nSize;
  int        m_nWrite;
  int        m_nRead;
};

#endif //MUX_CMDQUEUE_H

