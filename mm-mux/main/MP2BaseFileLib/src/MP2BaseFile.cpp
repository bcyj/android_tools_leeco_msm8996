/* =======================================================================
                              MP2BaseFile.cpp
DESCRIPTION
  This module is to record MP2 Transport and program streams.



  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */


/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Sink/FileMux/MP2BaseFileLib/main/latest/src/MP2BaseFile.cpp

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "MMTimer.h"
#include "AEEStdDef.h"
#include <stdio.h>
#include <sys/time.h>
#include "MP2StreamMuxConstants.h"
#include "MP2BaseFile.h"
#include "MMMemory.h"
#include "MMDebugMsg.h"
#include "DataSourcePort.h"

#ifdef FEATURE_FILEMUX_SECURE_MP2
#include "smux_mem.h"
#include "Transcode.h"
#endif

#ifdef ENABLE_MUX_STATS
#include <cutils/properties.h>
#include "MMTime.h"
#endif
//wfd_cfg_parser.h
extern "C" int PargeCfgForIntValueForKey(char *filename, char *pKey, int *Val);

#define WFD_MUX_STATISTICS

#define PCR_STREAM_NUM 4

uint64 MP2BaseFile::m_llTimeBase = 0;

/* ==========================================================================
                 DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */

/*===========================================================================

FUNCTION  MP2BaseFile

DESCRIPTION
This is the MP2BaseFile class constructor - initializes the class members.
===========================================================================*/
MP2BaseFile::MP2BaseFile( MUX_create_params_type *Params,
                            MUX_fmt_type file_format,
                            MUX_brand_type file_brand,
                            MUX_handle_type *output_handle)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::MP2BaseFile");

  m_Params = NULL;
  m_llTimeBase = 0;
  m_llVideoDelayCorrection = 0;
  m_bLookforIframe = false;
  m_write_fail = false;
  m_nAudioFrameDuration90Khz = 0;
  m_nAudioFrameRate = 1;
  m_nVideoFrameRate = 1;
  m_nCurrAudioTime = 0;
  m_nCurrVideoTime = 0;
  m_output_offset = 0;
  m_nTS_Stream_bufsz = 0;
  m_nPrevTimeStamp = 0;
  m_nCurrPCR = 0;
  m_nAVCHrdDescrLen = 0;
  m_data_alignment_indicator = FALSE;
  m_bFirstFrameDrop = false;
  m_bGenerateTables = false;
  m_bBaseTimeStampTaken = false;
  m_adaptation_field = 0;
  m_Transport_priority = FALSE;
  m_PSI_payload_start_indicator_set = FALSE;
  m_PTS_DTS_Flags = 0;
  m_output_open = false;
  m_output_size = 0;
  space_out_near = false;
  space_out_imminent = false;
  space_out = false;
  mmc_free_space = 0;
  flash_free_space = 0;
  m_MP2TSPacket = NULL;
  m_MP2PESPacket = NULL;
  m_filePtr.efs_file = NULL;
  m_pAVCHrdDescr = NULL;
  m_MP2PCRTSPacket = NULL;
  m_hCritSect = NULL;
  #ifdef ENABLE_MUX_STATS
  memset(&muxStats,0,sizeof(mux_stats_struct));
  m_pStatTimer = NULL;
  muxStats.bEnableMuxStat = true;
  m_nDuration = 5000;
  if(0 != MM_Timer_Create( m_nDuration, 1, readStatTimerHandler, (void *)(this), &m_pStatTimer))
  {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Creation of timer failed");
  }
  #endif /*ENABLE_MUX_STATS*/
  if(Params && (file_format == MUX_FMT_MP2))
  {

    m_bVideoPresent = false;
    m_bAudioPresent = false;
    m_audio_stream_num = 0;
    m_video_stream_num = 0;
    //TBD need to change. Right now only H264 and PCM
    for(uint8 i=0; i < Params->num_streams; i++)
    {
      if(Params->streams[i].type == MUX_STREAM_VIDEO)
      {
        if(Params->streams[i].subinfo.video.format == MUX_STREAM_VIDEO_H264)
        {
          m_bVideoPresent = true;
        }
        m_video_stream_num = i;
      }

      if(Params->streams[i].type == MUX_STREAM_AUDIO)
      {
        if(Params->streams[i].subinfo.audio.format == MUX_STREAM_AUDIO_PCM ||
            Params->streams[i].subinfo.audio.format == MUX_STREAM_AUDIO_MPEG4_AAC ||
            Params->streams[i].subinfo.audio.format == MUX_STREAM_AUDIO_AC3 )
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::Audio Found-->");
          m_bAudioPresent = true;
        }
        m_audio_stream_num = i;
      }
    }
    if(!m_bAudioPresent && !m_bVideoPresent)
    {
      _success = false;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile::MP2BaseFile Unsupported codecs");
      return;
    }
    /**
    * Cache the movie and stream parameters)
    */
    m_Params  =  ( MUX_create_params_type *)
                  MM_Malloc(sizeof(MUX_create_params_type));
    if(!m_Params)
    {
      _success = false;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile::MP2BaseFile Cannot alloc Params memory");
      return;
    }
    memcpy(m_Params, Params, sizeof(MUX_create_params_type));


    m_Params->streams = (MUX_stream_create_params_type*)MM_Malloc(
        m_Params->num_streams * sizeof(MUX_stream_create_params_type));

    if(!m_Params->streams)
    {
      _success = false;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile::MP2BaseFile Cannot alloc Params memory");
      return;
    }
    memcpy(m_Params->streams, Params->streams,
          m_Params->num_streams * sizeof(MUX_stream_create_params_type));

    if(Params->streams[m_audio_stream_num].subinfo.audio.format ==
               MUX_STREAM_AUDIO_MPEG4_AAC)
    {
        if(Params->streams[m_audio_stream_num].
             subinfo.audio.audio_params.frames_per_sample == 1)
        {
            m_eAACFormat = AAC_FORMAT_ADTS;
        }
        else
        {
            m_eAACFormat = AAC_FORMAT_LOAS;
        }

    }

    m_file_brand = file_brand;

    if(output_handle)
    {
      memcpy(&m_output_handle, output_handle, sizeof(MUX_handle_type));
    }
    /* initialize the class member variables */
    InitData();

    //Check if Filler data NALU has to be sent
    int nVal = 0;
    int nRet;
    nRet = PargeCfgForIntValueForKey("/system/etc/wfdconfig.xml", "DisableFillerNalU", &nVal);

    if(nVal == 1)
    {
      m_bSendFillerNalu = false;
    }

    /* Open the necessary temporary files */
#ifdef _WRITE_TO_FILE
    OpenFiles();
#else
    m_output_open = TRUE;
#endif
    if((!m_output_open) || (!_success))
    {
      _success = false;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile::MP2BaseFile Failed to open necessary files");
      return;
    }
  }
  else
  {
    _success = false;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile::MP2BaseFile Invalid arguments");
    return;
  }

  //Generate the default AVC timing and HRD descriptor
  if(Params->streams[m_video_stream_num].subinfo.video.format
                            == MUX_STREAM_VIDEO_H264)
  {
    (void)GenerateAVCHRDTimingDescriptor(&m_sHRDParams);
  }

  if(m_bVideoPresent)
  {
    m_nVideoFrameRate = Params->streams[m_video_stream_num].subinfo.video.frame_rate;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile Video Properties....");
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile Video FPS = %d",
                  m_nVideoFrameRate);
  }

  if(m_bAudioPresent)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile Audio Properties....");
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile Audio Samplerate = %d",
                  Params->streams[m_audio_stream_num].subinfo.audio.sampling_frequency);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile Audio FrameLength = %d",
                  Params->streams[m_audio_stream_num].sample_delta);
    if(Params->streams[m_audio_stream_num].sample_delta)
    {
       if(Params->streams[m_audio_stream_num].subinfo.audio.sampling_frequency %
          Params->streams[m_audio_stream_num].sample_delta == 0)
       {

          m_nAudioFrameRate = Params->streams[m_audio_stream_num].subinfo.
                                                             audio.sampling_frequency/
                              Params->streams[m_audio_stream_num].sample_delta;
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile Audio FrameRate = %d",
                  m_nAudioFrameRate);
       }
       else
       {
         //If sample rate is not a multiple we need to take a common multiple to avoid
         // precision loss
         uint32 nCMultiple = Params->streams[m_audio_stream_num].subinfo.
                                                       audio.sampling_frequency *
                                 Params->streams[m_audio_stream_num].sample_delta;
         uint32 nSmpleRate = Params->streams[m_audio_stream_num].subinfo.
                                                       audio.sampling_frequency;
         uint32 nDelta = Params->streams[m_audio_stream_num].sample_delta;

         //Limitation this will not cover all random selections
         //But will cover standard ones
         if(nDelta)
         {
           m_nAudioFrameRate = nSmpleRate/nDelta;
         }
         else
         {
           m_nAudioFrameRate = 1;
         }
         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile Audio FrameRate = %d",
                                                                m_nAudioFrameRate);
       }
       m_nAudioFrameDuration90Khz =
              (m_Params->streams[m_audio_stream_num].sample_delta * 90000)/
              m_Params->streams[m_audio_stream_num].subinfo.audio.sampling_frequency;
    }
  }

  m_bAdjustTSforCompliance = true;

  if(Params->streams[m_audio_stream_num].sample_delta == 0 ||
    Params->streams[m_audio_stream_num].subinfo.audio.sampling_frequency == 0 ||
    Params->streams[m_video_stream_num].subinfo.video.frame_rate == 0)
  {
    m_bAdjustTSforCompliance = false;
  }

  // Generate PAT packet
  if(file_brand == MUX_BRAND_MP2TS)
  {
    GeneratePATPacket();
    if(_success)
    {
      // Generate PMT packet
      GeneratePMTPacket();
    }
  }
  MM_CriticalSection_Create(&m_hCritSect);

  return;
}

/*! ======================================================================
    @brief  Initialize the necessary class member variables.

    @detail This method is called to initialize the necessary class member variables.
========================================================================== */
void MP2BaseFile::InitData()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::InitData");
  _success = true;
  m_write_fail = false;
  m_bSendFillerNalu = true;
  m_hCritSect = NULL;
  m_bBaseTimeStampTaken = false;
  m_nPrevTimeStamp = 0;
  m_nCurrAudioTime = 0;
  m_nCurrVideoTime = 0;
  m_nCurrPCR = 0;
  m_filePtr.efs_file = NULL;
  m_output_open = false;
  m_output_offset = 0;
  m_output_size = 0;
  space_out_near = false;
  space_out_imminent = false;
  space_out = false;
  mmc_free_space = 0;
  flash_free_space = 0;
  m_PSI_payload_start_indicator_set = FALSE;
  m_MP2TS_continuty_counter[0] = 0;
  m_MP2TS_continuty_counter[1] = 0;
  m_MP2TS_continuty_counter[PCR_STREAM_NUM] = 0;
  m_Transport_priority = FALSE;
  m_randon_access_indicator = FALSE;
  m_MP2TS_table_continuty_counter[0] = 0;
  m_MP2TS_table_continuty_counter[1] = 0;
  m_pAVCHrdDescr = NULL;
  m_nAVCHrdDescrLen = 0;
  m_adaptation_field = 0;
  m_PTS_DTS_Flags = 0;
  m_nVideoFrameRate = 1;
  m_nAudioFrameRate = 1;
  m_bGenerateTables = false;
  m_data_alignment_indicator = FALSE;
  for(int i=0; i < MUX_MAX_MEDIA_STREAMS; i++)
  {
    m_pHeader[i]     = NULL;
    m_nHeaderSize[i] = 0;
    m_bHeaderSent[i] = false;
  }
  memset(&m_sHRDParams,0, sizeof(m_sHRDParams));

  m_MP2TSPacket = (uint8 *) MM_Malloc(TS_PKT_SIZE);
  if(m_MP2TSPacket == NULL)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile::InitData MALLOC failure");
    return;
  }

  m_MP2PCRTSPacket = (uint8 *) MM_Malloc(TS_PKT_SIZE);
  if(m_MP2PCRTSPacket == NULL)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile::InitData MALLOC failure");
    return;
  }
  m_MP2PESPacket = (uint8 *) MM_Malloc(PES_PKT_SIZE);
  if(m_MP2PESPacket == NULL)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile::InitData MALLOC failure");
    MM_Free(m_MP2TSPacket);
    MM_Free(m_MP2PCRTSPacket);
    m_MP2TSPacket = NULL;
    m_MP2PCRTSPacket = NULL;
    return;
  }
  m_video_frame_timer = NULL;
}

#ifdef _WRITE_TO_FILE
/*! ======================================================================
    @brief  open files

    @detail Open all the necessary efs or memory files to write final and intermediate data
========================================================================== */
void MP2BaseFile::OpenFiles()
{
  OSCL_FILE *efs_fp = NULL;
  uint32 offset = 0;

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::OpenFiles");




  if(m_output_handle.method == MUX_METHOD_PORT)
  {
     m_output_open = TRUE;
     _success = true;
     return;
  }




  if(std_strlen(m_output_handle.efs.filename) == 0)
  {
    return;
  }
  /* If the output destination is a file in EFS, open the file now. */
  if (m_output_handle.method == MUX_METHOD_EFS)
  {
    /* Open the output file, mark it for asynchronous writing, and
     ** increase its buffer size. Note that the buffer size must be
     ** a power of two.
    */
    efs_fp = OSCL_FileOpen (m_output_handle.efs.filename, ("wb+"));
    if (efs_fp == NULL)
    {
      _success = false;
      return;
    }
    m_filePtr.efs_file = efs_fp;
    uint64 freespace;

#if defined PLATFORM_LTK && defined WIN32
    freespace = 0xffffffff;
#else
    freespace = OSCL_GetFileSysFreeSpace (m_output_handle.efs.filename);
#endif
    //Temporary work around until more than 4gb is supported.
    if(freespace >=  ((uint64)1 << 32))
    {
        freespace = ((uint64)1 << 32) - 1;
    }

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                 "MP2BaseFile: Freespace = %d", (int)freespace);
    if(m_save_to_mmc)
    {
      mmc_free_space = (uint32) freespace;
    }
    else
    {
      flash_free_space = (uint32)freespace;
    }
  }
  /* Mark movie file status as open. */
  m_output_open = TRUE;
}
#endif

/*!*************************************************************************
 * @brief     Timer handler for reading statistics flag from command line
 *
 * @param[in] ptr Reference to the current instance
 *
 * @return    NONE
 *
 * @note
 **************************************************************************/

#ifdef ENABLE_MUX_STATS
void MP2BaseFile::readStatTimerHandler(void* ptr)
{
    MP2BaseFile* mp2bf= (MP2BaseFile*)ptr;
    char szTemp[PROPERTY_VALUE_MAX];
    if(property_get("persist.debug.enable_mux_stats",szTemp,"true")<0)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Failed to read persist.debug.enable_mux_stats");
        return;
    }
    if(strcmp(szTemp,"false")==0)
    {
        memset(&(mp2bf->muxStats),0,sizeof(muxStats));
        mp2bf->muxStats.bEnableMuxStat = false;
    }
    else
    {
        mp2bf->muxStats.bEnableMuxStat = true;
    }
}
#endif


