/* =======================================================================
                              MP2BaseFileSecure.cpp
DESCRIPTION
  This module is to record Secure MP2 Transport and program streams.



  Copyright (c) 2011 - 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */


/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Sink/FileMux/MP2BaseFileLib/main/latest/src/MP2BaseFileSecure.cpp

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "MP2SecureMuxConstants.h"
#include "MP2BaseFileSecure.h"
#include "MMMemory.h"
#include "MMDebugMsg.h"
#include "DataSourcePort.h"
#include "smux_tz_commands.h"
#include "smux_mem.h"
#include "Transcode.h"

uint64 MP2BaseFileSecure::m_llTimeBase = 0;

/* =======================================================================
**                            Function Definitions
** ======================================================================= */
/*===========================================================================

FUNCTION  MP2BaseFileSecure

DESCRIPTION
This is the MP2BaseFileSecure class constructor - initializes the class members.
===========================================================================*/
MP2BaseFileSecure::MP2BaseFileSecure( MUX_create_params_type *Params,
                            MUX_fmt_type file_format,
                            MUX_brand_type file_brand,
                            MUX_handle_type *output_handle)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFileSecure::MP2BaseFileSecure");
  smux_config_t config_args;
  config_args.audioType = SEC_MUX_STREAM_AUDIO_NONE;
  config_args.videoType = SEC_MUX_STREAM_VIDEO_NONE;
  m_Params = NULL;
  m_llTimeBase = 0;
  m_prev_PCR_time = 0;
  m_table_offset = 0;
  m_prev_userdata_time = 0;
  m_llVideoDelayCorrection = 0;
  m_bLookforIframe = false;
  m_nAudioFrameDuration90Khz = 0;
  m_nAudioFrameRate = 1;
  m_nVideoFrameRate = 1;
  m_nCurrAudioTime = 0;
  m_nCurrVideoTime = 0;
  m_nCurrPCR = 0;
  m_nAVCHrdDescrLen = 0;
  m_bGenerateTables = false;
  m_bBaseTimeStampTaken = false;
  m_adaptation_field = 0;
  m_pAVCHrdDescr = NULL;
  securemux_handler = NULL;
  m_userdata_handler = NULL;
  m_is_PCR_data = false;
  m_is_user_data = false;

  if(smux_start_app(&securemux_handler, "securemux", 1024))
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFileSecure: smux_start_app Failed");
    return;
  }

  // allocate non secure ION buffer for PAT/PMT/PCR packets
  memset(&m_PAT_PMT_PCR_handler, 0, sizeof(struct smux_ion_info));

  if(smux_ION_memalloc(&m_PAT_PMT_PCR_handler, TS_PKT_SIZE*3) != 0)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFileSecure: smux_ION_memalloc failure");
    return;
  }

  if(Params && (file_format == MUX_FMT_SECURE_MP2))
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
          config_args.videoType = SEC_MUX_STREAM_VIDEO_H264;
        }
        m_video_stream_num = i;
      }
      if(Params->streams[i].type == MUX_STREAM_AUDIO)
      {
        if(Params->streams[i].subinfo.audio.format == MUX_STREAM_AUDIO_PCM ||
            Params->streams[i].subinfo.audio.format == MUX_STREAM_AUDIO_MPEG4_AAC)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFileSecure Audio Found-->");
          m_bAudioPresent = true;

          config_args.audioType = (smux_audio_type_t)Params->streams[i].subinfo.audio.format;
        }
        m_audio_stream_num = i;
      }
    }

    if(!m_bAudioPresent && !m_bVideoPresent)
    {
      _success = false;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFileSecure Unsupported codecs");
      return;
    }

    /**
    * Cache the movie and stream parameters)
    */
    m_Params = ( MUX_create_params_type *)
                  MM_Malloc(sizeof(MUX_create_params_type));
    if(!m_Params)
    {
      _success = false;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFileSecure Cannot alloc Params memory");
      return;
    }
    memcpy(m_Params, Params, sizeof(MUX_create_params_type));

    m_Params->streams = (MUX_stream_create_params_type*)MM_Malloc(
        m_Params->num_streams * sizeof(MUX_stream_create_params_type));

    if(!m_Params->streams)
    {
      _success = false;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFileSecure Cannot alloc Params memory");
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
            m_eAACFormat = SEC_AAC_FORMAT_ADTS;
        }
        else
        {
            m_eAACFormat = SEC_AAC_FORMAT_LOAS;
        }

    }
    m_file_brand = file_brand;
    if(output_handle)
    {
      memcpy(&m_output_handle, output_handle, sizeof(MUX_handle_type));
    }

    // initialize the class member variables
    InitData();

    if(!_success)
    {
      _success = false;
      return;
    }
  }
  else
  {
    _success = false;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFileSecure Invalid arguments");
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
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFileSecure Video Properties....");
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFileSecure Video FPS = %d",
                  m_nVideoFrameRate);
  }

  if(m_bAudioPresent)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFileSecure Audio Properties....");
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFileSecure Audio Samplerate = %d",
                  Params->streams[m_audio_stream_num].subinfo.audio.sampling_frequency);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFileSecure Audio FrameLength = %d",
                  Params->streams[m_audio_stream_num].sample_delta);
    if(Params->streams[m_audio_stream_num].sample_delta)
    {
       if(Params->streams[m_audio_stream_num].subinfo.audio.sampling_frequency %
          Params->streams[m_audio_stream_num].sample_delta == 0)
       {
          m_nAudioFrameRate = Params->streams[m_audio_stream_num].subinfo.
                                                             audio.sampling_frequency/
                              Params->streams[m_audio_stream_num].sample_delta;
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "MP2BaseFileSecure Audio FrameRate = %d",
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
    m_table_offset = 0;
  }

  // configure TZ secure mux  instance
  config_args.u16AudioPid = Params->audio_pid;
  config_args.u16VideoPid = Params->video_pid;
  config_args.u8AudioStreamNum = m_audio_stream_num;
  config_args.u8VideoStreamNum = m_video_stream_num;

  if (SMUX_config(securemux_handler , &config_args , &m_PAT_PMT_PCR_handler) != SEC_MUX_SUCCESS)
  {
		MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFileSecure: SMUX_config Failed");
  }

  return;
}

