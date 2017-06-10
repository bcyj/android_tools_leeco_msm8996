// -*- Mode: C++ -*-
//======================================================================
// FILE: simple_seektable.h
//
// SERVICES: Audio
//
// DESCRIPTION:
// Defines class simple_seektable that implements...
//
// DOXYGEN:
/// @file simple_seektable.h
///
/// Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
/// All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/SeekTableLib/main/latest/inc/seektable.h#7 $
//$DateTime: 2012/06/04 03:05:53 $
//$Change: 2472042 $

//======================================================================
#ifndef SIMPLE_SEEKTABLE_H
#define SIMPLE_SEEKTABLE_H


//======================================================================
// INCLUDES
//======================================================================
#include <stdio.h>
#include <stdlib.h>
#include "parserdatadef.h"

//======================================================================
// CONSTANTS
//=====================================================================

const uint32 SUCCESS = 0;
const uint32 FAILURE = 1;


//======================================================================
// DATA TYPES
////======================================================================

typedef struct {
  uint32 frame;
  uint64 time;
  uint64 position;
} seek_table_entry;

//======================================================================
// CLASS: simple_seektable
//
// DESCRIPTION:
/// \brief implementation of a simple seek table
///
/// TBD: add description
//======================================================================
class simple_seektable
{
public:

  simple_seektable(uint32 numentries);
  ~simple_seektable();
  uint32 update(uint32 frame, uint64 time, uint64 pos);
  uint32 lookup(uint64 time, uint64* tblposition,
                uint64* tbltime, uint32* tblframe);
  uint32 lookup_last_entry(uint64* tblposition, uint64* tbltime,
                           uint32* tblframe);

private:
  uint64 get_duration();//returns the time from the last entry
  uint32 m_numentries; ///< Number of seek table entries
  uint32 m_nWriteOffset;
  seek_table_entry* m_sseektable; //seek table pointer
}; // end_class

#endif // SIMPLE_SEEKTABLE_H