/*! ======================================================================
    @brief  Determine if the path is for MMC

    @detail Determine if the path is for MMC

    @param[in]  path path to determine

========================================================================== */
bool MP2BaseFile::Destination_Mmc(const WCHAR * /*path*/)
{
    return true;
}
/*===========================================================================

FUNCTION Output

DESCRIPTION
  This function writes the given data to the output destination.

DEPENDENCIES
  None.

RETURN VALUE
  The value TRUE is returned if the data was output successfully.  Otherwise,
  the value FALSE is returned.

SIDE EFFECTS
  None.

===========================================================================*/
boolean MP2BaseFile::Output (const uint8  *data, uint32 offset, uint32 len, bool bEOF)
{
  //fs_size_t  write_count;
  uint32  write_count;
  uint32 chunk;
  MM_CriticalSection_Enter(m_hCritSect);

  /* Branch according to method. */
  switch (m_output_handle.method)
  {
    case MUX_METHOD_MEM:
      /* Check for memory buffer overflow. */
      if (offset + len > m_output_handle.mem.len)
      {
        MM_CriticalSection_Leave(m_hCritSect);
        return FALSE;
      }
      /* Write data out to memory buffer. */
      memcpy (m_output_handle.mem.buf + offset, data, len);
      m_output_offset = offset + len;
      m_output_size = MAX (m_output_size, m_output_offset);
      MM_CriticalSection_Leave(m_hCritSect);
      return TRUE;

    case MUX_METHOD_EFS:
#ifdef _WRITE_TO_FILE
      /* If offset is different from last offset, perform seek operation. */
    //  This case need to be considered.............................

      if (offset != m_output_offset)
      {
        if (OSCL_FileSeek (m_filePtr.efs_file, (uint32)offset, SEEK_SET) != 0)
        {
          MM_CriticalSection_Leave(m_hCritSect);
          return FALSE;
        }
        m_output_offset = offset;
      }

      /* Write data out to the filesystem.  Write in 32KB chunks since SFAT
       ** service cannot handle larger than 65535 bytes at a time, and so we
       ** will pick the next power of 2 smaller than this.
      */
      while (len > 0)
      {
        /* Select next chunk size. */
        chunk = MIN (65536, len);
        /* Write out the chunk. */
        write_count = OSCL_FileWrite ((void *) data, 1, chunk, m_filePtr.efs_file);
        if (write_count != chunk)
        {
          MM_CriticalSection_Leave(m_hCritSect);
          return FALSE;
        }
        /* Move on to next chunk. */
        len -= chunk;
        data += chunk;
        m_output_offset += chunk;
      }
      m_output_size = MAX (m_output_size, m_output_offset);
      MM_CriticalSection_Leave(m_hCritSect);
#endif
      return TRUE;

    case MUX_METHOD_CLIENT:
      /* Send data to client callback. */
      m_output_handle.client.data_push_fn((uint8 *) data, len, offset,(void *)m_output_handle.client.client_data);
      m_output_offset = offset + len;
      m_output_size = MAX (m_output_size, m_output_offset);
      MM_CriticalSection_Leave(m_hCritSect);
      return TRUE;

    case MUX_METHOD_PORT:
    {

       //When streaming TS over RTP we need to follow the restriction of 7 TS bpackets per
       // RTP payload for Wifi Display. But it will work as well for other applications

       if((m_nTS_Stream_bufsz + len) <= TS_RTP_PAYLOAD_SIZE) {
           memcpy(m_TS_Stream_buf + m_nTS_Stream_bufsz, data, len);
           m_nTS_Stream_bufsz += len;
       }

       if(m_nTS_Stream_bufsz < TS_RTP_PAYLOAD_SIZE &&
          !bEOF)
       {
           MM_CriticalSection_Leave(m_hCritSect);
           return TRUE;
       }

       video::iSourcePort::DataSourceReturnCode eRet= video::iSourcePort::DS_SUCCESS;
       int64 nBytesWritten = 0;
       //int32 local_offset = 0;

       //WRITE using client IStreamport write API
       //Writer in a loop in case client is able to write in
       //smaller chunks.

       if(m_output_handle.OputPort.pOputStream != NULL)
       {
          eRet = m_output_handle.OputPort.pOputStream->WriteBlockData(m_TS_Stream_buf,
                      m_nTS_Stream_bufsz, 0, bEOF,&nBytesWritten);
       }

       if(eRet != video::iSourcePort::DS_SUCCESS ||
          (uint64)nBytesWritten != m_nTS_Stream_bufsz)
       {
           m_nTS_Stream_bufsz = 0;
           if(nBytesWritten == -1)
           {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile Port returns -1");
              _success = false;
           }

           MM_CriticalSection_Leave(m_hCritSect);
           return FALSE;
       }

       m_nTS_Stream_bufsz = 0;
       MM_CriticalSection_Leave(m_hCritSect);
       return TRUE;
    }

    /*
     * MUX_METHOD_TRANSCODE mode:
     * This is an initial implementation for testing purposes, and is
     * yet to be modified for real usage with the Transcode class.
     * note - when using ION buffers between processes, we will first have to call ion_map.
     */
#ifdef FEATURE_FILEMUX_SECURE_MP2

    case MUX_METHOD_TRANSCODE:
    {
#if 0
      Transcode* transcode = (Transcode*)m_output_handle.transcode.pTranscode;
      struct smux_ion_info* output_handle = (struct smux_ion_info*)transcode->GetBuffer();

      // wait for new available output buffer
      for(int i = 0; (i < 100) && (!output_handle); i++)
      {
        usleep(100);
        output_handle = (struct smux_ion_info*)transcode->GetBuffer();
      }

      if(!output_handle)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "No output buffer");
        return MUX_FAIL;
      }

      if(len <= output_handle->sbuf_len)
      {
        memcpy(output_handle->ion_sbuffer, data, len);
        transcode->SendCommand(output_handle, len);
      }
      else
      {
        transcode->SendCommand(output_handle, 0);
      }

      MM_CriticalSection_Leave(m_hCritSect);

      break;
#endif
    }
#endif
    case MUX_METHOD_INVALID:
    default:
      MM_CriticalSection_Leave(m_hCritSect);
      return FALSE;
    }

  return TRUE;
}


/*===========================================================================
FUNCTION Space_Check

DESCRIPTION
  This function calculates the available space

DEPENDENCIES


RETURN VALUE
  space left

SIDE EFFECTS
  None.

===========================================================================*/
uint32 MP2BaseFile::Space_Check(void)
{
  uint32  bytes_available = 0;
 // uint32  free_space;
 //
 // MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::Space_Check");
 // /* If writing to memory buffer, bytes available is total memory buffer
 //   ** size minus the bytes we've already used, with a margin.
 // */
 // if (m_output_handle.method == MUX_METHOD_MEM)
 // {
 //   bytes_available = m_output_handle.mem.len - m_output_size;
 //   bytes_available -= MIN (bytes_available, MUX_MEM_MARGIN);
 // }
 //
 // /* If writing to EFS, return space available on the EFS using movie
 //   ** file path, minus some margin amount.
 // */
 // else if ((m_output_handle.method == MUX_METHOD_EFS) || m_use_temp_file_1)
 // {
 //   if (m_save_to_mmc)
 //   {
 //     free_space = (mmc_free_space - m_output_size - (m_temp_file_size * m_mmc_temp_file_size_factor)) -
 //                  (m_temp_file_size_3 * m_mmc_temp_file_size_factor_3);
 //   }
    ///* Else, the destination is flash, memory, or something else. */
 //   else
 //   {
 //     free_space = flash_free_space;
 //     if (m_output_handle.method == MUX_METHOD_EFS)
 //     {
 //       free_space -= m_output_size;
 //     }
 //     free_space -= m_temp_file_size * m_temp_file_size_factor;
 //     free_space -= m_temp_file_size_3 * m_temp_file_size_factor_3;
 //   }
 //   free_space -= MIN (free_space, EFS_MARGIN);
 //   bytes_available = free_space;
 // }
 // else
 // {
 //   /* If we reach here, we have no way of knowing how much space is left
 //    ** on the recording device, so we will return a very large number to
 //    ** keep the movie writer from stopping.  It is up to the client to
 //    ** make sure we don't actually run out of space!
 //   */
 //   bytes_available = (uint32) -1;
 // }
  return bytes_available;
}