/*! ======================================================================
    @brief  Initialize the necessary class member variables.

    @detail This method is called to initialize the necessary class member variables.
========================================================================== */
void MP2BaseFileSecure::InitData()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFile::InitData");
  _success = true;
  m_bBaseTimeStampTaken = false;
  m_nCurrAudioTime = 0;
  m_nCurrVideoTime = 0;
  m_nCurrPCR = 0;
  m_randon_access_indicator = FALSE;
  m_user_data_continuty_counter = 0;
  m_MP2TS_table_continuty_counter[0] = 0;
  m_MP2TS_table_continuty_counter[1] = 0;
  m_MP2TS_table_continuty_counter[2] = 0;
  m_pAVCHrdDescr = NULL;
  m_nAVCHrdDescrLen = 0;
  m_adaptation_field = 0;
  m_nVideoFrameRate = 1;
  m_nAudioFrameRate = 1;
  m_bGenerateTables = false;
  memset(&m_sHRDParams, 0, sizeof(m_sHRDParams));
}
/*===========================================================================

FUNCTION MUX_write_user_meta_data

DESCRIPTION
Mux user meta data information in every timeout
DEPENDENCIES
  None.

RETURN VALUE
  MUX_STATUS.

SIDE EFFECTS
  This function is not thread safe.

===========================================================================*/
MUX_STATUS MP2BaseFileSecure::MUX_write_user_meta_data (const void *data, uint32 size, uint32 timeout)
{
	uint32 u32NumTSPackets;
	uint8 u8ContinutyCounter = 0;

	// check input parameters
	if(!data || !size || !timeout)
	{
		MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MUX_write_user_meta_data: Invalid parameters");
		return MUX_INVALID;
	}

	// calculate the number of TS packets that will be required to the new User metadata
	u32NumTSPackets = size/(TS_PKT_SIZE - MUX_TS_PKT_HDR_BYTES);
	if(size % (TS_PKT_SIZE - MUX_TS_PKT_HDR_BYTES))
	{
		u32NumTSPackets += 1;
	}

	// release the previous user meta-data ION handler
	if(m_userdata_handler != NULL)
	{
		if(smux_ion_dealloc(m_userdata_handler) != 0)
		{
			MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Failed deallocate User data ION buffer");
			return MUX_FAIL;
		}

		delete m_userdata_handler;
		m_userdata_handler = NULL;
	}

	// allocate new non secure ION buffer for User metadata packets
	m_userdata_handler = new struct smux_ion_info;
	memset(m_userdata_handler, 0, sizeof(struct smux_ion_info));
	if(smux_ION_memalloc(m_userdata_handler, TS_PKT_SIZE*u32NumTSPackets) != 0)
	{
		MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "MP2BaseFileSecure::failure to allocate user metadata ION buffer");
		return MUX_FAIL;
	}

	uint8* pMP2TSPacket = m_userdata_handler->ion_sbuffer;

	for(uint32 i=0; i < u32NumTSPackets; i++)
	{
		int index = 0;

		memset(pMP2TSPacket, 0, TS_PKT_SIZE);

		// TS stream sync byte  - 8 bits
		pMP2TSPacket[index] = TS_PKT_SYNC_BYTE;
		index += 1;

		// Transport_error_indicator always set to 0 - 1 bit
		pMP2TSPacket[index] |= 0X0;

		// Payload start indicator set to 1 if it the first packet - 1 bit
		if(i == 0)
		{
			pMP2TSPacket[index] |= 0X40;
		}

		// Transport_priority always set to 1 - 1 bit
		pMP2TSPacket[index] |= 0X20;

		pMP2TSPacket[index] |= (uint8)(m_Params->userdata_pid >> 8);
		index += 1;

		pMP2TSPacket[index] |= (uint8)(m_Params->userdata_pid);
		index += 1;

		// Transport scrambling control - 2 bits
		pMP2TSPacket[index] |= 0X00;

		// Adaptation_field_control - 2 bits - 01
		pMP2TSPacket[index] |= 0X10;

		// continuty counter 4 bits
		pMP2TSPacket[index] |= 0;

		index += 1;

		uint8 nBytesToCopy = (uint8)(MIN(TS_PKT_SIZE - MUX_TS_PKT_HDR_BYTES, size));

		// write the user metadata payload
		memcpy (pMP2TSPacket + index, (uint8*)data, nBytesToCopy);

		data = (uint8*)data + nBytesToCopy;
		size -= nBytesToCopy;
		index += nBytesToCopy;

		/* Fill the remaining TS packet bytes with FF */
        for(uint8 loop_index = (uint8)index; loop_index < TS_PKT_SIZE; loop_index++)
		{
			pMP2TSPacket[loop_index] = 0XFF;
		}

		pMP2TSPacket += TS_PKT_SIZE;
	}

   m_user_timeout = timeout;

   return MUX_SUCCESS;
}
/*===========================================================================

FUNCTION MUX_Process_Sample

DESCRIPTION
  This function process the audio and video samples.

DEPENDENCIES
  None.

RETURN VALUE
  MUX_STATUS.

SIDE EFFECTS
  This function is not thread safe.

===========================================================================*/
MUX_STATUS MP2BaseFileSecure::MUX_Process_Sample( uint32 stream_number,
                                        uint32 num_samples,
                                        const MUX_sample_info_type  *sample_info,
                                        const uint8 *sample_data)
{
	MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
			"MP2BaseFileSecure::MUX_Process_Sample stream number  %d ", stream_number);

	// check the given input
	if( !num_samples || !sample_info || !sample_data || !_success ||
			(((struct smux_ion_info*)sample_data)->sbuf_len > (SMUX_PHYSICAL_CHUNK_LEN * SMUX_MAX_PHYSICAL_CHUNKS)) ||
			(sample_info->extra_data_size - sample_info->extra_data_offset) > MUX_PES_EXTN_PVT_DATA_BYTES ||
			sample_info->fmt_pvtdata_ptr || sample_info->fmt_pvtdata_size /* fmt_pvt_data not supported */)
	{
		MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " Invalid Args ");
		return MUX_INVALID;
	}

	// calculate the current time.
	struct timespec currTime;
	clock_gettime(CLOCK_MONOTONIC, &currTime);
	uint64 u64currTime = (currTime.tv_sec * (uint64)1000000000) + (uint64)(currTime.tv_nsec);

	// check if between two calls to sample_process 90ms pass -
	// if so call PCR callback function
	if(m_prev_PCR_time && !m_is_PCR_data &&
			((uint64)(u64currTime - m_prev_PCR_time) > (uint64)(90000000)))
	{
		m_prev_PCR_time = u64currTime;
		PCR_callback((void *)this);
	}

	// create PAT/PMT tables
	if(m_bGenerateTables)
	{
		GeneratePATPacket();
		GeneratePMTPacket();
		m_table_offset = 0;
		m_bGenerateTables = false;
	}

	// check if between two calls to sample_process m_user_timeout pass -
	// if so mark user callback flag
	if(m_prev_PCR_time && m_userdata_handler && !m_is_user_data &&
			((uint64)(u64currTime - m_prev_userdata_time) > (m_user_timeout*(1000000))))
	{
		for(uint32 i = 0; i< m_userdata_handler->sbuf_len/TS_PKT_SIZE; i++)
		{
			*(m_userdata_handler->ion_sbuffer + i*TS_PKT_SIZE + 3) = *(m_userdata_handler->ion_sbuffer + i*TS_PKT_SIZE + 3) | m_user_data_continuty_counter;

            m_user_data_continuty_counter = static_cast<uint8>(m_user_data_continuty_counter + 1);

			if(m_user_data_continuty_counter >= 16)
			{
				m_user_data_continuty_counter = 0;
			}
		}
		m_is_user_data = true;
		m_prev_userdata_time = u64currTime;
	}

	if(m_bAudioPresent && m_bVideoPresent)
	{
		if(stream_number == m_audio_stream_num)
		{
			if(!m_bBaseTimeStampTaken)
			{
				MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Drop Early Audio No Video yet");
				return MUX_SUCCESS;
			}

			else if( ((int64)sample_info->time * 90) < ((int64)m_nBaseTimestamp + 9000) )
			{
				MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Drop Early Audio");
				return MUX_SUCCESS;
			}
			else if( (int64)sample_info->time * 90 - ((int64)m_nBaseTimestamp)
							 < (int64)m_nCurrPCR)
			{
				MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Drop Late Audio");
				return MUX_SUCCESS;
			}
		}

		if(stream_number == m_video_stream_num)
		{
			if(m_bLookforIframe == true)
			{
				MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,  "Looking for I frame");
				if(sample_info->sync)
				{
					MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Looking for I frame... found");
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
				// If fixed frame rate is used, timestamp jitter is corrected later and this has to be
				// accounted for while checking if PTS is behing PCR
				nFrameDelta90KHz = 90000/m_nVideoFrameRate;

				currTime90KHz = (((int64)sample_info->time * 90 - (int64)m_nBaseTimestamp)/
						nFrameDelta90KHz) * nFrameDelta90KHz;
			}
			if(m_bBaseTimeStampTaken && currTime90KHz <= (int64)m_nCurrPCR)
			{
				MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH, "Drop Late Video %u %u %u",
						(unsigned int)( (int64)sample_info->time * 90 - (int64)m_nBaseTimestamp),
						(unsigned int)currTime90KHz,
						(unsigned int)(m_nCurrPCR));
				m_bLookforIframe = true;
				return MUX_OUTDATED;
			}
		}
	}

	if(!m_bBaseTimeStampTaken && stream_number == m_video_stream_num)
	{
		m_nBaseTimestamp = sample_info->time * (uint64)90;
		MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "Mux Taking base time %u  %u",
								 (unsigned int)sample_info->time,
								 (unsigned int)m_nBaseTimestamp);

		m_bBaseTimeStampTaken = true;

		struct timespec tempTime;
		clock_gettime(CLOCK_MONOTONIC, &tempTime);
		m_llTimeBase = (tempTime.tv_sec * (uint64)1000000000) +
														 (uint64)(tempTime.tv_nsec);

		// Initiate the PCR call back
		if(!m_prev_PCR_time)
		{
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
			 GenerateMP2TSPCRPacket(&sSampleInfo);
			 m_prev_PCR_time = m_llTimeBase;
			 m_prev_userdata_time = m_llTimeBase;
			 m_is_PCR_data =  true;
			 if(m_file_brand != MUX_BRAND_MP2TS)
			 {
				 m_table_offset = 2*TS_PKT_SIZE;
			 }
		}

		m_nBaseTimestamp -= MUX_MP2_PTS_DELAY_CONST_90KHZ;
		MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
								"Mux Taking base time Base Adjusted %u",
								 (unsigned int)m_nBaseTimestamp);
	}

	//Drop audio frames which arrives with a TS which is less than start time
	if(stream_number == 0 &&
		 ((int64)((uint64)sample_info->time * 90 - m_nBaseTimestamp) < 0))
	{
		MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Audio with old TS Dropped");
		return MUX_SUCCESS;
	}


	// Compute PTS timestamp for the PES header, the PTS is in 90KHz scale.
	// Converting the timetamp to FPS timescale and back will help us
	// get rid of any jitter in taking the timestamp.
	uint64  nTimeMs = (uint64)sample_info->time * 90;
	uint64 PTS_Base = (nTimeMs - m_nBaseTimestamp);
	uint32 nTimeFPS = 0;
	uint32 nFramerate = 200;
	uint32 nRndValue = 0;

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
			nTimeFPS = (uint32)(PTS_Base * nFramerate + nRndValue)/ 90000;
			PTS_Base = ((uint64)nTimeFPS * 90000) / (uint64)nFramerate;
		}
	}

	if((int64)((uint64)nTimeMs - m_nBaseTimestamp) <= (int64)0)
	{
		PTS_Base = (uint64)0;
	}

	if(m_bAdjustTSforCompliance)
	{
		if(m_Params->streams[stream_number].type == MUX_STREAM_VIDEO)
		{
			if(PTS_Base <= m_nCurrVideoTime)
			{
				if(m_nVideoFrameRate)
				{
					PTS_Base += 90000/m_nVideoFrameRate;
				}
			}

			MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
							 "Video PTS Base %d", (uint32)(PTS_Base - m_nCurrVideoTime));
			m_nCurrVideoTime = PTS_Base;
		}
		else if(m_Params->streams[stream_number].type == MUX_STREAM_AUDIO)
		{
			// If sampling rate is not a multiple of frame length we have a problem.
			// We may not be able to accurately put the timestamp for each frame.
			// At this step we try to make sure the calculated PTS Base is
			// at least a multiple of frame duration in 90KHz
			if(m_nAudioFrameDuration90Khz)
			{
				PTS_Base = (PTS_Base + (m_nAudioFrameDuration90Khz>>1))/
						m_nAudioFrameDuration90Khz;
				PTS_Base *= m_nAudioFrameDuration90Khz;
			}
			if(PTS_Base <= m_nCurrAudioTime)
			{
				PTS_Base += m_nAudioFrameDuration90Khz;
			}
			if(PTS_Base <= m_nCurrAudioTime)
			{
				MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
						" Audio PTS Delta 0 or negative Drop");
				return MUX_OUTDATED;
			}
			m_nCurrAudioTime = PTS_Base;
		}
	}

	// call TZ sample_process that will write the PAT/PMT/PCR/user_data
	// to the secure output handle and will mux the video/audio sample to the output handle

	smux_sample_info_t sample_params;
	memset(&sample_params, 0, sizeof(sample_params));
    sample_params.u8IsSync = static_cast<uint8_t>(sample_info->sync);
	sample_params.u64PTSBase = PTS_Base;
	sample_params.u8IsUserData = (uint8_t)m_is_user_data;
	sample_params.u8IsPCRData = (uint8_t)m_is_PCR_data;
	sample_params.userData.length = m_userdata_handler ? m_userdata_handler->sbuf_len : 0;
	sample_params.inputSample.length = ((struct smux_ion_info*)sample_data)->sbuf_len;
	sample_params.inputSample.buff_chunks->length = sample_params.inputSample.length;
	sample_params.u8StartOfPES = 1;
	sample_params.u32InOffset = 0;
    sample_params.u8ExtraLen = static_cast<uint8_t>(sample_info->extra_data_size - sample_info->extra_data_offset);
	sample_params.nSize = ((struct smux_ion_info*)sample_data)->sbuf_len;
    sample_params.nStreamNum = (uint32_t)stream_number;
	sample_params.u8NsDbgFlag = m_Params->debug_flags;
	if(sample_info->extra_data_ptr)
	{
		memcpy(sample_params.aExtra, sample_info->extra_data_ptr + sample_info->extra_data_offset,
				sample_info->extra_data_size - sample_info->extra_data_offset);
	}
    uint32_t sample_size = (uint32_t)sample_info->size;
	uint32_t input_offset = 0;
	uint32_t output_offset = 0;
	uint32_t userdata_offset = 0;
	uint32_t pcr_offset = m_table_offset;
	smux_status_t ret;

	// call the sample_process command in TZ each call with one output buffer
	while (sample_size > 0)
	{
		Transcode* transcode = (Transcode*)m_output_handle.transcode.pTranscode;
		struct smux_ion_info* output_handler = (struct smux_ion_info*)((uint64)transcode->GetBuffer());

		// wait for new available output buffer
	  	for(int i = 0; (i < 100) && (!output_handler); i++)
		{
			usleep(100);
			output_handler = (struct smux_ion_info*)((uint64)transcode->GetBuffer());
		}

		if(!output_handler)
		{
			MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "No output buffer");
			return MUX_FAIL;
		}

		if(output_handler->sbuf_len > (SMUX_PHYSICAL_CHUNK_LEN * SMUX_MAX_PHYSICAL_CHUNKS))
		{
			MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Output buffer length too big ");
			return MUX_INVALID;
		}

		sample_params.outputBuffer.length = output_handler->sbuf_len;
		sample_params.outputBuffer.buff_chunks->length = output_handler->sbuf_len;
		sample_params.u32InOffset = input_offset;
		sample_params.u32UserDataOffset = userdata_offset;
		sample_params.u32PCROffset = pcr_offset;
		sample_params.u32OutOffset = 0;

		ret = SMUX_Process_Sample(securemux_handler, &sample_params,
				(struct smux_ion_info*)sample_data, output_handler, m_userdata_handler,
				&input_offset,&pcr_offset, &userdata_offset,&output_offset);

		// check if we finish writing the PAT/PMT/PCR packets to the output
		if(m_is_PCR_data)
		{
			if(m_PAT_PMT_PCR_handler.sbuf_len == pcr_offset)
			{
				m_is_PCR_data = false;
				sample_params.u8IsPCRData = false;
			}
		}

		// check if we finish writing the user metadata packets to the output
		if(m_userdata_handler && m_is_user_data)
		{
			if(m_userdata_handler->sbuf_len == userdata_offset)
			{
				m_is_user_data = false;
				sample_params.u8IsUserData = false;
			}
		}

		// send the output buffer to the Transcode class
		transcode->SendCommand((void*)output_handler, output_offset);

		switch(ret)
		{
			case SEC_MUX_BUFFER_FULL:
				break;
			case SEC_MUX_SUCCESS:
				return MUX_SUCCESS;
			case SEC_MUX_ERR_SECURITY_FAULT:
				return MUX_SECURITY_FAULT;
			default:
				return MUX_FAIL;
		}

		sample_params.u8StartOfPES = 0;
        sample_size = (uint32_t)(sample_size - (uint32_t)sample_info->size - input_offset);
	}

	return MUX_SUCCESS;
}
/*===========================================================================

FUNCTION PCR_callback

DESCRIPTION
  This function replace avc start codecs with NAL sizes.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

===========================================================================*/
void MP2BaseFileSecure::PCR_callback(void *pMp2File)
{
  MP2BaseFileSecure *pMP2BaseFileSecure = (MP2BaseFileSecure*)pMp2File;
  MUX_sample_info_type sSampleInfo;

  memset(&sSampleInfo, 0, sizeof(sSampleInfo));

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFileSecure:: SendPCR");

  pMP2BaseFileSecure->m_bGenerateTables = true;

  struct timespec tempTime;
  clock_gettime(CLOCK_MONOTONIC, &tempTime);
  uint64 lTime = ((uint64)tempTime.tv_sec * 1000000000) +
                       (uint64)(tempTime.tv_nsec);

  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                "Timer PCR %lu %lu",
               (unsigned long)tempTime.tv_sec,
               (unsigned long)tempTime.tv_nsec);

  lTime -= pMP2BaseFileSecure->m_llTimeBase;

  sSampleInfo.time = (uint64)lTime / 1000000000;
  sSampleInfo.delta =(uint64)lTime -
                       ((uint64)sSampleInfo.time * 1000000000);

  pMP2BaseFileSecure->GenerateMP2TSPCRPacket(&sSampleInfo);
  m_is_PCR_data =  true;
  return;
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
MUX_STATUS MP2BaseFileSecure::MUX_end_Processing()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFileSecure::MUX_end_Processing");
  return MUX_SUCCESS;
}

