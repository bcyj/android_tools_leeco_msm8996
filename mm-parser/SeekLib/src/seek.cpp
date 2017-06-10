/* =======================================================================
                              seek.cpp
DESCRIPTION
  Seekthread implements a scan ahead thread that parses a file to
  generate a seek table or location a specific playback position within
  a file.

Copyright 2009-2013 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/SeekLib/main/latest/src/seek.cpp#21 $
$DateTime: 2013/04/09 02:46:31 $
$Change: 3590211 $
========================================================================== */
#define SEEK_CPP

#include "seektable.h"
#include "seek.h"

//======================================================================
// FUNCTION: constructor
//
// DESCRIPTION:
//
// RETURN: None
//======================================================================
seek::seek(simple_seektable* ptrtoseektable, IAudioParser* pfmtparser,
           OSCL_FILE *FilePtr, uint32 fmt_frm_hdr_size,
           uint64 seek_start_offset,uint64 seek_end_offset)
{
   m_cpfmtParser = pfmtparser;
   m_cpseektable = ptrtoseektable;
   m_FmtFilePtr = FilePtr;
   m_frm_hdr_size = fmt_frm_hdr_size;
   m_seek_start_offset = seek_start_offset;
   m_seek_end_offset = seek_end_offset;
   m_emode = NO_TABLE; //default to no seek table,until one is set
   m_update_table = false;  // default mode is NO_TABLE
   m_scan_to_time = true; // init to true (doesn't really matter)
   m_end_time = 0; // no m_end_time set
   m_frame_count = 1; // init our frame count to start a 1
   m_elapsed_time = 0; // init elapsed playback time
   m_duration = 0; // init m_duration to 0 since its unknown at this time
   m_scan_ahead_complete = false; // haven't event started yet...
   EndOfFrame = false;
   m_prior_frame_size = 0;
   m_prior_frame_time = 0;
}
//======================================================================
// FUNCTION: destructor
//
// DESCRIPTION:
//
// RETURN: None
//======================================================================
seek::~seek(){}
//======================================================================
// FUNCTION: get_m_duration <>
//
// DESCRIPTION:
// This function process the file data and returns the total
//
// RETURN:
//   Total duration of the file
//======================================================================
//
uint64 seek::get_duration()
{
  uint64  file_duration = 0;
  if(m_cpfmtParser && m_cpseektable && m_FmtFilePtr)
  {
    if(!m_scan_ahead_complete)
    {
      m_scan_to_time = false;
      m_duration = seek_process_frame_data(m_seek_start_offset);
    }
  }
  file_duration = m_duration;
  m_scan_ahead_complete = true;
  m_scan_to_time = true;
  return file_duration;
}

//======================================================================
// FUNCTION: seek_process_frame_data <uint32 file_position>
//
// DESCRIPTION:
// This function Process the file data and updates the seek table
// Makes updates to the associated seektable when [m_scan_ahead_complete] is false.
// Stops parsing when elapase playback time exceeds <m_end_time> and resets the
// file position.
// if <m_scan_to_time> is false, no end time is specified and parsing continues
// to EOF, file position is not reset, but m_duration is updated.
//
// RETURN:
//   New Reposition Time
//======================================================================
//
uint64 seek::seek_process_frame_data(uint64 file_position)
{
   uint8 uBuf[20] = {0};
   uint32 uframesize = 0;
   uint32 uframetime = 0;
   bool bOK = false;
   uint32 ulFrameCorruptCount = 0;

   if(m_cpfmtParser && m_cpseektable && m_FmtFilePtr)
   {
     bOK = true;
   }
   // For the seek on file EndOfFrame should not be true
   if(file_position && bOK)
   {
     EndOfFrame = false;
   }
   if( (m_scan_to_time) && (m_end_time < m_elapsed_time) && (bOK) )
   {
     m_frame_count--;
     m_elapsed_time -=uframetime;
     file_position -=uframesize;
     m_cpfmtParser->set_newfile_position(file_position);
   }
   else if( (bOK) && (!m_scan_to_time) )
   {
     while( (!EndOfFrame) && (file_position < m_seek_end_offset) )
     {
       if(!OSCL_FileSeekRead(uBuf,m_frm_hdr_size,1,m_FmtFilePtr,file_position,SEEK_SET))
       {
         EndOfFrame = true;
         m_duration = m_elapsed_time;
         m_scan_ahead_complete = true;
       }
       else
       {
         if(IAUDIO_SUCCESS!=
            m_cpfmtParser->parse_frame_header(uBuf,&uframesize, &uframetime))
         {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
               "parse_frame_header skipping byte @ offset %llu",file_position);
         }
         if(uframesize == 0)
         {
           file_position++;
#ifdef FEATURE_FILESOURCE_SEEK_LIB_DEBUG
           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
              "uframesize is 0,incrementing fileoffset++ %llu",file_position);
#endif
           ulFrameCorruptCount++;
           if (ulFrameCorruptCount > 1024)
           {
             break;
           }
           continue;
         }
         ulFrameCorruptCount = 0;
         if(!m_scan_ahead_complete)
         {
           //SeekTable::Update returns 0 when update fails...
           if(m_cpseektable->update(m_frame_count,m_elapsed_time,file_position)==0)
           {
             break;
           }
         }
         m_frame_count++;
         m_elapsed_time +=uframetime;
         file_position +=uframesize;
       }
     }//while(!EndOfFrame)
   }//else if(bOK)
   if(bOK)
   {
#ifdef FEATURE_FILESOURCE_SEEK_LIB_DEBUG
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"m_emode = PREGENERATE_TABLE");
#endif
     m_emode = PREGENERATE_TABLE;
     m_duration = m_elapsed_time;
     m_scan_ahead_complete = true;
   }
   return m_elapsed_time;
}