/*===========================================================================

FUNCTION MUX_Process_Sample

DESCRIPTION
  This function process the audio and video samples.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  This functionis not thread safe.

===========================================================================*/
MUX_STATUS MP2BaseFile::MUX_Process_Sample( uint32 stream_number,
                                        uint32 num_samples,
                                        const MUX_sample_info_type  *sample_info,
                                        const uint8  *sample_data)
{
  MUX_STATUS return_status = MUX_SUCCESS;
  uint32 index = 0;
  uint16 PES_Header_Size = 0;
  uint32 sample_data_offset = 0;
  MUX_sample_info_type current_sample_info;
  uint8               *current_sample = (uint8*)sample_data;
  uint8               *temp_ptr = NULL;
  m_nTS_Stream_bufsz = 0;

  // check the given input
  if(!sample_info)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " MP2BaseFile::MUX_Process_Sample Invalid Arg sample_info ");
    return MUX_FAIL;
  }

  MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_MEDIUM,
                                 "MP2BaseFile::MUX_Process_Sample stream number  %d Size %d tim = %d, extra_data_size =%d",
                                 stream_number, sample_info->size, sample_info->time, sample_info->extra_data_size);

  memcpy(&current_sample_info, sample_info, sizeof(MUX_sample_info_type));

  if(m_bGenerateTables)
  {
    GeneratePATPacket();
    GeneratePMTPacket();
    m_bGenerateTables = false;
  }

  if(m_bAudioPresent && m_bVideoPresent)
  {
    if(stream_number == m_audio_stream_num)
    {
      if(!m_bBaseTimeStampTaken)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile:: Drop Early Audio No Video yet");
        return MUX_SUCCESS;
      }
      else if( ((int64)sample_info->time * 90) < ((int64)m_nBaseTimestamp + 9000) )
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile:: Drop Early Audio");
        return MUX_SUCCESS;
      }
      else if( (int64)sample_info->time * 90 - ((int64)m_nBaseTimestamp)
               < (int64)m_nCurrPCR)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile:: Drop Late Audio");
        return MUX_SUCCESS;
      }
    }
    if(stream_number == m_video_stream_num)
    {
      if(m_bLookforIframe == true)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile:: Looking for I frame");
        if(sample_info->sync)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile:: Looking for I frame... found");
          m_bLookforIframe = false;
        }
        else
        {
          return MUX_SUCCESS;
        }
      }

      int32 nFrameDelta90KHz = 0;
      int64 currTime90KHz = ((int64)sample_info->time * 90 - (int64)m_nBaseTimestamp);

      if(m_nVideoFrameRate && m_bAdjustTSforCompliance)
      {
          //If fixed framerate is used, timestamp jitter is corrected later and this has to be
          // accounted for while checking if PTS is behing PCR
          nFrameDelta90KHz = 90000/m_nVideoFrameRate;

          currTime90KHz = (((int64)sample_info->time * 90 - (int64)m_nBaseTimestamp)/
                              nFrameDelta90KHz) * nFrameDelta90KHz;
      }


      if(m_bBaseTimeStampTaken && currTime90KHz <= (int64)m_nCurrPCR)
      {
        MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile:: Drop Late Video %u %u %u",
                     (unsigned int)( (int64)sample_info->time * 90 - (int64)m_nBaseTimestamp),
                     (unsigned int)currTime90KHz,
                     (unsigned int)(m_nCurrPCR));
        m_bLookforIframe = true;
        return MUX_OUTDATED;
      }
    }
  }



  if ( num_samples && sample_data && _success )
  {

    if(!m_bBaseTimeStampTaken && stream_number == m_video_stream_num)
    {
      if(!m_bFirstFrameDrop)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFile, drop first frame to improve the G2G latency");
        m_bLookforIframe = true;
        m_bFirstFrameDrop = true;
        return MUX_OUTDATED;
      }

      m_nBaseTimestamp = 0;
      m_nBaseTimestamp = current_sample_info.time * (uint64)90;

      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "Mux Taking base time %u  %u",
                   (unsigned int)current_sample_info.time,
                   (unsigned int)m_nBaseTimestamp);

      m_bBaseTimeStampTaken = true;

      if(!m_video_frame_timer)
      {
         if(0 != MM_Timer_Create((int)90, 1, &MP2BaseFile::timer_callback,
                                (void *)this, &m_video_frame_timer))
         {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Timer creation failed!!!!");
         }

         struct timespec tempTime;
         clock_gettime(CLOCK_MONOTONIC, &tempTime);
         m_llTimeBase = (tempTime.tv_sec * (uint64)1000000000) +
                           (uint64)(tempTime.tv_nsec);

         if((int64)(m_llTimeBase - m_nBaseTimestamp) > (int64)0)
         {
           //Theoretically PCR time base must always be larger because,
           // video has travelled some distance carrying the timestamp
           m_llVideoDelayCorrection = m_llTimeBase - m_nBaseTimestamp;
           MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,
               "Mux Taking base time Delay Correction VideoBase %u  PCRBase %u  Corr %u",
               (unsigned int)m_nBaseTimestamp,
               (unsigned int)m_llTimeBase,
               (unsigned int)m_llVideoDelayCorrection);
         }
         MUX_sample_info_type sSampleInfo;
         memset(&sSampleInfo,0,sizeof(sSampleInfo));
         GenerateMP2TSPCRPacket(PCR_STREAM_NUM,&sSampleInfo);
      }
      m_nBaseTimestamp -= MUX_MP2_PTS_DELAY_CONST_90KHZ;
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                  "Mux Taking base time Base Adjusted %u",
                   (unsigned int)m_nBaseTimestamp);
    }

    if(stream_number == 0 &&
       ((int64)((uint64)current_sample_info.time * 90 - m_nBaseTimestamp) < 0))
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile Audio with old TS Dropped");
      //Drop audio frames which arrives with a TS which is less than start time
      return MUX_SUCCESS;
    }

    // WFD:STATISTICS -- start
    #ifdef ENABLE_MUX_STATS
    if(muxStats.bEnableMuxStat)
    {
         if(m_Params->streams[stream_number].type == MUX_STREAM_VIDEO)
         {
            MM_Time_GetTime(&muxStats.nStartTime);
            MM_MSG_PRIO2(MM_STATISTICS, MM_PRIO_MEDIUM,
             "WFD:STATISTICS:  MP2BaseFile::MUX_Process_Sample stream number %d \
              StartTime = %lu", (int)stream_number,(unsigned long)muxStats.nStartTime);
         }
    }
    #endif /* ENABLE_MUX_STATS */
    // WFD:STATISTICS -- end
    //For I-Frames we need to set the random access indicator and transport priority to 1.
    if( sample_info->sync)
    {
      m_Transport_priority = 1;
      m_randon_access_indicator = 1;
    }
    else
    {
      m_Transport_priority = 0;
    }
    /* The first packet will have the PES header and the payload */
    m_adaptation_field = MUX_MP2_ADAPTATON_AND_PAYLOAD;
    /* We always put the PTS in PES header */
    m_PTS_DTS_Flags = 3;
    /* For each frame/sample continuity counter will start from 0 */
   // m_MP2TS_continuty_counter = 0;
    /* Generate PES header for begining of the each sample
     * This code is common for both audio and video samples
    */
    if(m_MP2PESPacket)
    {
      PES_Header_Size = GeneratePESPacket(stream_number, sample_info, m_MP2PESPacket);
      if(PES_Header_Size == 0)
      {
        return MUX_OUTDATED;
      }
    }
    /* Initialize the inxex to 0 */
    index = 0;
    /* We need to set this for the first TS packet of each frame,
     * this flag will control and make sure that we set for the first TS packet only */
    m_PSI_payload_start_indicator_set = 0;
    if( m_Params->streams[stream_number].type == MUX_STREAM_VIDEO )
    {

      /* This is the first TS packet which carries the PES header and the payload
       * sample_data_offset will maintain the offset in the sample data
      */
      if(m_bHeaderSent[stream_number] == false &&
         m_nHeaderSize[stream_number] &&
         sample_info->sync)
      {
         temp_ptr = (uint8*)MM_Malloc(m_nHeaderSize[stream_number] +
                                            sample_info->size);
         current_sample = temp_ptr;
         if(!current_sample)
         {
            current_sample = (uint8*)sample_data;
         }
         else
         {
             memcpy(current_sample, m_pHeader[stream_number], m_nHeaderSize[stream_number]);
             memcpy(current_sample + m_nHeaderSize[stream_number],
                    sample_data, sample_info->size);
             current_sample_info.size += m_nHeaderSize[stream_number];
         }
         m_bHeaderSent[stream_number] = true;
      }

      sample_data_offset += GenerateMP2TSVideoPacket( stream_number, &current_sample_info,
                                                      m_MP2PESPacket, PES_Header_Size,
                                                      current_sample, sample_data_offset );
      /* Reset all these flags for remaining TS packets till we completely process the frame */
      //  m_Transport_priority = 0;
      m_randon_access_indicator = 0;
      m_adaptation_field = 0;
      m_PTS_DTS_Flags = 0;
      if(_success)
      {
        uint32 nBytesWritten = 0;
        /* Until we completely packed the sample */
        while(sample_data_offset < current_sample_info.size)
        {
          nBytesWritten = GenerateMP2TSVideoPacket( stream_number,
                                                          &current_sample_info,
                                                          NULL, 0, current_sample,
                                                          sample_data_offset );
          sample_data_offset += nBytesWritten;
          if(!_success || !nBytesWritten)
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                   "MP2BaseFile::MUX_Process_Sample failed to generate TS packet for video stream");
            if(!_success)
            {
                return_status = MUX_FAIL;
                _success = true;
            }
            else
            {
                m_bLookforIframe = true;
                return_status = MUX_OUTDATED;
            }
            break;
          }
        }
        m_PSI_payload_start_indicator_set = 0;
        current_sample_info.time += 33;
        current_sample_info.size = 0;
        m_adaptation_field = MUX_MP2_ADAPTATON_AND_PAYLOAD;


        if(current_sample_info.fmt_pvtdata_ptr && current_sample_info.fmt_pvtdata_size)
        {
          if(current_sample_info.fmt_pvtdata_ptr[0]
                                       == VIDEO_PVTDATA_TYPE_FILLERNALU ||
             current_sample_info.fmt_pvtdata_ptr[0]
                                      == VIDEO_PVTDATA_TYPE_FILLERNALU_ENCRYPTED)
          {
            /**------------------------------------------------------------------------
            Filler NALU format:= |SigByte|Size|PES Pvt Data|Size|Filler NALU |
            ---------------------------------------------------------------------------
            */

            uint32 nPESPvtSize = 0;
            uint32 nPayloadLen = 0;
            uint32 nNaluOffset = 0;


            bool bExtraValid = true;

            current_sample_info.decode_time     = 0;
            current_sample_info.delta           = 0;
            current_sample_info.size            = 0;
            current_sample_info.time            = 0;
            current_sample_info.extra_data_size = 0;

            m_PSI_payload_start_indicator_set = false;
            /* Reset all these flags for remaining TS packets till we completely process the frame */
            //  m_Transport_priority = 0;
            m_randon_access_indicator = 0;
            m_adaptation_field = 0;
            m_PTS_DTS_Flags = 0;

            if(current_sample_info.fmt_pvtdata_ptr[0]
                                 == VIDEO_PVTDATA_TYPE_FILLERNALU_ENCRYPTED)
            {
              bExtraValid = false;
              if(current_sample_info.fmt_pvtdata_size >
                       1/*Sig Byte*/ +(uint32)current_sample_info.fmt_pvtdata_ptr[1])
              {
                nPESPvtSize = current_sample_info.fmt_pvtdata_ptr[1];
                if(nPESPvtSize > 0)
                {
                  bExtraValid = true;
                  current_sample_info.extra_data_size = nPESPvtSize;
                  current_sample_info.extra_data_ptr =
                               current_sample_info.fmt_pvtdata_ptr + 1 + 1;
                  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"MP2BaseFile FillerNALU: Current Extra Size = %d",
                             current_sample_info.extra_data_size);

                }
              }
            }

            if(bExtraValid)
            {
              bExtraValid = false;

              nNaluOffset = 1;
              if(nPESPvtSize)
              {
                nNaluOffset = 1/*Sig*/ + 1/*Size*/ + nPESPvtSize;
              }

              if(nNaluOffset+ 1 /*Size*/ +
                        current_sample_info.fmt_pvtdata_ptr[nNaluOffset]
                                    <= current_sample_info.fmt_pvtdata_size)
              {
                bExtraValid = true;
                current_sample_info.size = current_sample_info.fmt_pvtdata_ptr[nNaluOffset];
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,"MP2BaseFile FillerNALU: Current Sample Size = %d",
                             current_sample_info.size);
              }
            }

            if(bExtraValid)
            {
              if(m_MP2PESPacket)
              {
                PES_Header_Size = GeneratePESPacket(stream_number, &current_sample_info, m_MP2PESPacket);
              }
              (void)GenerateMP2TSVideoPacket( stream_number,
                                              &current_sample_info,
                                              m_MP2PESPacket, PES_Header_Size,
                                              current_sample_info.fmt_pvtdata_ptr + nNaluOffset + 1,
                                              0 );
            }
          }
        }
      }
    }
    if( m_Params->streams[stream_number].type == MUX_STREAM_AUDIO )
    {
      /* This is the first TS packet which carries the PES header and the payload
       * sample_data_offset will maintain the offset in the sample data
      */
      if(m_bHeaderSent[stream_number] == false &&
         m_nHeaderSize[stream_number])
      {
         temp_ptr = (uint8*)MM_Malloc(m_nHeaderSize[stream_number] +
                                            sample_info->size);
         current_sample = temp_ptr;
         if(!current_sample)
         {
            current_sample = (uint8*)sample_data;
         }
         else
         {
             memcpy(current_sample, m_pHeader[stream_number], m_nHeaderSize[stream_number]);
             memcpy(current_sample + m_nHeaderSize[stream_number],
                    sample_data, sample_info->size);
             current_sample_info.size += m_nHeaderSize[stream_number];
         }
         m_bHeaderSent[stream_number] = true;
      }

//      if(m_MP2PESPacket)
//      {
//        PES_Header_Size = GeneratePESPacket(stream_number, &current_sample_info, m_MP2PESPacket);
//      }

      sample_data_offset += GenerateMP2TSAudioPacket( stream_number, &current_sample_info,
                                                      m_MP2PESPacket, PES_Header_Size,
                                                      current_sample, sample_data_offset );
      /* Reset all these flags for remaining TS packets till we completely process the frame */
   //   m_Transport_priority = 0;
      m_randon_access_indicator = 0;
      m_adaptation_field = 0;
      m_PTS_DTS_Flags = 0;
      if(_success)
      {
        uint32 nBytesWritten;
        /* Until we completely packed the sample */
        while(sample_data_offset < current_sample_info.size)
        {
          nBytesWritten = GenerateMP2TSAudioPacket( stream_number,
                                                          &current_sample_info,
                                                          NULL, 0, current_sample,
                                                          sample_data_offset );
          sample_data_offset += nBytesWritten;
          if(!_success || !nBytesWritten)
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                                   "MP2BaseFile::MUX_Process_Sample failed to generate TS packet for audio stream");
            if(!_success)
            {
                return_status = MUX_FAIL;
                _success = true;
            }
            else
            {
                //Drop the current Audio Frame. For audio no sync frame or retry required.
                return_status = MUX_SUCCESS;
            }
            break;
          }
          if(!sample_data_offset)
          {
              if(temp_ptr)
              {
                 MM_Free(temp_ptr);
              }
              return MUX_SUCCESS;
          }
        }
      }
    }

    // WFD:STATISTICS -- start
    #ifdef ENABLE_MUX_STATS
    if(muxStats.bEnableMuxStat)
    {
        if(m_Params->streams[stream_number].type == MUX_STREAM_VIDEO)
        {
            MM_Time_GetTime(&muxStats.nEndTime);
            MM_MSG_PRIO2(MM_STATISTICS, MM_PRIO_MEDIUM,
            "WFD:STATISTICS:  MP2BaseFile::MUX_Process_Sample stream number %d \
            EndTime = %lu, ", stream_number,muxStats.nEndTime);
            if(muxStats.nMaxTime < (muxStats.nEndTime-muxStats.nStartTime))
            {
                muxStats.nMaxTime = muxStats.nEndTime-muxStats.nStartTime;
            }
            if(muxStats.nStatCount%100 == 0)
            {
                MM_MSG_PRIO2(MM_STATISTICS, MM_PRIO_HIGH,
                "WFD:STATISTICS:  MP2BaseFile::Total time taken in processing\
                video sample is %lu  ms, maximum time taken is %lu ms",
                muxStats.nEndTime-muxStats.nStartTime, muxStats.nMaxTime);
            }
            muxStats.nStatCount++;
        }
    }
    #endif /* ENABLE_MUX_STATS */
    // WFD:STATISTICS -- end

    if(m_write_fail == TRUE)
    {
      return_status = MUX_WRITE_FAILED;
    }
  }
  else
  {
    return_status = MUX_FAIL;
  }
  if(temp_ptr)
  {
     MM_Free(temp_ptr);
  }
  return return_status;
}
/*===========================================================================

FUNCTION timer_callback

DESCRIPTION
  This function replace avc start codecs with NAL sizes.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/
void MP2BaseFile::timer_callback(void *pMp2File)
{
  MP2BaseFile *pMP2BaseFile = (MP2BaseFile*)pMp2File;
  MUX_sample_info_type sSampleInfo;

  memset(&sSampleInfo, 0, sizeof(sSampleInfo));

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile:: SendPCR");

 // pMP2BaseFile->GeneratePATPacket();
 // pMP2BaseFile->GeneratePMTPacket();
  pMP2BaseFile->m_bGenerateTables = true;
  for(int i = 0; i < MUX_MAX_MEDIA_STREAMS; i++)
  {
    pMP2BaseFile->m_bHeaderSent[i]   = false;
  }

  struct timespec tempTime;
  clock_gettime(CLOCK_MONOTONIC, &tempTime);
  uint64 lTime = ((uint64)tempTime.tv_sec * 1000000000) +
                       (uint64)(tempTime.tv_nsec);

  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
                "Timer PCR %lu %lu",
               (unsigned long)tempTime.tv_sec,
               (unsigned long)tempTime.tv_nsec);

  lTime -= pMP2BaseFile->m_llTimeBase;

  sSampleInfo.time = (uint64)lTime / 1000000000;
  sSampleInfo.delta =(uint64)lTime -
                       ((uint64)sSampleInfo.time * 1000000000);

  pMP2BaseFile->GenerateMP2TSPCRPacket(PCR_STREAM_NUM, &sSampleInfo);
  return;
}



/*===========================================================================

FUNCTION MUX_Modify_AVC_Start_Code

DESCRIPTION
  This function replace avc start codecs with NAL sizes.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/
void MP2BaseFile::MUX_Modify_AVC_Start_Code(
                            const uint8 * pData,
                            uint32  *nSize,
                            bool bHeader)
{
    uint32 nFirstOccurence = 0;
    uint32 sps_size=0;
    uint32 pps_size=0;

    uint32 pattern = (uint32)((*(pData + 0) << 24) |
                        (*(pData + 1) << 16) |
                        (*(pData + 2) <<  8) |
                        (*(pData + 3) <<  0)
                        );
    if(pattern != 0x00000001)
    {
        /**Could not find start code at the begining of
        buffer
        **/
       return;
    }

    if(bHeader == true)
    {
        sps_size = MUX_FindNextPattern(pData + 4, *nSize, 0x00000001);
        pps_size = *nSize - sps_size - 4 - 4;
        memcpy((uint8 *)pData + 3, (uint8 *)pData + 4, sps_size);
        memcpy((uint8 *)pData + 3/*marker in videofmt format*/ + sps_size
                     /*skip Annex B header*/ + 3
                     /*marker size in FileMuxformat*/,
                     (uint8 *)pData + sps_size + 4 + 4, pps_size);
       ((uint8 *)pData)[0] = 0xE1;//Reserved
       ((uint8 *)pData)[1] = 0x0;
       ((uint8 *)pData)[2] = (uint8)sps_size;
       ((uint8 *)pData)[sps_size + 3] = 0x1;
       ((uint8 *)pData)[sps_size + 4] = 0x0;
       ((uint8 *)pData)[sps_size + 5] = (uint8)pps_size;
       *nSize-=2;
    }
    else
    {
        uint32 nPos = nFirstOccurence;
        uint32 nTempBufSize = *nSize;
        uint32 nNalSize = 0;

        while(nTempBufSize >= sizeof(uint32))
        {
           //Replace each start code for NALUs in the frame
           //with corresponding size in big endian.
           nNalSize = MUX_FindNextPattern(pData + nPos + sizeof(uint32),
                                          (uint32)(nTempBufSize - sizeof(uint32)),
                                          0x00000001);

           ((uint8 *)pData)[nPos]     = (uint8)(nNalSize >> 24);
           ((uint8 *)pData)[nPos + 1] = (nNalSize >> 16) & 0xff;
           ((uint8 *)pData)[nPos + 2] = (nNalSize >> 8 ) & 0xff;
           ((uint8 *)pData)[nPos + 3] = (nNalSize      ) & 0xff;

           nTempBufSize -= (nNalSize + 4);
           nPos         += (nNalSize + 4);

        }
    }
    return;
}

/*===========================================================================

FUNCTION MUX_FindNextPattern

DESCRIPTION
  This function finds the offset from start of bitstream for a given pattern.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/
uint32 MP2BaseFile::MUX_FindNextPattern(
                           const uint8 *streamBuf,
                           uint32 bufSize,
                           uint32 startcode)
{
    uint8 *pstreamBuf;
    uint32 pattern;
    uint32 tempbufSize = bufSize;

    pstreamBuf = (uint8*)streamBuf;

    //search for startcode
    pattern =  (uint32)((*(pstreamBuf + 0) << 24) |
                        (*(pstreamBuf + 1) << 16) |
                        (*(pstreamBuf + 2) <<  8) |
                        (*(pstreamBuf + 3) <<  0)
                        );

    //Check if next four bytes are startcode
    while((pattern != startcode) && tempbufSize)
    {
      pstreamBuf++;
      tempbufSize--;

      pattern =  (uint32)((*(pstreamBuf + 0) << 24) |
                          (*(pstreamBuf + 1) << 16) |
                          (*(pstreamBuf + 2) <<  8) |
                          (*(pstreamBuf + 3) <<  0)
                          );
    }

    if(0 == tempbufSize)
    {
       return bufSize;
    }

    return (bufSize - tempbufSize);
}

/*===========================================================================

FUNCTION MUX_end_Processing

DESCRIPTION
  This function will end the recording

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/
MUX_STATUS MP2BaseFile::MUX_end_Processing()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::MUX_end_Processing");
  handle_close();
#ifdef _WRITE_TO_FILE
  if(m_filePtr.efs_file)
  {
    OSCL_FileClose(m_filePtr.efs_file);
    m_filePtr.efs_file = NULL;
  }
#endif
  return MUX_SUCCESS;
}

MUX_STATUS MP2BaseFile::MUX_update_streamport(uint64_t streamport)
{

  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                    "MP2BaseFile:: Update Stream Port with %ld",streamport);

  MM_CriticalSection_Enter(m_hCritSect);

  m_output_handle.OputPort.pOputStream = reinterpret_cast<video::iStreamPort*>(streamport);

 _success = true;

  MM_CriticalSection_Leave(m_hCritSect);

  return MUX_SUCCESS;
}

MUX_STATUS MP2BaseFile::MUX_get_current_PTS(uint64 *pnPTS)
{
    if(pnPTS)
    {
        *pnPTS = m_nCurrAudioTime > m_nCurrVideoTime ?
                  m_nCurrAudioTime:m_nCurrVideoTime;
        return MUX_SUCCESS;
    }
    return MUX_FAIL;
}

/*===========================================================================

FUNCTION  ~MP2BaseFile

DESCRIPTION
Destructor for the MP2BaseFile class

===========================================================================*/
MP2BaseFile::~MP2BaseFile()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::~MP2BaseFile");

  if(m_video_frame_timer)
  {
    MM_Timer_Release(m_video_frame_timer);
    m_video_frame_timer = NULL;
  }
  if(m_MP2TSPacket)
  {
    MM_Free(m_MP2TSPacket);
    m_MP2TSPacket = NULL;
  }

  if(m_MP2PCRTSPacket)
  {
    MM_Free(m_MP2PCRTSPacket);
    m_MP2PCRTSPacket = NULL;
  }

  if(m_MP2PESPacket)
  {
    MM_Free(m_MP2PESPacket);
    m_MP2PESPacket = NULL;
  }
#ifdef _WRITE_TO_FILE
  if(m_filePtr.efs_file)
  {
    OSCL_FileClose(m_filePtr.efs_file);
  }
#endif
  for(int i=0; i < MUX_MAX_MEDIA_STREAMS; i++)
  {
    if(m_pHeader[i])
      MM_Free(m_pHeader[i]);
  }
  if(m_Params)
  {
    if(m_Params->streams)
    {
      MM_Free(m_Params->streams);
    }
    MM_Free(m_Params);
  }
  #ifdef ENABLE_MUX_STATS
  if(m_pStatTimer != NULL)
  {
     MM_Timer_Release(m_pStatTimer);
  }
  m_nDuration = 0;
  #endif /*ENABLE_MUX_STATS*/

}