/*===========================================================================

FUNCTION  ~MP2BaseFileSecure

DESCRIPTION
Destructor for the MP2BaseFileSecure class

===========================================================================*/
MP2BaseFileSecure::~MP2BaseFileSecure()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFileSecure::~MP2BaseFileSecure");

  if(smux_shutdown_app(&securemux_handler) < 0)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " smux_shutdown_app Failed");
  }
  securemux_handler = NULL;

  /* release PAT/PMT/PCR/userdata ION buffers*/
  if(smux_ion_dealloc(&m_PAT_PMT_PCR_handler) != 0)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " Failed deallocate PCR/PAT/PMT ION buffer");
  }

  if(m_userdata_handler != NULL)
  {
    if(smux_ion_dealloc(m_userdata_handler) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " Failed deallocate User data ION buffer");
    }
    delete m_userdata_handler;
    m_userdata_handler = NULL;
  }

  if(m_pAVCHrdDescr)
  {
    MM_Free(m_pAVCHrdDescr);
    m_pAVCHrdDescr = NULL;
  }

  if(m_Params)
  {
    if(m_Params->streams)
    {
      MM_Free(m_Params->streams);
    }
    MM_Free(m_Params);
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
void MP2BaseFileSecure::GeneratePATPacket()
{
  uint8 loop_index = 0;
  int index = 0;
  uint32 nCRC =0;
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFileSecure::GeneratePATPacket");

  uint8* m_MP2TSPacket = m_PAT_PMT_PCR_handler.ion_sbuffer;
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

    m_MP2TS_table_continuty_counter[0] = static_cast<uint8>(m_MP2TS_table_continuty_counter[0] + 1);

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
    for( loop_index = (uint8)index; loop_index < TS_PKT_SIZE; loop_index++)
    {
      m_MP2TSPacket[loop_index] = 0XFF;
    }
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
void MP2BaseFileSecure::GeneratePMTPacket()
{
  int i;
  int index = 0;
  uint8 section_length = MUX_MP2_PMT_FIX_SIZE;
  uint32 nCRC = 0;

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFileSecure::GeneratePMTPacket");

  uint8* m_MP2TSPacket = m_PAT_PMT_PCR_handler.ion_sbuffer + TS_PKT_SIZE;

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
          section_length = (uint8)(section_length + m_nAVCHrdDescrLen);
        }
      }
    }
    if(m_bAudioPresent)
    {
     // A+V streams then the section length would be 18 + 5 = 23
      section_length = (uint8)(section_length + 5);
      if(m_Params->streams[m_audio_stream_num].subinfo.audio.format ==
             MUX_STREAM_AUDIO_MPEG4_AAC && m_eAACFormat == SEC_AAC_FORMAT_LOAS)
      {
          section_length = (uint8)(section_length + 3);
      }
      else if(m_Params->streams[m_audio_stream_num].subinfo.audio.format ==
             MUX_STREAM_AUDIO_PCM)
      {
          section_length = (uint8)(section_length + 4);
      }
     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "m_bAudioPresent--> section_length ");
    }
	  /* If HDCP registration descriptor present, add this to section length */
    if((m_Params->encrypt_param.streamEncrypted == TRUE)  &&
       (m_Params->encrypt_param.type == MUX_ENCRYPT_TYPE_HDCP))
    {
      /* Adding 7 byte header in descriptor section */
      section_length = (uint8)(section_length + 7);
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
   // uint16 pcr_pid = 0x1000;

    m_MP2TSPacket[index] |= (uint8)(m_Params->pcr_pid >> 8);
    index += 1;
    m_MP2TSPacket[index] |= (uint8)(m_Params->pcr_pid & 0xFF);
    index += 1;

    /* If there are any descriptors present add the descriptors and program_info_length */
    if((m_Params->encrypt_param.streamEncrypted == TRUE)  &&
       (m_Params->encrypt_param.type == MUX_ENCRYPT_TYPE_HDCP))
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
                    "GeneratePMTPacket() setting the HDCP registration descriptor");
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
        if(m_eAACFormat == SEC_AAC_FORMAT_ADTS)
        {
           m_MP2TSPacket[index] |= 0x0F;
        }
        else
        {
           m_MP2TSPacket[index] |= 0x11;
        }
      }
      else //PCM
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
         m_eAACFormat == SEC_AAC_FORMAT_LOAS)
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
    m_MP2TSPacket[index] |= static_cast<uint8>(nCRC & 0xFF);
    index += 1;


    /* Fille the remaining TS packet bytes with FF */
    for( i=index; i<TS_PKT_SIZE; i++)
    {
      m_MP2TSPacket[i] = 0XFF;
    }
  }
}

