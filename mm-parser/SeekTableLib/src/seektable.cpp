// -*- Mode: C++ -*-
//======================================================================
// FILE: simple_seektable.cpp
//
// SERVICES: Audio
//
// DESCRIPTION:
// Implementation of class simple_seektable that ....
//
// Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
// All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/SeekTableLib/main/latest/src/seektable.cpp#12 $
//$DateTime: 2013/04/25 02:16:26 $
//$Change: 3668452 $

//======================================================================

//======================================================================
// INCLUDES
//======================================================================
#include "seektable.h"
#include <string.h>
#include "MMMalloc.h"
#include "MMDebugMsg.h"
//======================================================================
// DATA TYPES
//======================================================================


//======================================================================
// CONSTANTS
//======================================================================
//======================================================================
// CLASS METHODS
//======================================================================

//======================================================================
// FUNCTION: constructor
//
// DESCRIPTION:
//
// RETURN: None
//======================================================================
simple_seektable::simple_seektable(uint32 num_entries)
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "simple_seektable num_entries %lu",
               num_entries);
  m_numentries = num_entries;
  //m_frameoffset = DEFAULT_START_OFFSET; // start w/ this offset
  //m_framenumber = 0;
  m_sseektable = NULL;
  m_nWriteOffset = 0;
  //m_needsrefactoring = false;
  if(m_numentries)
  {
    m_sseektable = (seek_table_entry*)
                   MM_Malloc(sizeof(seek_table_entry)*m_numentries);
    if(m_sseektable)
    {
      memset(m_sseektable,0,sizeof(seek_table_entry)*m_numentries);
    }
  }
}
//======================================================================
// FUNCTION: destructor
//
// DESCRIPTION:
//
// RETURN: None
//======================================================================
simple_seektable::~simple_seektable()
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "~simple_seektable num_entries %lu",
               m_numentries);
  if(m_sseektable)
  {
    MM_Free(m_sseektable);
    m_sseektable = NULL;
  }
}
//======================================================================
// FUNCTION: update
//
// DESCRIPTION:
// Update seek table with new frame count/time/file position data.
// RETURN:
// SUCCESS - non zero value, indicating table updated successfully
// FAILURE - 0, failed to update the table
//======================================================================
uint32 simple_seektable::update(uint32 frame,uint64 time,uint64 pos)
{
  uint32 success = 0;
  bool bOK = true;
  if(m_sseektable)
  {
    if(m_nWriteOffset >= m_numentries)
    {
      //need to realloc
      seek_table_entry* ptemp =
        (seek_table_entry*)MM_Realloc(m_sseektable,
                                    (sizeof(seek_table_entry)*2*m_numentries));
      if(ptemp)
      {
        m_sseektable = ptemp;
        memset(m_sseektable + m_numentries, 0,
              (sizeof(seek_table_entry)*m_numentries));
        m_numentries *= 2;
      }
      else
      {
        bOK= false;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                "simple_seektable::update realloc failed..current entries %lu",
                m_numentries);
        success = 0;
      }
    }
    if(bOK)
    {
      //add the entry
      m_sseektable[m_nWriteOffset].frame    = frame;
      m_sseektable[m_nWriteOffset].time     = time;
      m_sseektable[m_nWriteOffset].position = pos;
      m_nWriteOffset++;
      success = 1;
    }
  }
  return success;
}
//======================================================================
// FUNCTION: lookup <time> <out:position>
//
// DESCRIPTION:
// RETURN:
// SUCCESS - non zero value, indicating lookup is successful
// FAILURE - 0, failed indicate lookup failed
//======================================================================
uint32 simple_seektable::lookup(uint64 time, uint64* tblposition,
                                   uint64* tbltime,uint32* tblframe)
{
  uint32 success = 0;
  uint32 idx = 0;
  if(m_sseektable && tbltime && tblframe && tblposition)
  {
    if( (m_nWriteOffset > 0)&& (m_sseektable[m_nWriteOffset-1].time <= time) )
    {
      //Always return the last entry time if requested time >= last entry time
      *tbltime = m_sseektable[m_nWriteOffset-1].time;
      *tblframe = m_sseektable[m_nWriteOffset-1].frame;
      *tblposition = m_sseektable[m_nWriteOffset-1].position;
      success = 1;
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
        "simple_seektable::lookup returning last enrty #Frame %lu TS %llu",
        (*tblframe),(*tbltime));
    }
    else
    {
      uint32 targetindex = 0;
      while( idx < (m_numentries-1) )
      {
        if(m_sseektable[idx].time == time)
        {
          success = 1;
          targetindex = idx;
          break;
        }
        else if(
            (m_sseektable[idx].time < time) &&
            (m_sseektable[idx+1].time > time)
          )
        {
          success = 1;
          targetindex = idx+1;
          break;
        }
        idx++;
      }
      if( (!success) && (idx == m_numentries-1) )
      {
        targetindex = idx;
        success = 1;
      }
      *tblposition = m_sseektable[targetindex].position;
      if (tbltime)
      {
        *tbltime = m_sseektable[targetindex].time;
      }
      if (tblframe)
      {
        *tblframe = m_sseektable[targetindex].frame;
      }
    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                 "simple_seektable::lookup returning #Frame %lu TS %llu",
                 (*tblframe),(*tbltime));
  }
  else
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "simple_seektable::NULL pointers are provided ");

  return success;
}

//======================================================================
// FUNCTION: lookup_last_entry <out:position> <out:time> <out:frame>
//
// DESCRIPTION:
// Returns last table entry
// This is useful when we want to resume table construction from
// a previous interuption such as a seek operation.
//
// RETURN:
// SUCCESS - non zero value, indicating lookup is successful
// FAILURE - 0, failed indicate lookup failed
//======================================================================
uint32 simple_seektable::lookup_last_entry(uint64* tblposition,
                                           uint64* tbltime,
                                           uint32* tblframe)
{
  uint32 success = 0;
  if(m_sseektable && tblposition && tblframe)
  {
    *tblposition = m_sseektable[m_nWriteOffset].position;
    *tbltime     = m_sseektable[m_nWriteOffset].time;
    *tblframe    = m_sseektable[m_nWriteOffset].frame;
    success = 1;
  }
  return success;
}

