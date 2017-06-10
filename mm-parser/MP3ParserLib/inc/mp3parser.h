#ifndef MP3_PARSER_H
#define MP3_PARSER_H

/* =======================================================================
                              mp3Parser.h
DESCRIPTION

Copyright 2009-2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/inc/mp3parser.h#30 $
========================================================================== */
#include <stdio.h>
#include <stdlib.h>

#include "filebase.h"
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "mp3headers.h"
#include "oscl_file_io.h"
#include "iaudioparser.h"
#include "mp3consts.h"
#include "mp3metadata.h"
#include "id3.h"

//======================================================================
//Foraward declarartion
//====================================================================
class simple_seektable;
class seek;
class mp3vbrheader;

/*
*Callback function used by parser for reading the file data.
*Parser does not implement this function, it's the responsibility
*of the APP to implement it.
*/

extern uint32 MP3CallbakGetData (uint64 ullOffset,
                                 uint32 ulNumBytesRequest,
                                 uint8* pucData,
                                 uint32 ulMaxBufSize,
                                 void*  pUserData,
                                 bool   &bEndOfData);
/*
*mp3Parser class.
*/
class mp3Parser : public IAudioParser
{
friend class MP3File;
public:
                         mp3Parser(void*,uint64,OSCL_FILE*,
                                   bool bHTTPStream = false);
  virtual                ~mp3Parser();

  virtual PARSER_ERRORTYPE   StartParsing(void);
  virtual PARSER_ERRORTYPE   GetMP3Header(mp3_header_mp3h* pMp3HdrPtr);
  virtual PARSER_ERRORTYPE   GetAudioInfo(mp3_audio_info* pAudioInfo);
  virtual PARSER_ERRORTYPE   GetCurrentSample(uint8*  dataBuffer,
                                              uint32  nMaxBufSize,
                                              uint32* nBytesNeeded,
                                              bool bUpdateTime = true);
  virtual PARSER_ERRORTYPE   Seek(uint64 nReposTime, uint64 *nSeekedTime);
  virtual IAudioReturnType   parse_frame_header (uint8* frame,
                                                 uint32* frame_size,
                                                 uint32* frame_time);
  virtual PARSER_ERRORTYPE   parse_mp3_frame_header (const uint8* frame,
                                   struct tech_data_mp3 &hdr_ptr) const;
  virtual PARSER_ERRORTYPE   GetMP3DecodeInfo(mpeg1_tag *mpeg1_tag_info);

  virtual bool   GetBaseTime(uint64* nBaseTime);
  virtual bool   SetBaseTime(uint64 nBaseTime);

  /* ======================================================================
  FUNCTION:
    is_mp3_format

  DESCRIPTION:
    Called by user to check whether input is mp3 complaint or not

  INPUT/OUTPUT PARAMETERS:
    None

  RETURN VALUE:
   TRUE if it is MP3 complaint, else returns FALSE

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual bool   is_mp3_format() {return m_parse_file_header_done;};
  /* ======================================================================
  FUNCTION:
    set_newfile_position

  DESCRIPTION:
    Called by user to set new file offset in MP3 Parser object

  INPUT/OUTPUT PARAMETERS:
    ullOffset: Offset value

  RETURN VALUE:
   None

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual void   set_newfile_position(uint64 ullOffset)
                {m_nCurrOffset = ullOffset;};
  /* ======================================================================
  FUNCTION:
    init_file_position

  DESCRIPTION:
    Called by user to set the offset to first MP3 audio frame

  INPUT/OUTPUT PARAMETERS:
    None

  RETURN VALUE:
   None

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual void   init_file_position()
                {m_nCurrOffset = m_audio_track.start;};
  /* ======================================================================
  FUNCTION:
    get_id3v1_info

  DESCRIPTION:
    Called by user to get the ID3 V1 object pointer

  INPUT/OUTPUT PARAMETERS:
    None

  RETURN VALUE:
   ID3 v1 object Pointer

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual void*  get_id3v1_info()
                {return (void*)m_metadata->get_id3v1();};
  /* ======================================================================
  FUNCTION:
    get_id3v2_info

  DESCRIPTION:
    Called by user to get the ID3 V2 object pointer

  INPUT/OUTPUT PARAMETERS:
    ulIndex: Index whose object pointer is required

  RETURN VALUE:
   ID3 v2 object Pointer

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual void*  get_id3v2_info(uint32 ulIndex)
                {return (void*)m_metadata->get_id3v2(ulIndex);};
  /* ======================================================================
  FUNCTION:
    get_total_id3v2

  DESCRIPTION:
    Called by user to get the total number of ID3 V2 objects available

  INPUT/OUTPUT PARAMETERS:
    None

  RETURN VALUE:
   Total count of ID3 V2 objects available

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual uint32 get_total_id3v2()
                {return m_metadata->getTotalID3V2Entries();};
  /* ======================================================================
  FUNCTION:
    getSamplesPerFrame

  DESCRIPTION:
    Called by user to get the samples per frame

  INPUT/OUTPUT PARAMETERS:
    ulTrackID: Track Id

  RETURN VALUE:
   samples per frame available

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual uint32 getSamplesPerFrame(uint32 /*ulTracID*/)
  {
    return MP3_SAMPLES_TABLE
      [(m_header_info.version == MP3_VER_1 ? 0 : 1)][m_header_info.layer];
  };
  /* ======================================================================
  FUNCTION:
    getBitrate

  DESCRIPTION:
    Called by user to get the bit rate

  INPUT/OUTPUT PARAMETERS:
    ulTrackID: Track Id

  RETURN VALUE:
   bit rate for the given track

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual uint32 getBitrate(uint32 /*ulTracID*/)
                {return m_header_info.bitrate;};
  /* ======================================================================
  FUNCTION:
    getMaxBitrate

  DESCRIPTION:
    Called by user to get the Max bit rate

  INPUT/OUTPUT PARAMETERS:
    ulTrackID: Track Id

  RETURN VALUE:
   Max bit rate for the given track

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual uint32 getMaxBitrate(uint32 /*ulTracID*/)
                {return m_header_info.max_bitrate;};

  virtual uint64 getMediaTimestampForCurrentSample(uint32 id);
  virtual uint64 GetClipDurationInMsec();
  /* ======================================================================
  FUNCTION:
    SetAudioOutputMode

  DESCRIPTION:
    Called by user to set output mode specified by henum

  INPUT/OUTPUT PARAMETERS:
    henum-Output mode

  RETURN VALUE:
   FILE_SOURCE_SUCCESS if successful in setting output mode else returns FILE_SOURCE_FAIL

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum);
  /* ======================================================================
  FUNCTION:
    GetAudioOutputMode

  DESCRIPTION:
    Called by user to retrieve output mode specified by henum

  INPUT/OUTPUT PARAMETERS:
    henum-Output mode

  RETURN VALUE:
   FILE_SOURCE_SUCCESS if successful in retrieving output mode else returns FILE_SOURCE_FAIL

  SIDE EFFECTS:
    None.
  ======================================================================*/
  virtual FileSourceStatus GetAudioOutputMode(bool*,FileSourceConfigItemEnum);

  virtual uint64 GetCurrentTime(){return m_nCurrentTime;};
  virtual uint32 GetEncoderDelay() {return m_ulEncoderDelay;};
  virtual uint32 GetPaddingDelay() {return m_ulPaddingDelay;};
  virtual void   GetMP3Metadata(tech_data_mp3 **pMetadata)
                  {*pMetadata = &(m_metadata->m_techmetadata);};

  const static uint8 m_header_size;