/*===========================================================================

FUNCTION handle_close

DESCRIPTION
  This function finishes up and closes the movie file being written.  It is
  called once the video encoder has finished stopping, or the movie file is
  closed for any other reason.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  The movie file meta data is written and the file is closed.

===========================================================================*/
void MP2BaseFile::handle_close ()
{
  /* Mark file status as closed. */
  m_output_open = FALSE;
  if(m_video_frame_timer)
  {
    MM_Timer_Release(m_video_frame_timer);
    m_video_frame_timer = NULL;
  }
}

/*===========================================================================

FUNCTION GeneratePATPacket

DESCRIPTION
  Generate TS Program Association table and send to output

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  none

===========================================================================*/
void MP2BaseFile::GeneratePATPacket()
{
  int loop_index = 0;
  int index = 0;
  uint32 nCRC =0;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::GeneratePATPacket");
  m_nTS_Stream_bufsz = 0;
  if(m_MP2TSPacket)
  {
    memset(m_MP2TSPacket, 0, TS_PKT_SIZE);

    // TS stream sync byte  - 8 bits
    m_MP2TSPacket[index] = TS_PKT_SYNC_BYTE;
    index += 1;
    // Transport_error_indicator always set to 0 - 1 bit
    m_MP2TSPacket[index] |= 0X0;
    // Payload start indicator  1 - if the first byte of PSI section - 1 bit
    m_MP2TSPacket[index] |= 0X40;
    // Transport_priority always set to 1 for PSI tables - 1 bit
    m_MP2TSPacket[index] |= 0X20;
    index += 1;
    // PID for PAT - 0X0000  - 13 bits
    m_MP2TSPacket[index] |= 0X00;
    index += 1;
    // Transport scrambling control - 2 bits
    m_MP2TSPacket[index] |= 0X00;
    // Adaptation_field_control - 2 bits - 01
    m_MP2TSPacket[index] |= 0X10;

    // continuty counter 4 bits
    // When multiple PAT is sent continuity counter should be
    // incremented in each packet carrying PAT
    m_MP2TSPacket[index] |= m_MP2TS_table_continuty_counter[0];

    m_MP2TS_table_continuty_counter[0] = (uint8)(m_MP2TS_table_continuty_counter[0] + 1);

    if(m_MP2TS_table_continuty_counter[0] >= 16)
    {
        m_MP2TS_table_continuty_counter[0] = 0;
    }

    index += 1;

    //Pointer - 8 bits
    //our PAT payload always starts immediately
    m_MP2TSPacket[index] = 0;
    index += 1;

    //Program Association Table - PAT
    //table_id - 8 bits always 0 for PAT
    m_MP2TSPacket[index] = 0;
    index += 1;
    // Section Syntax indicator - 1 bit always set to 1
    // 0 - 1bit
    //reserved - 11 - 2 bits
    m_MP2TSPacket[index] |= 0XB0;

    //section_length - 13 bytes - 12 bits
    m_MP2TSPacket[index] |= 0X00;
    index += 1;
    m_MP2TSPacket[index] |= 0X0D;
    index += 1;

    // Transport_stream_id - 0001 - 16 bits  user defined
    m_MP2TSPacket[index] |= 0X00;
    index += 1;
    m_MP2TSPacket[index] |= 0X01;
    index += 1;

    //reserved - 11 - 2 bits
    //version number - 00 - 5 bits
    m_MP2TSPacket[index] |= 0X60;
    //current_next_indicator  - 1 bit
    m_MP2TSPacket[index] |= 0X01;
    index += 1;

    //Section_number - 8 bits - 0
    m_MP2TSPacket[index] |= 0X00;
    index += 1;
    //Last Section_number  - 8 bits - 0
    m_MP2TSPacket[index] |= 0X00;
    index += 1;

    // Program number - 16 bits - 0X0100
    m_MP2TSPacket[index] |= 0X01;
    index += 1;
    m_MP2TSPacket[index] |= 0X00;
    index += 1;

    //Reserved - 111- 3 bits
    //Praogram_map_pid - 13 bits  - 0X0010
    m_MP2TSPacket[index] |= 0XE0;
    m_MP2TSPacket[index] |= 0X01;
    index += 1;
    m_MP2TSPacket[index] |= 0X00;
    index += 1;

    nCRC = FindCheckSum(m_MP2TSPacket + 5, index - 5 );

    m_MP2TSPacket[index] |= static_cast<uint8>(nCRC >> 24);
    index += 1;
    m_MP2TSPacket[index] |= static_cast<uint8>((nCRC >> 16) &0xFF);
    index += 1;
    m_MP2TSPacket[index] |= static_cast<uint8>((nCRC >> 8) & 0XFF);
    index += 1;
    m_MP2TSPacket[index] |= static_cast<uint8>(nCRC & 0xFF);
    index += 1;


    /* Fille the remaining TS packet bytes with FF */
    for( loop_index = index; loop_index < TS_PKT_SIZE; loop_index++)
    {
      m_MP2TSPacket[loop_index] = 0XFF;
    }
    /* Write the transport packet to output */
    if (!Output(m_MP2TSPacket, m_output_offset, TS_PKT_SIZE, true ))
    {
      MM_MSG_PRIO1( MM_GENERAL, MM_PRIO_ERROR,
                 " MP2BaseFile::GeneratePATPacket failed to write data  %d",
                  TS_PKT_SIZE);
      m_nTS_Stream_bufsz = 0;
      return;
    }
    m_nTS_Stream_bufsz = 0;
  }
}
/*===========================================================================

FUNCTION GeneratePMTPacket

DESCRIPTION
  Generate TS Program Map Table and send to output

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  none

===========================================================================*/
void MP2BaseFile::GeneratePMTPacket()
{
  int i;
  int index = 0;
  uint8 section_length = MUX_MP2_PMT_FIX_SIZE;
  uint32 nCRC = 0;

  m_nTS_Stream_bufsz = 0;

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::GeneratePMTPacket");

  if( m_MP2TSPacket )
  {
    memset(m_MP2TSPacket, 0, TS_PKT_SIZE);
    // TS stream sync byte  - 8 bits
    m_MP2TSPacket[index] = TS_PKT_SYNC_BYTE;
    index += 1;
    // Transport_error_indicator always set to 0 - 1 bit
    m_MP2TSPacket[index] |= 0X0;
    // Payload start indicator  1 - if the first byte of PSI section - 1 bit
    m_MP2TSPacket[index] |= 0X40;

    // Transport_priority always set to 1 for PSI tables - 1 bit
    m_MP2TSPacket[index] |= 0X20;
//    index += 1;
    // PID for PMT - 0X0010  - 13 bits
    m_MP2TSPacket[index] |= 0X01;
    index += 1;
    m_MP2TSPacket[index] |= 0X00;
    index += 1;

    // Transport scrambling control - 2 bits
    m_MP2TSPacket[index] |= 0X00;
    // Adaptation_field_control - 01 - 2 bits
    m_MP2TSPacket[index] |= 0X10;

    // continuity_counter - 4 bits
    // When multiple PMT is sent continuity counter should be
    // incremented in each packet carrying PMT
    m_MP2TSPacket[index] |= m_MP2TS_table_continuty_counter[1];

    m_MP2TS_table_continuty_counter[1] = static_cast<uint8>(m_MP2TS_table_continuty_counter[1] + 1);

    if(m_MP2TS_table_continuty_counter[1] >= 16)
    {
      m_MP2TS_table_continuty_counter[1] = 0;
    }

    index += 1;

    //Pointer - 8 bits
    //PMT payload always starts immediately
    m_MP2TSPacket[index] = 0;
    index += 1;

    //Program MAP Table - PMT
    //table_id - 8 bits always 0X02 for PMT
    m_MP2TSPacket[index] = 02;
    index += 1;
    // Section Syntax indicator - 1 bit always set to 1
    // 0 - 1bit
    //reserved - 11- 2 bits
    m_MP2TSPacket[index] |= 0XB0;

    if(m_bVideoPresent)
    {
      // video onlty stream then the section length would be 18
      section_length = static_cast<uint8>(section_length + 5);

      // If video codec is AVC check for any descriptors
      if(m_Params->streams[m_video_stream_num].subinfo.video.format ==
          MUX_STREAM_VIDEO_H264)
      {
        //AVC HRD and TimingInfo Descriptor
        if(m_nAVCHrdDescrLen)
        {
          section_length = static_cast<uint8>(section_length + m_nAVCHrdDescrLen);
        }
      }
    }
    if(m_bAudioPresent)
    {
     // A+V streams then the section length would be 18 + 5 = 23
      section_length = static_cast<uint8>(section_length + 5);
      if(m_Params->streams[m_audio_stream_num].subinfo.audio.format ==
             MUX_STREAM_AUDIO_MPEG4_AAC &&
          m_eAACFormat == AAC_FORMAT_LOAS)
      {
          section_length = static_cast<uint8>(section_length + 3);
      }
      else if(m_Params->streams[m_audio_stream_num].subinfo.audio.format ==
             MUX_STREAM_AUDIO_PCM)
      {
          section_length = static_cast<uint8>(section_length + 4);
      }
      else if ( m_Params->streams[m_audio_stream_num].subinfo.audio.format ==
             MUX_STREAM_AUDIO_AC3)
      {
        if(m_Params->num_channels <=6)
        {
          section_length += 11;
        }
        else
        {
          section_length += 10;
        }
      }
     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "m_bAudioPresent--> section_length ");
    }
     /* If HDCP registration descriptor present, add this to section length */
    if((m_Params->encrypt_param.streamEncrypted == TRUE)  &&
       (m_Params->encrypt_param.type == MUX_ENCRYPT_TYPE_HDCP))
    {
      /* Adding 7 byte header in descriptor section */
      section_length = static_cast<uint8>(section_length + 7);
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "hdcp registration descriptor present--> section_length ");
    }

    //section_length  - 23 bytes - 12 bits
    m_MP2TSPacket[index] |= 0X00;
    index += 1;

    m_MP2TSPacket[index] |= section_length;
    index += 1;


    //m_MP2TSPacket[index] |= 0X17;

    // Program_number
    m_MP2TSPacket[index] |= 0X01;
    index += 1;
    m_MP2TSPacket[index] |= 0X00;
    index += 1;

    //reserved - 11 - 2 bits
    //version number - 00 -  5 bits
    m_MP2TSPacket[index] |= 0X60;
    //current_next_indicator  - 1 bit
    m_MP2TSPacket[index] |= 0X01;
    index += 1;

    //Section_number - 8 bits - 0
    m_MP2TSPacket[index] |= 0X00;
    index += 1;
    //Last Section_number  - 8 bits - 0
    m_MP2TSPacket[index] |= 0X00;
    index += 1;

    // reserved - 111 - 3 bits
    m_MP2TSPacket[index] |= 0XE0;
    // PCR_PID = 0x1fff for private streams which do not put PCR
    uint16 pcr_pid = 0x1000;

    m_MP2TSPacket[index] |= (uint8)(pcr_pid >> 8);
    index += 1;
    m_MP2TSPacket[index] |= (uint8)(pcr_pid & 0xFF);
    index += 1;

    /* If there are any descriptors present add the descriptors and program_info_length */
    if((m_Params->encrypt_param.streamEncrypted == TRUE)  &&
       (m_Params->encrypt_param.type == MUX_ENCRYPT_TYPE_HDCP))
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                    "MP2BaseFile::GeneratePMTPacket() setting the HDCP registration descriptor");
      //Reserved -1111- 4 bits
      //program_info_length - 0 - 12 bits
      m_MP2TSPacket[index] |= 0XF0;
      index += 1;
      m_MP2TSPacket[index] |= 0X07;
      index += 1;
      /* descriptor_tag from the spec */
      m_MP2TSPacket[index] |= 0X05;
      index += 1;
      /* descriptor_length */
      m_MP2TSPacket[index] |= 0X05;
      index += 1;

      /* format_identifier */
      m_MP2TSPacket[index] = 'H';
      index += 1;
      m_MP2TSPacket[index] = 'D';
      index += 1;
      m_MP2TSPacket[index] = 'C';
      index += 1;
      m_MP2TSPacket[index] = 'P';
      index += 1;
      /* HDCP_version */
      m_MP2TSPacket[index] = (uint8)(m_Params->encrypt_param.nEncryptVersion);
      index += 1;
    }
    else /*No descriptors*/
    {
      //Reserved -1111- 4 bits
      //program_info_length - 0 - 12 bits
      m_MP2TSPacket[index] |= 0XF0;
      index += 1;
      m_MP2TSPacket[index] |= 0X00;
      index += 1;
    }

    //Video
    if(m_bVideoPresent)
    {
      //stream_type - for AVC
      m_MP2TSPacket[index] |= 0X1B;
      index += 1;
      // reserved -111- 3 bits
      //elementary PID - 13 bits - 0x1011 for video
      m_MP2TSPacket[index] |= 0XF0;
      index += 1;
      m_MP2TSPacket[index] |= 0X11;
      index += 1;

      if(m_pAVCHrdDescr && m_nAVCHrdDescrLen)
      {
        //reserved -1111- 4 bits
        //ES_info_length - 12 bits
        m_MP2TSPacket[index] |= 0XF0;
        index += 1;
        m_MP2TSPacket[index] |= m_nAVCHrdDescrLen;
        index += 1;

        //Copy descriptors to packet
        memcpy(m_MP2TSPacket + index,
               m_pAVCHrdDescr,
               m_nAVCHrdDescrLen);

        index += m_nAVCHrdDescrLen;
      }
      else
      {
        //reserved -1111- 4 bits
        // ES_info_length - 12 bits
        m_MP2TSPacket[index] |= 0XF0;
        index += 1;
        m_MP2TSPacket[index] |= 0X00;
        index += 1;
      }
    }
    // Audio
    if(m_bAudioPresent)
    {
      //stream_type -
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "m_eAACFormat = %d",m_eAACFormat);
      if(m_Params->streams[m_audio_stream_num].subinfo.audio.format == MUX_STREAM_AUDIO_MPEG4_AAC)
      {
        if(m_eAACFormat == AAC_FORMAT_ADTS)
        {
           m_MP2TSPacket[index] |= 0x0F;
        }
        else
        {
           m_MP2TSPacket[index] |= 0x11;
        }
      }
      else if(m_Params->streams[m_audio_stream_num].subinfo.audio.format == MUX_STREAM_AUDIO_AC3)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, " AC3 AUDIO ");
       // 0x06 for System B DVB
       // m_MP2TSPacket[index] |= 0X06;
       // 0x81 for System A (ATSC)
        if(m_Params->num_channels > 6)
        {
          m_MP2TSPacket[index] |= 0XCC; //EAC3
        }
        else
        {
          m_MP2TSPacket[index] |= 0X81; //AC3
        }
      }
     else
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "PCM samples ");
        m_MP2TSPacket[index] |= 0X83;
      }
      index += 1;
      // reserved -111- 3 bits
      //elementary PID - 13 bits - 0x1100 to 0x111F for Audio
      // Revisit when add another audo stream
      m_MP2TSPacket[index] |= 0XF1;
      index += 1;
      m_MP2TSPacket[index] |= 0X00;
      index += 1;
      //reserved -1111- 4 bits
      // ES_info_length - 12 bits
      if(m_Params->streams[m_audio_stream_num].subinfo.audio.format == MUX_STREAM_AUDIO_MPEG4_AAC &&
         m_eAACFormat == AAC_FORMAT_LOAS)
      {
        m_MP2TSPacket[index] |= 0XF0;
        index += 1;
        m_MP2TSPacket[index] |= 0x3; //ES info len mpeg-4_audio_descriptor size
        index += 1;
        m_MP2TSPacket[index] |= 28; //mpeg-4_audio_descriptor_tag
        index += 1;
        m_MP2TSPacket[index] |= 0x1; //mpeg-4_audio_descriptor_length
        index += 1;
        m_MP2TSPacket[index] |= 0x58; //profilelevel
        index += 1;
      }
      else if(m_Params->streams[m_audio_stream_num].subinfo.audio.format == MUX_STREAM_AUDIO_PCM)
      {
        m_MP2TSPacket[index] |= 0XF0;
        index += 1;
        m_MP2TSPacket[index] |= 0X04;
        index += 1;
        m_MP2TSPacket[index] |= 0x83; //PCM descriptor
        index += 1;
        m_MP2TSPacket[index] |= 0x2; //01000110
        index += 1;
        if(m_Params->sampling_rate == 48000)
        {
          m_MP2TSPacket[index] |= 0x46; //sampling rate and bits per sample
        }
        else
        {
          m_MP2TSPacket[index] |= 0x26;
        }
        index += 1;
        m_MP2TSPacket[index] |= 0x3f; //num channels(001) and reserved(11111b)
        index += 1;
      }
      else if(m_Params->streams[m_audio_stream_num].subinfo.audio.format == MUX_STREAM_AUDIO_AC3 &&
        m_Params->num_channels <= 6)
      {
        m_MP2TSPacket[index] |= 0XF0;
        index += 1;
        m_MP2TSPacket[index] |= 0xB;
        index += 1;
        m_MP2TSPacket[index] |= 0x05; //AC3 Registration_descriptor
        index += 1;
        m_MP2TSPacket[index] |= 0x04; //descriptor length
        index += 1;
        m_MP2TSPacket[index] |= 0x41; //The AC-3 format_identifier is 0×41432D33 ("AC-3").
        index += 1;
        m_MP2TSPacket[index] |= 0x43;
        index += 1;
        m_MP2TSPacket[index] |= 0x2D;
        index += 1;
        m_MP2TSPacket[index] |= 0x33;
        index += 1;
        //AC-3 audio_descriptor for System - A
        m_MP2TSPacket[index] |= 0x81; //descriptor tag
        index += 1;
        m_MP2TSPacket[index] |= 0x3; //descriptor length
        index += 1;
        // sample_rate_code = 000 and bsid - 01000
        m_MP2TSPacket[index] |= 0x08;
        index += 1;
        // bit_rate_code = 110010 - max 640 KBPS
        // surmod - "00" not indicated
        m_MP2TSPacket[index] |= 0xC8 ;
        index += 1;

        // bsmod = 000
        // num_channels - 1101
        // fullsvc = 1
        m_MP2TSPacket[index] |= 0x1B;
        index += 1;
      }
      else if(m_Params->streams[m_audio_stream_num].subinfo.audio.format == MUX_STREAM_AUDIO_AC3 &&
        m_Params->num_channels > 6 /*EAC3*/)
      {
        m_MP2TSPacket[index] |= 0XF0;
        index += 1;
        m_MP2TSPacket[index] |= 0xA;
        index += 1;
        m_MP2TSPacket[index] |= 0x05; //EAC3 Registration_descriptor
        index += 1;
        m_MP2TSPacket[index] |= 0x04; //descriptor length
        index += 1;
        m_MP2TSPacket[index] |= 0x45; //The AC-3 format_identifier is 0×45414333("EAC3").
        index += 1;
        m_MP2TSPacket[index] |= 0x41;
        index += 1;
        m_MP2TSPacket[index] |= 0x43;
        index += 1;
        m_MP2TSPacket[index] |= 0x33;
        index += 1;
        //EAC-3 audio_descriptor for System - A
        m_MP2TSPacket[index] |= 0xCC; //descriptor tag
        index += 1;
        m_MP2TSPacket[index] |= 0x02; //descriptor length
        index += 1;

        //reserved - 1 bit
        //bsid flag - 1bit
        //mainid_flag - 1bit
        //asvc_flag - 1bit
        //misinfoexists - 1bit
        //substream1_flag - 1bit
        //substream2_flag - 1bit
        //substream3_flag - 1bit
        m_MP2TSPacket[index] |= 0x80;
        index += 1;

        //reserved  - 1bit
        //full_service_flag - 1bit
        //audio_service_type - 3bits
        //number-of_channels - 3bits
        m_MP2TSPacket[index] |= 0xc5;
        index += 1;
      }
      else
      {
        m_MP2TSPacket[index] |= 0XF0;
        index += 1;
        m_MP2TSPacket[index] |= 0X00;
        index += 1;
      }
    }

    nCRC = FindCheckSum(m_MP2TSPacket + 5, index - 5);

    m_MP2TSPacket[index] |= static_cast<uint8>(nCRC >> 24);
    index += 1;
    m_MP2TSPacket[index] |= static_cast<uint8>((nCRC >> 16) &0xFF);
    index += 1;
    m_MP2TSPacket[index] |= static_cast<uint8>((nCRC >> 8) & 0XFF);
    index += 1;
    m_MP2TSPacket[index] |= static_cast<uint8>((nCRC & 0xFF));
    index += 1;


    /* Fille the remaining TS packet bytes with FF */
    for( i=index; i<TS_PKT_SIZE; i++)
    {
      m_MP2TSPacket[i] = 0XFF;
    }
    /* Write the transport packet to output */
    if (!Output(m_MP2TSPacket, m_output_offset, TS_PKT_SIZE, true ))
    {
      MM_MSG_PRIO1( MM_GENERAL, MM_PRIO_ERROR,
                            " MP2BaseFile::GeneratePMTPacket failed to write data  %d",
                            TS_PKT_SIZE);
      m_nTS_Stream_bufsz = 0;
      return;
    }
    m_nTS_Stream_bufsz = 0;
  }
}