//======================================================================
// FUNCTION: process_seek <SEEK_FILE_OFFSET nReposTime>
//
// DESCRIPTION:
// Process the seek operation for the corresponding reposition time.
//
// RETURN:
//   New Reposition Time
//======================================================================
uint64 seek::process_seek(uint64 nReposTime)
{
  uint64 status = 0;
  m_scan_to_time = true;
  uint64 nTimeReturn = 0;
  uint64 nFilePosn = 0;
  uint32 nTablePosn = 0;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"seek::process_seek nReposTime %llu",
               nReposTime);

  // Determine where we want our seek to start from, utilizing seek table
  // if it exists.
  if (NO_TABLE == m_emode)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "seek::process_seek NO_TABLE == m_emode");
    if(nReposTime == 0)
    {
      nFilePosn = m_seek_start_offset;
      nTimeReturn = 0;
      status = 1;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "seek::process_seek NO_TABLE == m_emode,seeking to 0");
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                 "seek::process_seek NO_TABLE == m_emode, seek will be done \
                 to the begining of the file....");
    }
  }
  else
  {
    // seek table exists...
    if (m_scan_to_time)
    {
      status = m_cpseektable->lookup(nReposTime, &nFilePosn, &nTimeReturn,
                                     &nTablePosn);
      MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_HIGH,
        "seek::process_seek returned from lookup status %llu nReposTime %llu\
        nFilePosn %llu nTimeReturn %llu nTablePosn %lu",
        status,nReposTime,nFilePosn,nTimeReturn,nTablePosn);
    }
    else
    {
       status = m_cpseektable->lookup_last_entry(&nFilePosn, &nTimeReturn,
                                                 &nTablePosn);
       MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_HIGH,
        "seek::process_seek returned from lookup_last_entry status %llu\
        nFilePosn %llu nTimeReturn %llu nTablePosn %lu",
        status,nFilePosn,nTimeReturn,nTablePosn);
    }
  }
  if(status)
  {
    //Non zero status means seek is successful using pregenerated seek table,
    //seek the file pointer to correct file offset to complete the repositioning
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"seek::process_seek seek successful!!");
    if(OSCL_FileSeek(m_FmtFilePtr, nFilePosn, SEEK_SET))
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"seek::process_seek seek Failed");
      return nTimeReturn;
    }
    m_cpfmtParser->set_newfile_position(nFilePosn);
    return nTimeReturn;
  }
  return nTimeReturn;
}
//======================================================================
// FUNCTION: set_mode <mode>
//
// DESCRIPTION:
// Set the mod of operation.
// Support for
// dynamic mode change may be added in the future, but is not
// supported in this release.
//
// @param mode - Seek mode of operation, one of the following:
//   - PREGENERATE_TABLE: a seek table is pregenerated to improve seek
//        performance at the cost of power consumption
//        (note that if get_m_duration will be called this is the best option)
//   - JUSTINTIME_TABLE: a seek table is used, but generated only after
//        the first seek. This is a middle ground of performance vs. power
//   - NO_TABLE: no seek table is ever used. Lowest power consumption
//        assuming get_m_duration is never called. Also save heap memory a
//        seek table would ultimately comsume.
//
// RETURN:
//   SEEK_SUCCESS - mode has been set
//   SEEK_EUNSUPPORTED - mode of operation is not currently supported
//
//   ASSERTS on - set PERGENERATE, but didn't provide a seektable, etc.
//======================================================================
uint32 seek::set_mode(seek_mode_type emode)
{
   uint32 status = SEEK_SUCCESS;
   switch(emode)
   {
     case PREGENERATE_TABLE:
     case JUSTINTIME_TABLE:
     {
       m_update_table = true;
     }
     break;
     case NO_TABLE:
     {
       m_update_table = false;
     }
     break;
     default:
     {
       status = SEEK_FAILURE;
     }
     break;
   }
   return status;
}
