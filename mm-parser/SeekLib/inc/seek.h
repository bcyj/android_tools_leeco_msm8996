// -*- Mode: C++ -*-
//======================================================================
// FILE: seek.h
//
// SERVICES: Audio
//
// DESCRIPTION:
// Seek lib is used to generate a seek
// table and/or locate a specific playback position within a file
//
// Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
// All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/SeekLib/main/latest/inc/seek.h#9 $
//$DateTime: 2012/06/04 03:05:53 $
//$Change: 2472042 $

//======================================================================
#ifndef SEEK_H
#define SEEK_H

#include "iaudioparser.h"
#include "filebase.h"

//======================================================================
// DATA TYPES
//======================================================================

typedef enum
{
  PREGENERATE_TABLE,//pre-generate seek a table at start of playback
  JUSTINTIME_TABLE, //use a seek table, but don't generate until needed
  NO_TABLE          //never use a seek table, always seek from start
} seek_mode_type;

/*======================================================================
 CONSTANTS
=====================================================================*/
const uint32 SEEK_SUCCESS = 0;
const uint32 SEEK_FAILURE = 1;
//======================================================================
// Forward Declarations
//======================================================================
class simple_seektable;
class seek
{
  public:

   seek(simple_seektable*,IAudioParser*,OSCL_FILE*,uint32,uint64, uint64);
   ~seek();

   /// Get total playback duration
   uint64 get_duration();
   uint64 process_seek(uint64 nReposTime);
   uint32 set_mode(seek_mode_type);

  private:

   uint64 seek_process_frame_data(uint64 file_position);
   simple_seektable* m_cpseektable;//pointer to a seek table implementation
   bool EndOfFrame;
   IAudioParser* m_cpfmtParser;    //Pointer to audio format parser
   uint32 m_frm_hdr_size;     //size of the frame header
   uint64 m_seek_start_offset;//audio data start position
   uint64 m_seek_end_offset;  //audio data end file offset
   OSCL_FILE* m_FmtFilePtr;        //File pointer to use
   seek_mode_type m_emode;         //Seek mode of operaiton
   bool m_update_table;            //true indicates seek table should be updated
   bool m_scan_to_time;            //process to end_time if ture, otherwise process to EOF
   uint64 m_end_time;
   uint64 m_elapsed_time;     //elapsed playback time parsed
   uint64 m_duration;         //total playback duration
   uint32 m_frame_count;      //keep a count for frames parsed for debug
   bool m_scan_ahead_complete;     //indicates when we've parsed the entire file

   /// Track prior frame data. When seeking we stop only once we've passed
   /// the actual frame we want, so we need to save this data
   uint32 m_prior_frame_time;
   uint32 m_prior_frame_size;
};

#endif