/*!
   @brief     This function is used to update AVC Timing and HRD params.

   @detail    Params used to populate the Timing and HRD descriptor in
              AVC is set using this function.

   @param[in] pHRDParams - pointer to Timing and HRD params structure.

   @return    MUX_STATUS.
*/
MUX_STATUS MP2BaseFile::MUX_update_AVC_Timing_HRD_Params
(
  MUX_AVC_TimingHRD_params_type *pHRDParams
)
{
  if(!pHRDParams)
  {
      return MUX_FAIL;
  }

  memcpy(&m_sHRDParams, pHRDParams, sizeof(m_sHRDParams));

  (void)GenerateAVCHRDTimingDescriptor(pHRDParams);

  return MUX_SUCCESS;
}

/*!
   @brief     This function is used to generate AVC HRD params.

   @detail    HRD params used to generate the HRD descriptor to be sent in PMT.

   @param[in] pHRDParams - pointer to HRD params structure.

   @return    MUX_STATUS.
*/
bool MP2BaseFile::GenerateAVCHRDTimingDescriptor
(
  MUX_AVC_TimingHRD_params_type *pAVCHrdParams
)
{
  int index = 0;

  if(!m_pAVCHrdDescr)
  {
    //If descriptor Buffer is not already available malloc here
    m_pAVCHrdDescr = (uint8*)MM_Malloc(MUX_MP2_AVC_HRD_DESC_SIZE);

    if(!m_pAVCHrdDescr)
    {
      return false;
    }
  }

  //Initialize to 0 and create new descriptor
  memset(m_pAVCHrdDescr, 0, MUX_MP2_AVC_HRD_DESC_SIZE);

  //Size needs to be updated after generating the descriptor
  m_nAVCHrdDescrLen = MUX_MP2_AVC_HRD_DESC_SIZE;

  //DescriptorTag  8bits.
  m_pAVCHrdDescr[index] = 42;
  index++;

  //Descriptor Length  8bits  Filled Later
  m_pAVCHrdDescr[index] = 0;
  index++;

  //hrd_management_valid_flag  1bit
  if(pAVCHrdParams->bHrdManagementValid)
  {
    m_pAVCHrdDescr[index] = 0x80;
  }

  //reserved  6bits  Fill 1s
  m_pAVCHrdDescr[index] |= 0x7E;

  //picture_and_timing_info_present 1bit
  if(pAVCHrdParams->bPictureAndTimingInfo)
  {
    m_pAVCHrdDescr[index] |= 0x1;
  }
  index++;

  if(pAVCHrdParams->bPictureAndTimingInfo)
  {
    //90kHz_flag  1bit
    if(pAVCHrdParams->nAVCTimeScale == 90000)
    {
      m_pAVCHrdDescr[index] = 0x80;
    }

    //reserved  7bits    Fill 1s
    m_pAVCHrdDescr[index]  |= 0x7f;
    index++;

    if(pAVCHrdParams->nAVCTimeScale != 90000)
    {
      //N       32bits
      //K       32bits
      uint32 N = 0;
      uint32 K = 0;
      //N and K are derived from the equation
      //
      //time_scale = (N * system_clock_frequency)/ K
      //
      //where system_clock_frequency = 27MHz for TS

      if((uint32)27000000 % pAVCHrdParams->nAVCTimeScale)
      {
        N = pAVCHrdParams->nAVCTimeScale;
        K = (uint32)27000000;
      }
      else
      {
        N = 1;
        K = (uint32)27000000 / pAVCHrdParams->nAVCTimeScale;
      }

      //Update N
      m_pAVCHrdDescr[index] = (uint8)(N >> 24);
      index++;
      m_pAVCHrdDescr[index] = (uint8)(N >> 16);
      index++;
      m_pAVCHrdDescr[index] = (uint8)(N >>  8);
      index++;
      m_pAVCHrdDescr[index] = (uint8)(N      );
      index++;

      //Update K
      m_pAVCHrdDescr[index] = (uint8)(K >> 24);
      index++;
      m_pAVCHrdDescr[index] = (uint8)(K >> 16);
      index++;
      m_pAVCHrdDescr[index] = (uint8)(K >>  8);
      index++;
      m_pAVCHrdDescr[index] = (uint8)(K      );
      index++;

    }

    //num_units_in_tick  32bits
    uint32 nNumTicks = pAVCHrdParams->nNumUnitsInTick;
    m_pAVCHrdDescr[index] = (uint8)(nNumTicks >> 24);
    index++;
    m_pAVCHrdDescr[index] = (uint8)(nNumTicks >> 16);
    index++;
    m_pAVCHrdDescr[index] = (uint8)(nNumTicks >>  8);
    index++;
    m_pAVCHrdDescr[index] = (uint8)(nNumTicks      );
    index++;
  }

  //fixed_frame_rate_flag  1bit
  if(pAVCHrdParams->bFixedFrameRate)
  {
    m_pAVCHrdDescr[index] = 0x80;
  }

  //temporal_poc_flag  1bit
  if(pAVCHrdParams->bTemporalPocFlag)
  {
    m_pAVCHrdDescr[index] |= 0x40;
  }

  //When the temporal_poc_flag is set to '1' and the fixed_frame_rate_flag
  //is set to '1', then the associated AVC video stream shall carry
  //Picture Order Count (POC) information

  //picture_to_display_conversion_flag  1bit
  if(pAVCHrdParams->bPictureToDisplayConversionFlag)
  {
    m_pAVCHrdDescr[index] |= 0x20;
  }

  //reserved 5bits-Fill 1s
   m_pAVCHrdDescr[index] |= 0x1F;
   index++;

  //Descriptor Length. Num of bytes following descriptor Length field
  m_pAVCHrdDescr[1] = static_cast<uint8>(index - 2);

  m_nAVCHrdDescrLen = static_cast<uint8>(index);

  return true;
}

/*===========================================================================

FUNCTION GenerateMP2TSVideoPacket

DESCRIPTION
  Generate TS video packets for video frames and send to output

DEPENDENCIES
  None.

RETURN VALUE
  no of bytes consumed from the original sample

SIDE EFFECTS
  none

===========================================================================*/
uint32 MP2BaseFile::GenerateMP2TSVideoPacket( uint32   stream_number,
                                             MUX_sample_info_type  *sample_info,
                                             const uint8  *PES_Header,
                                             const uint16 PES_Header_Size,
                                             uint8  *sample_data,
                                             const uint32 offset_sample_data)
{
  int index = 0;
  uint8 sample_data_offset = 0;
  uint8 bytes_to_copy = 0;
  bool  bputPCR = false;

  //MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::GenerateMP2TSVideoPacket");

  if(m_MP2TSPacket)
  {
    memset(m_MP2TSPacket, 0, TS_PKT_SIZE);
    // TS stream sync byte  - 8 bits
    m_MP2TSPacket[index] = TS_PKT_SYNC_BYTE;
    index += 1;
    // Transport_error_indicator always set to 0 - 1 bit
    m_MP2TSPacket[index] |= 0X0;
    if(!m_PSI_payload_start_indicator_set)
    {
      // Payload start indicator  1 - always set for the first TS packet of the each video frame
      m_MP2TSPacket[index] |= 0X40;
      m_PSI_payload_start_indicator_set = 1;
      if(m_Params->streams[stream_number].type == MUX_STREAM_VIDEO)
      {
        bputPCR = false;
      }
    }
    else
    {
      /* Payload start indicator  0 - for all the TS packet of the each
       * video frame other then first TS packet.
       */
      m_MP2TSPacket[index] |= 0X00;
    }
    // Transport_priority always set to 1 for I frames and 0 for all other freames - 1 bit
    if(m_Transport_priority)
    {
      m_MP2TSPacket[index] |= 0X20;
    }

    // PID for Video - 0X1011  - 13 bits
    m_MP2TSPacket[index] |= 0X10;
    index += 1;
    m_MP2TSPacket[index] |= 0X11;
    index += 1;
    // Transport scrambling control - 00 - 2 bits
    m_MP2TSPacket[index] |= 0X00;
    if(m_adaptation_field < 4)
    {
      uint8 nAdaptationControl = m_adaptation_field;

      if(!nAdaptationControl)
      {
         nAdaptationControl = MUX_MP2_NO_ADAPTATION_PAYLOAD_ONLY;
      }
      // Adaptation_field_control - 2 bits
      m_MP2TSPacket[index] |= static_cast<uint8>(nAdaptationControl << 4);
    }

    m_MP2TSPacket[index] |= m_MP2TS_continuty_counter[stream_number];
    m_MP2TS_continuty_counter[stream_number] = static_cast<uint8>((m_MP2TS_continuty_counter[stream_number] + 1) % 16);
    index += 1;

    //Adaptation field
    if(m_adaptation_field == MUX_MP2_ADAPTATON_AND_PAYLOAD)
    {
      // adaptation_field_length  - 8 bits
      if(bputPCR)
          m_MP2TSPacket[index] = 0X07;
      else
          m_MP2TSPacket[index] = 0X01;
      index += 1;
      //Discontinuity indicator - 1 bit - 0
      m_MP2TSPacket[index] |= 0X00;
      //random_access_indicator  - 1-bit - 1 for I-Frames, 0 for rest of the frames
      //elementary_stream_priority_indicator - 1-bit - 1 for I-Frames, 0 for rest of the frames
      if(m_randon_access_indicator)
      {
        m_MP2TSPacket[index] |= 0X40;
        m_MP2TSPacket[index] |= 0X20;
      }
      //PCR_flag  - 1 bit  - 0
      //OPCR_flag - 1 bit  - 0
      //splicing_point_flag - 1 bit - 0
      //transport_private_data_flag - 1 bit - 0
      // adaptation_field_extension_flag - 1 bit  - 0
      if(bputPCR)
      m_MP2TSPacket[index] |= 0X10;
      else
      m_MP2TSPacket[index] |= 0X00;

      index += 1;
      if(bputPCR)
      {

          uint64 PCR_base =  (uint64)sample_info->time * 90;//ms to 90KHz conversion
          PCR_base -= m_nBaseTimestamp;

          if((int64)(PCR_base - m_nCurrPCR) <= 0)
          {
             // If PCRBase goes behing current PCR, Stop it at current PCr
             // This will only happen if video timestamp goes out of order
             // which will not happen as long as B frames are not supported
             // Revisit for B frame.
             PCR_base = m_nCurrPCR;
          }

          uint64 PCR = PCR_base * 300;

          m_nCurrPCR = PCR_base;


          m_MP2TSPacket[index] |= (uint8)((PCR_base >> 25) & 0xff);
          index += 1;
          m_MP2TSPacket[index] |= (uint8)((PCR_base >> 17) & 0xff);
          index += 1;
          m_MP2TSPacket[index] |= (uint8)((PCR_base >> 9) & 0xff);
          index += 1;
          m_MP2TSPacket[index] |= (uint8)((PCR_base >> 1) & 0xff);
          index += 1;
          m_MP2TSPacket[index] |= (uint8)((PCR_base << 7) & 0x80);
          index += 1;
          index += 1;
      }
    }

    /* Copy the remaining bytes from the sample data to the TS packet */
    if(index < TS_PKT_SIZE)
    {

          /* Copy PES header into the TS packet. PES will be there for the first packet of each frame */

      uint8 loop_index = 0;
      if(PES_Header_Size >= 183 || !sample_info->size)
      {
          //If PES header itself takes a whole TS packet, we cannot send any video frame bytes here.
        bytes_to_copy = 0;
      }
      else
      {
        bytes_to_copy =  (uint8)(MIN(   ((uint32)(sample_info->size - offset_sample_data )) ,
                                           ((uint32) (TS_PKT_SIZE - index - PES_Header_Size))));
      }


      if(sample_info->extra_data_size != 0)
      {
        //HDCP mandates that each payload in an MPEG TS packet for video
        // must be a multiple of 16 bytes except for the last TS packet
        // carrying the end of the PES
        if((bytes_to_copy > 16) && (bytes_to_copy % 16 != 0))
        {
          bytes_to_copy = static_cast<uint8>(bytes_to_copy - (bytes_to_copy % 16));
        }
      }

      sample_data_offset = bytes_to_copy;

      bytes_to_copy = static_cast<uint8>(bytes_to_copy + PES_Header_Size);

      if(!bytes_to_copy)
      {
        MM_MSG_PRIO1( MM_GENERAL, MM_PRIO_ERROR,
                      " MP2BaseFile::GenerateMP2TSVideoPacket bytes_to_copy %d no need to create any TS packet",
                              (int)bytes_to_copy);
        return bytes_to_copy;
      }
      /* This is special case where we have 183 bytes to copy and just we need to
       * stuff only one byte becasue 183 + 4(TS header size), then we need to put
       * the adaptation field as 00 and put one stuffing byte FF
       */
      if( bytes_to_copy == 183)
      {
        // Adaptation_field_control - 00 -  2 bits
        m_MP2TSPacket[3] |= (MUX_MP2_ADAPTATON_AND_PAYLOAD << 4);
        /* We need to add only one stuffing byte */
        m_MP2TSPacket[4] = 0X0;
        /* Increment the index by 1 */
        index += 1;
      }
      else if((index + bytes_to_copy) < TS_PKT_SIZE )
      {
        // 2 bytes - 1 byte adaptation_field_length, 1 byte adaptation header
        uint8 bytes_to_stuff;
        if(m_MP2TSPacket[4])
        {
           bytes_to_stuff = static_cast<uint8>(TS_PKT_SIZE - (index + bytes_to_copy));
        }
        else
        {
           bytes_to_stuff = static_cast<uint8>(TS_PKT_SIZE - (index + bytes_to_copy + 2));
        }


        // Adaptation_field_control - 2 bits
        m_MP2TSPacket[3] |= (MUX_MP2_ADAPTATON_AND_PAYLOAD << 4);
        /* Add bytes to stuff to adaptation header */
        if(m_MP2TSPacket[4])
        {
          m_MP2TSPacket[4] = static_cast<uint8>(m_MP2TSPacket[4] + (bytes_to_stuff));
        }
        else
        {
          m_MP2TSPacket[4] = static_cast<uint8>(m_MP2TSPacket[4] + (bytes_to_stuff+1));
          /* Adaptation header  1- Byte */
          m_MP2TSPacket[5] |= 0X00;
          /* Increment the index by 2 */
          index += 2;
        }


        if(bytes_to_stuff < TS_PKT_SIZE)
        {
          /* stuff bytes_to_stuff no. of bytes */
          for( loop_index = (uint8)index; loop_index < (index + bytes_to_stuff); loop_index++)
          {
            m_MP2TSPacket[loop_index] = 0XFF;
          }
         index += bytes_to_stuff;
        }
      }
    }
    /* Copy PES header into the TS packet. PES will be there for the first packet of each frame */
    if(PES_Header_Size && PES_Header)
    {
        memcpy((void *)(m_MP2TSPacket +index), PES_Header, PES_Header_Size);
        index += PES_Header_Size;
        bytes_to_copy = static_cast<uint8>(bytes_to_copy - PES_Header_Size);
    }
    memcpy((void *)(m_MP2TSPacket +index), (sample_data + offset_sample_data), bytes_to_copy);

    bool bEOF = false;
    if(offset_sample_data + sample_data_offset >= sample_info->size)
    {
       bEOF = true;
    }
    if (!Output(m_MP2TSPacket, m_output_offset, TS_PKT_SIZE, bEOF ))
    {
      MM_MSG_PRIO1( MM_GENERAL, MM_PRIO_ERROR,
                          " MP2BaseFile::GenerateMP2TSVideoPacket failed to write data  %d",
                          TS_PKT_SIZE);

      return 0;
    }
  }
  return sample_data_offset;
}