protected:
  // Preffered number of buffers for MP3
  uint8 m_pref_buf_num;
  // length of sync word of MP3 audio frame
  uint8 m_sync_len;
  // maximum possible frame size
  uint16 m_max_frame_size;
  // maximum number of frames in one output samples
  uint16 m_max_frames_in_output;

private:
  PARSER_ERRORTYPE parse_file_header ();
  PARSER_ERRORTYPE file_header_preprocessing (OSCL_FILE *m_MP3FilePtr);
  PARSER_ERRORTYPE parse_id3(OSCL_FILE *m_MP3FilePtr, uint64 length);
  PARSER_ERRORTYPE parse_LAMEtag();
  PARSER_ERRORTYPE get_duration (uint64* time) ;
  PARSER_ERRORTYPE file_header_postprocessing(OSCL_FILE *m_MP3FilePtr,
                                              uint64 first_sync_offset);
  PARSER_ERRORTYPE update_audio_track (uint64 first_sync_offset,
                                       AudioTrack &audio_track_info) const;
  PARSER_ERRORTYPE calc_playback_duration (uint64 &duration) const;
  PARSER_ERRORTYPE get_seek_position (uint64 time,uint64* file_position);
  PARSER_ERRORTYPE get_accurate_seek_position (/*in*/ uint64 RequestedTime,
                                               /*rout*/ uint64* FilePosition,
                                               /*rout*/ uint64* SeekedTime);

  bool   find_mp3_frame_sync (const uint8 *buffer, uint32 buf_len,
                              tech_data_mp3 &header_info) const;
  void   update_mpeg1_tag_info();
  bool   is_mp3_sync(const uint8* frame) const;
  uint64 calc_frame_time (const struct tech_data_mp3 &header_info);
  uint32 calc_frame_length (const struct tech_data_mp3 &header_info) const;
  uint32 calc_frame_length () const;

  simple_seektable* m_psimple_seektable;
  seek*             m_pseek;

  void*        m_pUserData;
  uint64       m_nCurrOffset;
  uint64       m_nFileSize;
  uint8        m_ReadBuffer[MP3_READ_BUFFER_SIZE];
  uint8*       m_dataBuffer;
  OSCL_FILE*   m_MP3FilePtr;

  ParserStatusCode   m_CurrentParserState;

  mp3vbrheader*   m_vbr_header;// VBR header information
  mp3_header_mp3h m_mp3_header_mp3h;
  mp3_audio_info  m_mp3_audio_info;
  mp3metadata*    m_metadata;
  AudioTrack      m_audio_track;
  tech_data_mp3   m_header_info;// MP3 header info
  mpeg1_tag m_mpeg1_tag_info;   //MP3 tag info

  //Time related
  uint64 m_duration;
  uint64 m_nCurrentTime;
  uint64 m_nNextFrameTime;
  uint64 m_firstFrameTS;
  uint64 m_baseTS;
  //! Residual time missed in milli-sec precision. It is in micro-sec units
  uint32 m_FrameTimeinUS;
  uint32 m_nFracResidualTs; //! Current sample fractional part time (micro-sec)
  bool m_SeekDone;
  bool m_isFirstTSValid;
  bool m_firstFrame;
  bool m_id3tagfound;
  bool m_id3tagparsed;
  bool m_bHTTPStreaming;
  bool m_bLAMETagAvailable;

  // true if MP3 format parser has parsed the file header
  bool m_parse_file_header_done;
  bool m_is_vbr;// true if file is VBR file
  bool m_seek_function_defined;

  //Current output mode
  FileSourceConfigItemEnum m_hFrameOutputModeEnum;
  FileSourceConfigItemEnum m_hHeaderOutputModeEnum;

  //Encoder and Padding delay
  uint32 m_ulEncoderDelay;
  uint32 m_ulPaddingDelay;
};
#endif