/*===========================================================================

FUNCTION GenerateAVCHRDTimingDescriptor

DESCRIPTION
 This function is used to generate AVC HRD params.
 HRD params used to generate the HRD descriptor to be sent in PMT.

DEPENDENCIES
  pHRDParams - pointer to HRD params structure.

RETURN VALUE
  bool.

SIDE EFFECTS
  none

===========================================================================*/
bool MP2BaseFileSecure::GenerateAVCHRDTimingDescriptor
(
  MUX_AVC_TimingHRD_params_type *pAVCHrdParams
)
{
  uint8 index = 0;

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

  m_nAVCHrdDescrLen = index;

  return true;
}

/*===========================================================================

FUNCTION GenerateMP2TSPCRPacket

DESCRIPTION
  Generate TS PCR packets for MPG2 stream

DEPENDENCIES
  None.

RETURN VALUE
  no of bytes consumed from the original sample

SIDE EFFECTS
  none

===========================================================================*/
uint32 MP2BaseFileSecure::GenerateMP2TSPCRPacket(MUX_sample_info_type  *sample_info)
{
  int index = 0;
  uint8 sample_data_offset = 0;
  uint8 bytes_to_copy = 0;

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "MP2BaseFileSecure::GenerateMP2TSPCRPacket");

  uint8* m_MP2PCRTSPacket = m_PAT_PMT_PCR_handler.ion_sbuffer + 2*TS_PKT_SIZE;

  if(m_MP2PCRTSPacket)
  {
  	memset(m_MP2PCRTSPacket, 0, TS_PKT_SIZE);
  	// TS stream sync byte  - 8 bits
  	m_MP2PCRTSPacket[index] = TS_PKT_SYNC_BYTE;
  	index += 1;

  	m_MP2PCRTSPacket[index] |= 0X40;

  	// Transport_priority always set to 1 for I frames and 0 for all other frames - 1 bit
  	m_MP2PCRTSPacket[index] |= 0X20;

  	uint16_t pcr_pid = m_Params->pcr_pid;

  	m_MP2PCRTSPacket[index] |= (uint8_t)(pcr_pid >> 8);
  	index += 1;

  	m_MP2PCRTSPacket[index] |= (uint8_t)(pcr_pid);
  	index += 1;

  	// Transport scrambling control - 00 - 2 bits
  	m_MP2PCRTSPacket[index] |= 0X00;

  	uint8 nAdaptationControl = MUX_MP2_ADAPTATON_AND_PAYLOAD;

  	// Adaptation_field_control - 2 bits
    m_MP2PCRTSPacket[index] |= (uint8)(nAdaptationControl << 4);

  	m_MP2PCRTSPacket[index] |= m_MP2TS_table_continuty_counter[2];
    m_MP2TS_table_continuty_counter[2] = static_cast<uint8>((m_MP2TS_table_continuty_counter[2] + 1) % 16);
  	index += 1;

  	//Adaptation field

  	// adaptation_field_length  - 8 bits
  	m_MP2PCRTSPacket[index] = 0X07;

  	index += 1;
  	//Discontinuity indicator - 1 bit - 0
  	m_MP2PCRTSPacket[index] |= 0X00;

  	//random_access_indicator - 00 - 1 bit
  	m_MP2PCRTSPacket[index] |= 0X00;

  	//elementary_stream_priority_indicator - 1-bit - same priority
  	m_MP2PCRTSPacket[index] |= 0X20;

  	//PCR_flag  - 1 bit  - 0
  	//OPCR_flag - 1 bit  - 0
  	//splicing_point_flag - 1 bit - 0
  	//transport_private_data_flag - 1 bit - 0
  	// adaptation_field_extension_flag - 1 bit  - 0
  	m_MP2PCRTSPacket[index] |= 0X10;

  	index += 1;

  	// Note that for PCR sample info comes in this format. sample_info->time in
  	// seconds and sampleinfo->delta in nano seconds
  	uint64 PCR_base = (uint64)sample_info->time * 90000 +  //sec to 90Khz
  			(uint64)(((uint64)sample_info->delta/1000) * 90) / 1000;

  	uint64 PCR_27MHz = (uint64)((uint64)sample_info->delta * 27)/1000;

  	uint64 PCR_Ext = (uint64)PCR_27MHz % 300;

  	MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_HIGH,
  			"PCR Incoming sec  %u nsec %u 27Mhz %u Ext %u",
  			(unsigned int)sample_info->time,
  			(unsigned int)sample_info->delta,
  			(unsigned int)PCR_27MHz,
  			(unsigned int)PCR_Ext);


  	if((int64)(PCR_base - m_nCurrPCR) <= 0)
  	{
  		// If PCRBase goes behing current PCR, Stop it at current PCR
  		// This will only happen if video timestamp goes out of order
  		// which will not happen as long as B frames are not supported
  		// Revisit for B frame.
  		PCR_base = m_nCurrPCR;
  	}

  	if(PCR_base - m_nCurrPCR > 190 * 27000)
  	{
  		MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,
  				"PCR is late curr %d prev %d",
  				(unsigned int)PCR_base, (unsigned int)m_nCurrPCR );
  	}
  	m_nCurrPCR = PCR_base;

  	MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "Timer PCR SI %d - %d",
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
    m_MP2PCRTSPacket[index] = (uint8)(m_MP2PCRTSPacket[index] | ((PCR_Ext >> 8) & 0x1));
  	index += 1;
  	m_MP2PCRTSPacket[index] |= (uint8)(PCR_Ext);
  	index += 1;


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
  			if(!m_MP2PCRTSPacket[4])
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

  }
  return 0;
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
uint32 MP2BaseFileSecure::FindCheckSum(uint8* pData, uint32 nSize)
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