/*===========================================================================

FUNCTION GenerateMP2TSAudioPacket

DESCRIPTION
  Generate TS audio packets for audio frames and send to output

DEPENDENCIES
  None.

RETURN VALUE
  no of bytes consumed from the original sample

SIDE EFFECTS
  none

===========================================================================*/
uint32 MP2BaseFile::GenerateMP2TSAudioPacket( uint32   stream_number,
                                            MUX_sample_info_type  *sample_info,
                                            const uint8  *PES_Header,
                                            const uint16 PES_Header_Size,
                                            uint8  *sample_data,
                                            const uint32 offset_sample_data)
{
  int index = 0;
  uint8 sample_data_offset = 0;
  uint8 bytes_to_copy = 0;
  bool  bputPCR = false;
  uint32 n_data_len = (sample_info->size - offset_sample_data);
  uint32 n_stuff_len = 0;
  uint32 n_align_len = m_Params->streams[stream_number].subinfo.audio.format ==
                       MUX_STREAM_AUDIO_PCM ? 4 : 0;
  if(sample_info->extra_data_size)
  {
    n_align_len = MAX(n_align_len , 16);
  }
 // MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::GenerateMP2TSVideoPacket");

  if(m_MP2TSPacket)
  {
    memset(m_MP2TSPacket, 0, TS_PKT_SIZE);
    // TS stream sync byte  - 8 bits
    m_MP2TSPacket[index] = TS_PKT_SYNC_BYTE;
    index += 1;
    // Transport_error_indicator always set to 0 - 1 bit
    m_MP2TSPacket[index] |= 0X0;
    if(!m_PSI_payload_start_indicator_set)
    {
      // Payload start indicator  1 - always set for the first TS packet of the each video frame
      m_MP2TSPacket[index] |= 0X40;
      m_PSI_payload_start_indicator_set = 1;
      bputPCR = false;
    }
    else
    {
      /* Payload start indicator  0 - for all the TS packet of the each
       * video frame other then first TS packet.
       */
      m_MP2TSPacket[index] |= 0X00;
    }

    // PID for Audio - 0X1100  - 13 bits
    m_MP2TSPacket[index] |= 0X11;
    index += 1;
    m_MP2TSPacket[index] |= 0X00;
    index += 1;
    // Transport scrambling control - 00 - 2 bits
    m_MP2TSPacket[index] |= 0X00;
    if(m_adaptation_field < 4)
    {
      uint8 nAdaptationControl = m_adaptation_field;

      if(!nAdaptationControl)
      {
         nAdaptationControl = MUX_MP2_NO_ADAPTATION_PAYLOAD_ONLY;
      }
      // Adaptation_field_control - 2 bits
      m_MP2TSPacket[index] |= static_cast<uint8>(nAdaptationControl << 4);
    }

    m_MP2TSPacket[index] |= m_MP2TS_continuty_counter[stream_number];
    m_MP2TS_continuty_counter[stream_number] = static_cast<uint8>
          ((m_MP2TS_continuty_counter[stream_number] + 1) % 16);
    index += 1;

    //Adaptation field
    if(m_adaptation_field == MUX_MP2_ADAPTATON_AND_PAYLOAD)
    {
      // adaptation_field_length  - 8 bits
      if(bputPCR)
          m_MP2TSPacket[index] = 0X07;
      else
          m_MP2TSPacket[index] = 0X01;
      index += 1;
      //Discontinuity indicator - 1 bit - 0
      m_MP2TSPacket[index] |= 0X00;
      //random_access_indicator  - 1-bit - 1 for I-Frames, 0 for rest of the frames
      //elementary_stream_priority_indicator - 1-bit - 1 for I-Frames, 0 for rest of the frames
      if(m_randon_access_indicator)
      {
        m_MP2TSPacket[index] |= 0X40;
        m_MP2TSPacket[index] |= 0X20;
      }
      //PCR_flag  - 1 bit  - 0
      //OPCR_flag - 1 bit  - 0
      //splicing_point_flag - 1 bit - 0
      //transport_private_data_flag - 1 bit - 0
      // adaptation_field_extension_flag - 1 bit  - 0
      if(bputPCR)
      m_MP2TSPacket[index] |= 0X10;
      else
      m_MP2TSPacket[index] |= 0X00;

      index += 1;
      if(bputPCR)
      {
          uint64 PCR = (sample_info->time + 33) * 27000;
          uint64 PCR_base = (sample_info->time + 33) * 27000 /300;
          m_MP2TSPacket[index] |= (uint8)((PCR_base >> 25) & 0xff);
          index += 1;
          m_MP2TSPacket[index] |= (uint8)((PCR_base >> 17) & 0xff);
          index += 1;
          m_MP2TSPacket[index] |= (uint8)((PCR_base >> 9) & 0xff);
          index += 1;
          m_MP2TSPacket[index] |= (uint8)((PCR_base >> 1) & 0xff);
          index += 1;
          m_MP2TSPacket[index] |= (uint8)((PCR_base << 7) & 0x80);
          index += 1;
          index += 1;
      }
    }

    bytes_to_copy = static_cast<uint8>(TS_PKT_SIZE - index);
    //Get Adaptation field length for stuffing,

    if( PES_Header_Size + index + n_data_len < TS_PKT_SIZE )
    {
        n_stuff_len = TS_PKT_SIZE - (PES_Header_Size + index + n_data_len);

        if((m_MP2TSPacket[3] & 0x30) == (MUX_MP2_NO_ADAPTATION_PAYLOAD_ONLY << 4))
        {
            // Adaptation_field_control - 00 -  2 bits
            m_MP2TSPacket[3] |= (MUX_MP2_ADAPTATON_AND_PAYLOAD << 4);
            /* We need to add only one stuffing byte */
            m_MP2TSPacket[4] = (uint8)(n_stuff_len -1);
            /* Increment the index by 1 */
            index += 1;
            if(n_stuff_len  > 1)
            {
                n_stuff_len -= 1; //Leave space for adaptation field
                m_MP2TSPacket[5] = 0;
                index += 1;
            }
            n_stuff_len -= 1;
        }
        else
        {
             m_MP2TSPacket[4] = (uint8)(m_MP2TSPacket[4] + n_stuff_len);
        }
        if(n_stuff_len)
        {
            memset((void*) (m_MP2TSPacket + index), 0xFF, n_stuff_len);
            index += (uint8)n_stuff_len;
        }
        /* Copy PES header into the TS packet. PES will be there for the first packet of each frame */
        if(PES_Header_Size && PES_Header && (PES_Header_Size < (TS_PKT_SIZE - index)))
        {
            memcpy((void *)(m_MP2TSPacket +index), PES_Header, PES_Header_Size);
            index += PES_Header_Size;
        }
        bytes_to_copy = static_cast<uint8>(TS_PKT_SIZE - index);
    }
    else if(n_align_len && ((TS_PKT_SIZE - (PES_Header_Size + index)) % n_align_len))
    {
        n_stuff_len = (TS_PKT_SIZE - (PES_Header_Size + index)) % n_align_len;

        if((m_MP2TSPacket[3] & 0x30) == (MUX_MP2_NO_ADAPTATION_PAYLOAD_ONLY << 4))
        {
            // Adaptation_field_control - 00 -  2 bits
            m_MP2TSPacket[3] |= (MUX_MP2_ADAPTATON_AND_PAYLOAD << 4);
            /* We need to add only one stuffing byte */
            m_MP2TSPacket[4] = (uint8)(n_stuff_len -1);
            /* Increment the index by 1 */
            index += 1;
            if(n_stuff_len  > 1)
            {
                n_stuff_len -= 1; //Leave space for adaptation field
                m_MP2TSPacket[5] = 0;
                index += 1;
            }
            n_stuff_len -= 1;
        }
        else
        {
             m_MP2TSPacket[4] = static_cast<uint8>(m_MP2TSPacket[4] + n_stuff_len);
        }
        if(n_stuff_len)
        {
            memset((void*) (m_MP2TSPacket + index), 0xFF, n_stuff_len);
            index += (uint8)n_stuff_len;
        }
        /* Copy PES header into the TS packet. PES will be there for the first packet of each frame */
        if(PES_Header_Size && PES_Header && (PES_Header_Size < (TS_PKT_SIZE - index)))
        {
            memcpy((void *)(m_MP2TSPacket +index), PES_Header, PES_Header_Size);
            index += PES_Header_Size;
        }
        bytes_to_copy = static_cast<uint8>(TS_PKT_SIZE - index);
    }
    else if(PES_Header_Size && PES_Header && (PES_Header_Size < (TS_PKT_SIZE - index)))
    {
        memcpy((void *)(m_MP2TSPacket +index), PES_Header, PES_Header_Size);
        index += PES_Header_Size;
        bytes_to_copy = static_cast<uint8>(TS_PKT_SIZE - index);
    }
    memcpy((void *)(m_MP2TSPacket +index), (sample_data + offset_sample_data), bytes_to_copy);
    bool bEOF = false;
    if(offset_sample_data + bytes_to_copy >= sample_info->size)
    {
       bEOF = true;
    }
    if (!Output(m_MP2TSPacket, m_output_offset, TS_PKT_SIZE, bEOF ))
    {
      MM_MSG_PRIO1( MM_GENERAL, MM_PRIO_ERROR,
                          " MP2BaseFile::GenerateMP2TSVideoPacket failed to write data  %d",
                          TS_PKT_SIZE);

      return 0;
    }
  }
  return bytes_to_copy;

}

/*===========================================================================

FUNCTION GenerateMP2TSVideoPacket

DESCRIPTION
  Generate TS video packets for video frames and send to output

DEPENDENCIES
  None.

RETURN VALUE
  no of bytes consumed from the original sample

SIDE EFFECTS
  none

===========================================================================*/
uint32 MP2BaseFile::GenerateMP2TSPCRPacket( uint32   stream_number,
                                             MUX_sample_info_type  *sample_info)
{
  int index = 0;
  uint8 sample_data_offset = 0;
  uint8 bytes_to_copy = 0;
  bool  bputPCR = true;

  //MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::GenerateMP2TSVideoPacket");

  if(m_MP2PCRTSPacket)
  {
    memset(m_MP2PCRTSPacket, 0, TS_PKT_SIZE);
    // TS stream sync byte  - 8 bits
    m_MP2PCRTSPacket[index] = TS_PKT_SYNC_BYTE;
    index += 1;
    // Transport_error_indicator always set to 0 - 1 bit
    m_MP2PCRTSPacket[index] |= 0X0;

    m_MP2PCRTSPacket[index] |= 0X40;
    bputPCR = true;

    // Transport_priority always set to 1 for I frames and 0 for all other freames - 1 bit
    if(1)
    {
      m_MP2PCRTSPacket[index] |= 0X20;
    }

    // PID for Video - 0X1011  - 13 bits
    m_MP2PCRTSPacket[index] |= 0X10;
    index += 1;
    m_MP2PCRTSPacket[index] |= 0X00;
    index += 1;
    // Transport scrambling control - 00 - 2 bits
    m_MP2PCRTSPacket[index] |= 0X00;
    if(m_adaptation_field < 4)
    {
      uint8 nAdaptationControl = 0x3;

      // Adaptation_field_control - 2 bits
      m_MP2PCRTSPacket[index] |= static_cast<uint8>(nAdaptationControl << 4);
    }

    m_MP2PCRTSPacket[index] |= m_MP2TS_continuty_counter[stream_number];
    m_MP2TS_continuty_counter[stream_number] = static_cast<uint8>((m_MP2TS_continuty_counter[stream_number] + 1) % 16);
    index += 1;

    //Adaptation field
    if(1)
    {
      // adaptation_field_length  - 8 bits
      if(bputPCR)
          m_MP2PCRTSPacket[index] = 0X07;
      else
          m_MP2PCRTSPacket[index] = 0X01;
      index += 1;
      //Discontinuity indicator - 1 bit - 0
      m_MP2PCRTSPacket[index] |= 0X00;
      //random_access_indicator  - 1-bit - 1 for I-Frames, 0 for rest of the frames
      //elementary_stream_priority_indicator - 1-bit - 1 for I-Frames, 0 for rest of the frames
      if(m_randon_access_indicator)
      {
        m_MP2PCRTSPacket[index] |= 0X40;
        m_MP2PCRTSPacket[index] |= 0X20;
      }
      //PCR_flag  - 1 bit  - 0
      //OPCR_flag - 1 bit  - 0
      //splicing_point_flag - 1 bit - 0
      //transport_private_data_flag - 1 bit - 0
      // adaptation_field_extension_flag - 1 bit  - 0
      if(bputPCR)
      m_MP2PCRTSPacket[index] |= 0X10;
      else
      m_MP2PCRTSPacket[index] |= 0X00;

      index += 1;
      if(bputPCR)
      {
          // Note that for PCR sample info comes in this format. sample_info->time in
          // seconds and sampleinfo->delta in nano seconds

          uint64 PCR_base = (uint64)sample_info->time * 90000 +  //sec to 90Khz
                            (uint64)(((uint64)sample_info->delta/1000) * 90) / 1000;


          uint64 PCR_27MHz = (uint64)((uint64)sample_info->delta * 27)/1000;

          uint64 PCR_Ext = (uint64)PCR_27MHz % 300;

          MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_MEDIUM,
                       "PCR Incoming sec  %u nsec %u 27Mhz %u Ext %u",
                       (unsigned int)sample_info->time,
                       (unsigned int)sample_info->delta,
                       (unsigned int)PCR_27MHz,
                       (unsigned int)PCR_Ext);



          if((int64)(PCR_base - m_nCurrPCR) <= 0)
          {
             // If PCRBase goes behing current PCR, Stop it at current PCr
             // This will only happen if video timestamp goes out of order
             // which will not happen as long as B frames are not supported
             // Revisit for B frame.
             PCR_base = m_nCurrPCR;
          }

          uint64 PCR = PCR_base * 300;

          if(PCR_base - m_nCurrPCR > 190 * 27000)
          {
             MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                           "PCR is late curr %d prev %d",
                            (unsigned int)PCR_base, (unsigned int)m_nCurrPCR );
          }

          m_nCurrPCR = PCR_base;

          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "Timer PCR SI %d - %d",
                                               (unsigned int)sample_info->time,
                                               (unsigned int)m_nCurrPCR );

          m_MP2PCRTSPacket[index] |= (uint8)((PCR_base >> 25) & 0xff);
          index += 1;
          m_MP2PCRTSPacket[index] |= (uint8)((PCR_base >> 17) & 0xff);
          index += 1;
          m_MP2PCRTSPacket[index] |= (uint8)((PCR_base >> 9) & 0xff);
          index += 1;
          m_MP2PCRTSPacket[index] |= (uint8)((PCR_base >> 1) & 0xff);
          index += 1;
          m_MP2PCRTSPacket[index] |= (uint8)((PCR_base << 7) & 0x80);

          m_MP2PCRTSPacket[index] |= (uint8)0x7E; //Reserved 6 bits

          //Put PCR extension 27Mhz clock value % 300
          m_MP2PCRTSPacket[index] = static_cast<uint8>(m_MP2PCRTSPacket[index] | ((PCR_Ext >> 8) & 0x1));
          index += 1;
          m_MP2PCRTSPacket[index] |= (uint8)(PCR_Ext);
          index += 1;

      }
    }

    /* Copy the remaining bytes from the sample data to the TS packet */
    if(index < TS_PKT_SIZE)
    {

          /* Copy PES header into the TS packet. PES will be there for the first packet of each frame */

      uint8 loop_index = 0;

      if(index < TS_PKT_SIZE )
      {
        // 2 bytes - 1 byte adaptation_field_length, 1 byte adaptation header
        uint8 bytes_to_stuff;
        if(m_MP2PCRTSPacket[4])
        {
           bytes_to_stuff = static_cast<uint8>(TS_PKT_SIZE - index);
        }
        else
        {
           bytes_to_stuff = static_cast<uint8>(TS_PKT_SIZE - (index + 2));
        }


        // Adaptation_field_control - 2 bits
        m_MP2PCRTSPacket[3] |= (MUX_MP2_ADAPTATON_AND_PAYLOAD << 4);
        /* Add bytes to stuff to adaptation header */
        if(m_MP2PCRTSPacket[4])
        {
          m_MP2PCRTSPacket[4] = m_MP2PCRTSPacket[4];
        }
        else
        {
          m_MP2PCRTSPacket[4] = static_cast<uint8>(m_MP2PCRTSPacket[4] + 1);
          /* Adaptation header  1- Byte */
          m_MP2PCRTSPacket[5] |= 0X00;
          /* Increment the index by 2 */
          index += 2;
        }


        if(bytes_to_stuff < TS_PKT_SIZE)
        {
          /* stuff bytes_to_stuff no. of bytes */
          for( loop_index = (uint8)index; loop_index < (index + bytes_to_stuff); loop_index++)
          {
            m_MP2PCRTSPacket[loop_index] = 0XFF;
          }
         index += bytes_to_stuff;
        }
      }
    }

    bool bEOF = true;

    if (!Output(m_MP2PCRTSPacket, 0, TS_PKT_SIZE, bEOF ))
    {
      MM_MSG_PRIO1( MM_GENERAL, MM_PRIO_ERROR,
                          " MP2BaseFile::GenerateMP2TSPCRPacket failed to write data  %d",
                          TS_PKT_SIZE);

      return 0;
    }
  }
  return 0;
}



/*===========================================================================

FUNCTION GeneratePESPacket

DESCRIPTION
  Generate PES packet for each audio/video frame

DEPENDENCIES
  None.

RETURN VALUE
  no of bytes consumed from the original sample

SIDE EFFECTS
  none

===========================================================================*/
uint16 MP2BaseFile::GeneratePESPacket(  uint32   stream_number,
                                       const MUX_sample_info_type  *sample_info,
                                       uint8  *PES_Header )
{
  int index = 0;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::GeneratePESPacket");
  uint64 nTimeMs  = 0;
  uint32 nTimeFPS = 0;
  uint32 nFramerate = 200;
  uint32 nRndValue = 0;
  uint64 PTS_Base  = 0;

  nTimeMs = (uint64)sample_info->time * 90;

  if(PES_Header)
  {
    memset((void *)PES_Header, 0, PES_PKT_SIZE);
    //PES packet
    //PES start code prefix - 0X000001
    PES_Header[index] |= 0X00;
    index += 1;
    PES_Header[index] |= 0X00;
    index += 1;
    PES_Header[index] |= 0X01;
    index += 1;

    if(m_Params->streams[stream_number].type == MUX_STREAM_VIDEO)
    {
      //stream_id  - 8 bits - E0 for AVC
      switch(m_Params->streams[stream_number].subinfo.video.format)
      {
         case MUX_STREAM_VIDEO_H264:
           PES_Header[index] |= 0XE0;
           index += 1;
           break;
         default:
           //The expectation is that validation of supported codecs are
           //done at initialization
           break;
      }
      nFramerate = m_nVideoFrameRate;

      nRndValue = 0;
    }
    else if(m_Params->streams[stream_number].type == MUX_STREAM_AUDIO)
    {
      //stream_id  - 8 bits -
      switch(m_Params->streams[stream_number].subinfo.audio.format)
      {
         case MUX_STREAM_AUDIO_MPEG4_AAC:
           PES_Header[index] |= 0XC1;
           index += 1;
           break;
         case MUX_STREAM_AUDIO_PCM:
           PES_Header[index] |= 0XBD;
           index += 1;
           break;
         case MUX_STREAM_AUDIO_AC3:
           PES_Header[index] |= 0xBD;
           index += 1;
           break;
         default:
           //The expectation is that validation of supported codecs are
           //done at initialization
           break;
      }

      nFramerate = m_nAudioFrameRate;

      nRndValue = 0;
    }

    /* PES_packet_length -  16 bits - 0 PES packets payload consists of bytes
     * from a video elementary stream
     */
    PES_Header[index] |= 0X00;
    index += 1;
    PES_Header[index] |= 0X00;
    index += 1;

    // 10 - 2 bits
    PES_Header[index] |= 0X80;
    // PES_scrambling_control - 2 bits
    PES_Header[index] |= 0X00;
    // PES_Priority - 1 bit
    if(m_randon_access_indicator)
    {
      PES_Header[index] |= 0X08;
    }
    // data_alignment_indicator
    if(m_data_alignment_indicator)
    {
      PES_Header[index] |= 0X04;
    }
    //copyright - 1 bit
    PES_Header[index] |= 0X00;
    //original_or_copy - 1 bit
    PES_Header[index] |= 0X00;
    index += 1;

    //PTS_DTS_Flags - 2 - bits
    //ESCR_flag
    //ES_rate_flag
    //DSM_trick_mode_flag
    //additional_copy_info_flag
    //PES_CRC_flag
    //PES_extension_flag
    //PES_header_data_length
    //PTS
    //DTS

    //Compute timestamp in 90KHz scale. Before that convert
    // to framerate scale to avoid any jitter and make
    //timestamps precisely multiples of frame duration
    if(m_PTS_DTS_Flags)
    {
      /**---------------------------------------------------------------------
         Converting the timetamp to FPS timescale and back will help us
         get rid of any jitter in taking the timestamp.
      ------------------------------------------------------------------------
      */
      PTS_Base = (nTimeMs - m_nBaseTimestamp);

      if(m_bAdjustTSforCompliance)
      {
        if(stream_number == m_audio_stream_num &&
          (m_Params->streams[m_audio_stream_num].subinfo.audio.sampling_frequency %
           m_Params->streams[m_audio_stream_num].sample_delta))
        {
           //NOTHING TO DO FOR NOW
        }
        else
        {
            nTimeFPS =  (PTS_Base * nFramerate + nRndValue)/ 90000;

            PTS_Base = ((uint64)nTimeFPS * 90000) /
                   (uint64)nFramerate;
        }
      }

      if((int64)((uint64)nTimeMs - m_nBaseTimestamp) <= (int64)0)
      {
        PTS_Base = (uint64)0;
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                     "Mux PTS Base to 0, Early frame %d, %d ",
                     (unsigned int)m_nBaseTimestamp,
                     (unsigned int)nTimeMs);
      }

      m_nPrevTimeStamp = PTS_Base;

      if(m_bAdjustTSforCompliance)
      {
        if(m_Params->streams[stream_number].type == MUX_STREAM_VIDEO)
        {
          if(PTS_Base <= m_nCurrVideoTime)
          {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile Error Video Timestamp %d %d",
                 (uint32)PTS_Base, (uint32)m_nCurrVideoTime);
            if(m_nVideoFrameRate)
            {
              PTS_Base += 90000/m_nVideoFrameRate;
            }
          }
          if(PTS_Base - m_nCurrVideoTime >=
           /*2times frameduration*/(90000 * 2)/m_nVideoFrameRate)
          {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFile Framedrop Video %d %d",
                 (uint32)PTS_Base, (uint32)m_nCurrVideoTime);
          }
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                   "MP2BaseFile::Video PTS Base %d", (uint32)(PTS_Base - m_nCurrVideoTime));
          m_nCurrVideoTime = PTS_Base;
        }
        else if(m_Params->streams[stream_number].type == MUX_STREAM_AUDIO)
        {
          /**---------------------------------------------------------------------
           If sampling rate is not a multiple of frame length we have a problem.
           We may not be able to accurately put the timestamp for each frame.
           At this step we try to make sure the calculated PTS Base is
           atleast a multiple of frame duration in 90KHz
          ------------------------------------------------------------------------
          */

          if(m_nAudioFrameDuration90Khz)
          {
            PTS_Base = (PTS_Base + (m_nAudioFrameDuration90Khz>>1))/
                         m_nAudioFrameDuration90Khz;
            PTS_Base *= m_nAudioFrameDuration90Khz;
          }
          if(PTS_Base <= m_nCurrAudioTime)
          {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile Error Audio Timestamp %d %d",
                 (uint32)PTS_Base, (uint32)m_nCurrAudioTime);
            PTS_Base += m_nAudioFrameDuration90Khz;
          }
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                 "MP2BaseFile::Audio PTS Base %d", (uint32)(PTS_Base - m_nCurrAudioTime));
          if(PTS_Base <= m_nCurrAudioTime)
          {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                  "MP2BaseFile::Audio PTS Delta 0 or negative Drop");
              return 0;
          }
          m_nCurrAudioTime = PTS_Base;
        }

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                 "MP2BaseFile::PTS_Base %d", (uint32)PTS_Base);
      }
    }
    uint8 header_length_stuff = 0;
    if(m_Params->streams[stream_number].type == MUX_STREAM_AUDIO)
    {
      m_PTS_DTS_Flags = 2;
      if(m_Params->streams[stream_number].subinfo.audio.format == MUX_STREAM_AUDIO_PCM)
      {
        header_length_stuff = 2;

      }
    }
    //PTS_DTS_Flags - 2 - bits
    //ESCR_flag
    //ES_rate_flag
    //DSM_trick_mode_flag
    //additional_copy_info_flag
    //PES_CRC_flag
    //PES_extension_flag
    //PES_header_data_length
    //PTS
    //DTS
    /* If extra data is available, then we will mark that flag*/

    bool extraDataValid = false;

    if(sample_info->extra_data_size > 0 && sample_info->extra_data_ptr)
    {
      extraDataValid = true;
    }
    if(extraDataValid)
    {
      PES_Header[index] |= 0X01;
    }

    switch (m_PTS_DTS_Flags)
    {
      case 0:
        /* No PTS, DTS */
        PES_Header[index] |= 0X00;
        index += 1;
        //PES_header_data_length
        PES_Header[index] = 0X00 + header_length_stuff;

        if(extraDataValid)
        {
          PES_Header[index] = static_cast<uint8>(PES_Header[index] + (1+sample_info->extra_data_size));
        }
        index += 1;
        break;

      case 1:
        /* 01 if forbidden */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                 "MP2BaseFile::GeneratePESPacket m_PTS_DTS_Flags 01 is not allowed");
        PES_Header[index] |= 0X00;
        index += 1;
        //PES_header_data_length
        PES_Header[index] = 0X00;
        break;

      case 2:
        /* PTS only */
        PES_Header[index] |= 0X80;
        index += 1;
        //PES_header_data_length
        PES_Header[index] = static_cast<uint8>(0X05+ header_length_stuff);

        if(extraDataValid)
        {
          PES_Header[index] = static_cast<uint8>(PES_Header[index] + (1+sample_info->extra_data_size));
        }
        index += 1;


        //PTS
        //4 bits - 0011 for PTS
        PES_Header[index] |= 0X20;
        //PTS[32...30]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 29);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[29....22]
        //PES_Header[index] |= (PTS_Base >> 21) & 0xff;
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 22);
        index += 1;
        //PTS[21....15]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 14);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[14....7]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 7);
        index += 1;
        //PTS[6....0]
        PES_Header[index] |= static_cast<uint8>(PTS_Base << 1);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        break;

      case 3:
        /* PTS + DTS */
        PES_Header[index] |= 0XC0;
        index += 1;
        //PES_header_data_length
        PES_Header[index] = static_cast<uint8>(0X0A + header_length_stuff);
        if(extraDataValid)
        {
          PES_Header[index] = static_cast<uint8>(PES_Header[index] + (1+sample_info->extra_data_size));
        }
        index += 1;
        //PTS
        //4 bits - 0011 for PTS
        PES_Header[index] |= 0X30;
        //PTS[31...30]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 29);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[29....22]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 22);
        index += 1;
        //PTS[21....15]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 14);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[14....7]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 7);
        index += 1;
        //PTS[6....0]
        PES_Header[index] |= static_cast<uint8>(PTS_Base << 1);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;

        //DTS
        //4 bits - 0011 for PTS
        PES_Header[index] |= 0X10;
        //DTS[31...30]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 29);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //DTS[29....22]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 22);
        index += 1;
        //DTS[21....15]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 14);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[14....7]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 7);
        index += 1;
        //DTS[6....0]
        PES_Header[index] |= static_cast<uint8>(PTS_Base << 1);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        break;

      default:
        break;
    }
    if(extraDataValid)
    {
      PES_Header[index] |= 0x80; // PES_private_data_flag 1bit
                                 // pack_header_field_flag 1bit
                                 // program_packet_sequence
                                 // _counter_flag 1bit
                                 // P-STD_buffer_flag 1bit
      PES_Header[index] |= 0x0E; // Reserved 3bits 0b111
                                 // PES_extension_flag_2 1bit

      index += 1;

      memcpy(&(PES_Header[index]), sample_info->extra_data_ptr, sample_info->extra_data_size);
      for ( unsigned int idx = 0; idx < sample_info->extra_data_size; idx++)
      {
         MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_LOW,
         "MP2BaseFile::GeneratePESPacket Extradata[%d]: %x",idx,sample_info->extra_data_ptr[idx]);
      }
      index += (int)sample_info->extra_data_size;
  }
    if(m_Params->streams[stream_number].type == MUX_STREAM_AUDIO)
    {
      if(m_Params->streams[stream_number].subinfo.audio.format == MUX_STREAM_AUDIO_PCM)
      {

        //Update Private Stream Properties.
        //
        //stuffing
        PES_Header[index] = 0xFF;
        index += 1;
        PES_Header[index] = 0xFF;
        index += 1;
        /* MP2BaseFile:: Mux Client has to add the 4 byte LPCM private header */
      }
    }

    // Update 16 bit PES packet length field
    if((index - 6   + sample_info->size) < (1 << 16)) {
      //Update the PES packet length for audio
      PES_Header[4] = ((index - 6   + sample_info->size) >> 8) & 0xFF;
      PES_Header[5] = ((index - 6   + sample_info->size)) & 0xFF;
    }
  }
  return (uint16)index;
}

/*===========================================================================

FUNCTION GeneratePESPacket

DESCRIPTION
  Generate PES packet for each audio/video frame

DEPENDENCIES
  None.

RETURN VALUE
  no of bytes consumed from the original sample

SIDE EFFECTS
  none

===========================================================================*/
#if 1
uint16 MP2BaseFile::GenerateFreezeFramePESPacket(  uint32   stream_number,
                                       const MUX_sample_info_type  *sample_info,
                                       uint8  *PES_Header )
{
  int index = 0;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::GeneratePESPacket");

  if(PES_Header)
  {
    memset((void *)PES_Header, 0, TS_PKT_SIZE);
    //PES packet
    //PES start code prefix - 0X000001
    PES_Header[index] |= 0X00;
    index += 1;
    PES_Header[index] |= 0X00;
    index += 1;
    PES_Header[index] |= 0X01;
    index += 1;

    if(m_Params->streams[stream_number].type == MUX_STREAM_VIDEO)
    {
      //stream_id  - 8 bits - E0 for AVC
      switch(m_Params->streams[stream_number].subinfo.video.format)
      {
         case MUX_STREAM_VIDEO_H264:
           PES_Header[index] |= 0XE0;
           index += 1;
           break;
         default:
           //The expectation is that validation of supported codecs are
           //done at initialization
           break;
      }

    }
    else if(m_Params->streams[stream_number].type == MUX_STREAM_AUDIO)
    {
      /*
      * Only applicable to video stream
      */
      return 0;
    }

    /* PES_packet_length -  16 bits - 0 PES packets payload consists of bytes
     * from a video elementary stream
     */
    PES_Header[index] |= 0X00;
    index += 1;
    PES_Header[index] |= 0X00;
    index += 1;

    // 10 - 2 bits
    PES_Header[index] |= 0X80;
    // PES_scrambling_control - 2 bits
    PES_Header[index] |= 0X00;
    // PES_Priority - 1 bit
    if(m_randon_access_indicator)
    {
      PES_Header[index] |= 0X08;
    }
    // data_alignment_indicator
    if(m_data_alignment_indicator)
    {
      PES_Header[index] |= 0X04;
    }
    //copyright - 1 bit
    PES_Header[index] |= 0X00;
    //original_or_copy - 1 bit
    PES_Header[index] |= 0X00;
    index += 1;

    //PTS_DTS_Flags - 2 - bits
    //ESCR_flag
    //ES_rate_flag
    //DSM_trick_mode_flag
    //additional_copy_info_flag
    //PES_CRC_flag
    //PES_extension_flag
    //PES_header_data_length
    //PTS
    //DTS
    uint64 PTS      = (sample_info->time + 66) *
                       (uint64)MUX_MP2_SYSTEM_REFERENCE_CLOCK
                         / 1000;
    uint64 PTS_Base = ((sample_info->time  + 66)* 90);


    switch (m_PTS_DTS_Flags)
    {
      case 0:
        /* No PTS, DTS */
        PES_Header[index] |= 0X00;
        PES_Header[index] |= 0x8;//DSM trick mode flag
        index += 1;
        //PES_header_data_length
        PES_Header[index] = 0X02;
        index += 1;
        break;

      case 1:
        /* 01 if forbidden */
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                 "MP2BaseFile::GeneratePESPacket m_PTS_DTS_Flags 01 is not allowed");
        PES_Header[index] |= 0X00;
        PES_Header[index] |= 0x8;//DSM trick mode flag
        index += 1;
        //PES_header_data_length
        PES_Header[index] = 0X02;
        index += 1;
        break;

      case 2:
        /* PTS only */
        PES_Header[index] |= 0X80;
        PES_Header[index] |= 0x8;//DSM trick mode flag
        index += 1;
        //PES_header_data_length
        PES_Header[index] = 0X07;
        index += 1;


        //PTS
        //4 bits - 0011 for PTS
        PES_Header[index] |= 0X30;
        //PTS[32...30]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 29);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[29....22]
        PES_Header[index] |= static_cast<uint8>((PTS_Base >> 21) & 0xff);
        index += 1;
        //PTS[21....15]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 14);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[14....7]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 7);
        index += 1;
        //PTS[6....0]
        PES_Header[index] |= static_cast<uint8>(PTS_Base << 1);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        break;

      case 3:
        /* PTS + DTS */
        PES_Header[index] |= 0XC0;
        PES_Header[index] |= 0x8;//DSM trick mode flag
        index += 1;
        //PES_header_data_length
        PES_Header[index] = 0X0C;
        index += 1;
        //PTS
        //4 bits - 0011 for PTS
        PES_Header[index] |= 0X30;
        //PTS[31...30]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 29);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[29....22]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 22);
        index += 1;
        //PTS[21....15]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 14);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[14....7]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 7);
        index += 1;
        //PTS[6....0]
        PES_Header[index] |= static_cast<uint8>(PTS_Base << 1);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;

        //DTS
        //4 bits - 0011 for PTS
        PES_Header[index] |= 0X10;
        //DTS[31...30]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 29);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //DTS[29....22]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 22);
        index += 1;
        //DTS[21....15]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 14);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        //PTS[14....7]
        PES_Header[index] |= static_cast<uint8>(PTS_Base >> 7);
        index += 1;
        //DTS[6....0]
        PES_Header[index] |= static_cast<uint8>(PTS_Base << 1);
        // marker_bit - 1 bit, always 1
        PES_Header[index] |= 0X01;
        index += 1;
        break;

      default:
        break;
    }


    // trick mode comtrol
    PES_Header[index] |= 0x60; // freeze_frame
    PES_Header[index] |= 0x10; // repeat complete frame
    index += 1;
    PES_Header[index] = 0xFF;
    index += 1;
    PES_Header[5] = (uint8)(index -  6 + 1);// size of PES packet LSB
    PES_Header[4] = (uint8)(( index - 6  + 1) >> 8);// size of PES packet MSB

  }
  return (uint16)index;
}
#endif
/*===========================================================================

FUNCTION MUX_write_header

DESCRIPTION
  This is used to pass sps/pps or any video/audio header information to be
  written to the header of file.

DEPENDENCIES
  None

RETURN VALUE
  MUX_STATUS

SIDE EFFECTS
 None

===========================================================================*/
MUX_STATUS MP2BaseFile::MUX_write_header (uint32 stream_id, uint32 header_size, const uint8 *header)
{
  MUX_STATUS return_status = MUX_SUCCESS;
  uint32 sample_data_offset = 0;

  //this is temporary code, need to remove this
  // Right now we are getting sps/pps prepended by 0X27 and 0X28
  //but most of the m2ts streams are starting with 67 and 68 after the start code 0X00000001
  //Once we know which is correct then we can remove this code.
  /* uint8 * header_temp = (uint8 *) MM_Malloc(header_size);
  if(header_temp)
  {
    memcpy(header_temp, header,  header_size);
    header_temp[4] = 0X67;
    header_temp[28] = 0X68;
  }*/

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::MUX_write_header");

  if(!m_Params || !m_Params->streams || stream_id > m_Params->num_streams)
  {
      return MUX_FAIL;
  }
  /* If audio is enabled, write codec-specific header information. */
  if (header_size && _success)
  {
    m_pHeader[stream_id] = (uint8*)MM_Malloc(header_size);

    if(!m_pHeader[stream_id])
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::MUX_write_header Malloc fail");
      return MUX_FAIL;
    }

    memcpy(m_pHeader[stream_id], header, header_size);
MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::MUX_write_header size =%d",header_size );
    m_nHeaderSize[stream_id] = header_size;

#if 0

    /* Write the header to the movie file. */
    if( m_Params->streams[stream_id].type == MUX_STREAM_VIDEO )
    {
      GenerateMP2TSVideoPacketForHeader(stream_id, header_size, header_temp );
      //this is temporary code, need to remove this
      MM_Free(header_temp);
      header_temp = NULL;

      if(!_success)
      {
        return_status = MUX_FAIL;
      }
    }
#endif
  }
  return return_status;
}

/*===========================================================================

FUNCTION GenerateMP2TSVideoPacketForHeader

DESCRIPTION
  This is used to pass sps/pps video header information to generate TS packet with the header.

DEPENDENCIES
  None

RETURN VALUE
  MUX_STATUS

SIDE EFFECTS
 None

===========================================================================*/
MUX_STATUS MP2BaseFile::GenerateMP2TSVideoPacketForHeader(uint32 stream_number, uint32 header_size, uint8 *header )
{
  uint32 sample_data_offset = 0;
  int index = 0;
  uint8 bytes_to_copy = 0;
  uint16 PES_Header_Size = 0;
  bool PES_Header_Copied;

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::GenerateMP2TSVideoPacketForHeader");
  m_PSI_payload_start_indicator_set = 0;
  m_MP2TS_continuty_counter[stream_number] = 0;
  /* This sample info is to pass the time stamp to PES packets. */
  MUX_sample_info_type  *sample_info = (MUX_sample_info_type *) MM_Malloc(sizeof(MUX_sample_info_type));
  if(sample_info == NULL)
  {
    return MUX_FAIL;
  }
  else
  {
    memset((void *)sample_info, 0, sizeof(MUX_sample_info_type));
    /* Set the time stamp to 0 */
    sample_info->time = 0;
  }

  m_adaptation_field = MUX_MP2_ADAPTATON_AND_PAYLOAD;
  m_PTS_DTS_Flags    = 3;
  m_MP2TS_continuty_counter[stream_number] = 0;
  if(m_MP2PESPacket)
  {
    /* Generate the PES packet for header */
    PES_Header_Size = GeneratePESPacket(stream_number, sample_info, m_MP2PESPacket);
  }
  PES_Header_Copied = FALSE;
  MM_Free(sample_info);
  sample_info = NULL;

  while(sample_data_offset < (header_size + PES_Header_Size))
  {
    if(m_MP2TSPacket)
    {
      memset(m_MP2TSPacket, 0, TS_PKT_SIZE);
      // TS stream sync byte  - 8 bits
      m_MP2TSPacket[index] = TS_PKT_SYNC_BYTE;
      index += 1;
      // Transport_error_indicator always set to 0 - 1 bit
      m_MP2TSPacket[index] |= 0X0;
      if(!m_PSI_payload_start_indicator_set)
      {
        // Payload start indicator  1 - if the first byte of PSI section - 1 bit
        m_MP2TSPacket[index] |= 0X40;
        m_PSI_payload_start_indicator_set = 1;
      }
      else
      {
        // Payload start indicator  1 - if the first byte of PSI section - 1 bit
        m_MP2TSPacket[index] |= 0X00;
      }
      // Transport_priority always set to 1 for header - 1 bit
      m_MP2TSPacket[index] |= 0X20;
      // PID for Video - 0X0020  - 13 bits
      m_MP2TSPacket[index] |= 0X00;
      index += 1;
      m_MP2TSPacket[index] |= 0X20;
      index += 1;
      // Transport scrambling control - 2 bits
      m_MP2TSPacket[index] |= 0X00;
      m_MP2TSPacket[index] |= m_MP2TS_continuty_counter[stream_number];
      m_MP2TS_continuty_counter[stream_number] = static_cast<uint8>
          ((m_MP2TS_continuty_counter[stream_number] + 1) % 16);
      index += 1;
      if(index < TS_PKT_SIZE)
      {
        bytes_to_copy = 0;
        uint8 loop_index = 0;
        if(!PES_Header_Copied)
        {
          bytes_to_copy =  (uint8)(MIN( ((uint32)((header_size + PES_Header_Size) - sample_data_offset)), ((uint32)(TS_PKT_SIZE - index))));
        }
        else
        {
          bytes_to_copy =  (uint8)(MIN( ((uint32)(header_size - sample_data_offset)), ((uint32)(TS_PKT_SIZE - index))));
        }
        if( bytes_to_copy == 183)
        {
          // Adaptation_field_control - 2 bits
          m_MP2TSPacket[3] |= (MUX_MP2_ADAPTATON_AND_PAYLOAD << 4);
          m_MP2TSPacket[4] = 0X0;
          index = 5;
        }
        else if((index + bytes_to_copy) < TS_PKT_SIZE )
        {
          // 2 bytes - 1 byte adaptation_field_length, 1 byte adaptation header
          uint8 bytes_to_stuff = static_cast<uint8>(TS_PKT_SIZE - (index + bytes_to_copy + 2));
          // Adaptation_field_control - 2 bits
          m_MP2TSPacket[3] |= (MUX_MP2_ADAPTATON_AND_PAYLOAD << 4);
          m_MP2TSPacket[4] = static_cast<uint8>(m_MP2TSPacket[4] + (bytes_to_stuff + 1));
          m_MP2TSPacket[5] |= 0X00;
          index = 6;
          if(bytes_to_stuff < TS_PKT_SIZE)
          {
            /* Stuff the remaining bytes */
            for( loop_index = (uint8)index; loop_index < (index + bytes_to_stuff); loop_index++)
            {
              m_MP2TSPacket[loop_index] = 0XFF;
            }
          }
          index += bytes_to_stuff;
        }
        if(!PES_Header_Copied)
        {
          if(PES_Header_Size && m_MP2PESPacket && (PES_Header_Size < (TS_PKT_SIZE - index)))
          {
            /* Copy the PES header into the TS packet */
            memcpy((void *)(m_MP2TSPacket +index), m_MP2PESPacket, PES_Header_Size);
            index += PES_Header_Size;
          }
          memcpy((void *)(m_MP2TSPacket +index), (header + sample_data_offset), (bytes_to_copy - PES_Header_Size));
          PES_Header_Copied = TRUE;
        }
        else
        {
          memcpy((void *)(m_MP2TSPacket +index), (header + sample_data_offset), (bytes_to_copy));
        }
      }
      bool bEOF = false;
      if(bytes_to_copy >= header_size)
      {
        bEOF = true;
      }
      if (!Output(m_MP2TSPacket, m_output_offset, TS_PKT_SIZE, bEOF))
      {
        MM_MSG_PRIO1( MM_GENERAL, MM_PRIO_ERROR,
                      " MP2BaseFile::GenerateMP2TSVideoPacket failed to write data  %d",
                      TS_PKT_SIZE);

        return MUX_FAIL;
      }
    }
    sample_data_offset += bytes_to_copy;
  }
  return MUX_SUCCESS;
}

/*=============================================================================
FUNCTION:
 OGGStreamParser::FindCheckSum

DESCRIPTION:
 Finds the checksum for current page
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 CRC check sum
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 MP2BaseFile::FindCheckSum(uint8* pData, uint32 nSize)
{
    uint32 i =0;
    uint32 dlay = -1;
    uint32 BYTE = 0;

    /**-------------------------------------------------------------------------
       This is an implementation of the adder circuit(Fig A.1) in Annex A
       of the spec.
       Need to try to optimize it. For now we can live with this.
    ----------------------------------------------------------------------------
    */

    for(i =0; i < nSize*8; i++)
    {
        if(i % 8 == 0)
        {
            BYTE = *pData++ << 24;

        }
        if(MSBIT(BYTE) ^ MSBIT(dlay))
        {
            dlay = ((dlay << 1) ^ (MP2_CRC_GENERATOR_POLYNOMIAL ));
            //dlay|= 1;
        }
        else
        {
            dlay = dlay << 1;
        }
        BYTE <<= 1;

    }
    return dlay;
}
