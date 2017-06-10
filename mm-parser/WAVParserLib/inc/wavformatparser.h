#ifndef WAV_PARSER_H
#define WAV_PARSER_H

/* =======================================================================
                              wavparser.h
DESCRIPTION

Copyright (c) 2009-2014 QUALCOMM Technologies Inc, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/WAVParserLib/main/latest/inc/wavformatparser.h#16 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $

========================================================================== */

//=============================================================================
// INCLUDES
//=============================================================================
#include "AEEStdDef.h"
#include "wavconstants.h"
#include "wavheaders.h"
#include "parserdatadef.h"
#include "iaudioparser.h"
#include "filebase.h"

#ifdef FEATURE_FILESOURCE_WAVADPCM
/* Callback function used by parser for reading the file data.
   Parser does not implement this function, it's the responsibility
   of the APP to implement it.*/
extern uint32 WAVCallbakGetData (uint64 ullOffset,
                                 uint32 ulNumBytesRequest,
                                 uint8* pucData,
                                 uint32 ulMaxBufSize,
                                 void*  pUserData );

extern void set_data_offset(uint32 t_valid_data_offset);

class wavformatParser : public IAudioParser
{
  friend class WAVFile;
  public:
    //Constructor
    wavformatParser(void* pUserData,uint64 fsize,OSCL_FILE *FilePtr);
    //Distructor
    virtual  ~wavformatParser();
    //Starts the parsing process
    virtual  PARSER_ERRORTYPE  StartParsing(void);
    //Reads WAV header data
    virtual  PARSER_ERRORTYPE  GetWAVHeader(wav_header_wavh* pPcmHdrPtr);
    //Reads WAV audio info
    virtual  PARSER_ERRORTYPE  GetAudioInfo(wav_audio_info* pAudioInfo);
    //Reads WAV format chunk
    virtual PARSER_ERRORTYPE  GetFormatChunk(tech_data_wav*);
    //Calculates and returns the playback duration
    virtual  uint64  GetClipDurationInMsec();
    //Gets next sequence of sample for playback
    virtual  PARSER_ERRORTYPE  GetCurrentSample(uint8* dataBuffer,
                                                uint32 nMaxBufSize,
                                                uint32 *nBytesNeeded);

   //parsing the frame and calculates frame time and frame size
   virtual IAudioReturnType  parse_frame_header (uint8* /*frame*/,
                                                 uint32* /*frame_size*/,
                                                 uint32* /*frame_time*/)
                             {return IAUDIO_NOT_SUPPORTED;};

    //Seeks to the file position for the corresponding time
    virtual  uint64  Seek(uint64 nReposTime, uint32 *ValidDataOffset);
    //sets the new file position in file port
    virtual void  set_newfile_position(uint64);
    //resets the file pointer to initial position
    virtual void  init_file_position();
    //Updates the decoded bytes
    virtual void ActualBytesDecoded(uint32 t_DecodeBytes);
    //returns the wav format subtype
    virtual void get_wav_subtype(uint32 *);
    //returns nonzero if seek is not supported
    virtual uint8 RandomAccessDenied();
    virtual uint64 GetCurSampleTime() {return m_nCurrStartTime;};

  private:
    //Pointer to OSCL File interface
    OSCL_FILE  *m_WAVFilePtr;
    //User callback data
    void*   m_pUserData;
    //Pointer to wave file
    uint32  m_BytesAfterDecode;
    //This is the offset in 'movi' used for retrieval of audio/video samples.
    uint64  m_nCurrOffset;
    uint64 m_nCurrStartTime;
    //Total file size
    uint64  m_nFileSize;
    //Maximum buffer size
    uint32  m_maxBufferSize;
    //Total playback duration
    uint64  m_duration;
    //Current parser state
    ParserStatusCode  m_CurrentParserState;
    //Data read buffer size
    uint8  m_ReadBuffer[WAV_READ_BUFFER_SIZE];
    //WAV header structure
    wav_header_wavh  m_wav_header_wavh;
    //WAV audio info structure
    wav_audio_info  m_wav_audio_info;
    //audio track file location
    AudioTrack  m_audio_track;
    //WAV header info
    tech_data_wav  m_header_info;
    //Wave format
    wav_format_type  m_wav_format;
    //impcm adpcm format specific structure info
    ima_adpcm_data  m_ima_adpcm_data;
    //wav metadata information
    tech_data_wav m_wav_tech_metadata;

    /* Private Class Methods */
    //Read RIFF chunk
    PARSER_ERRORTYPE  read_riff_chunk ( uint32 *chunk_size);
    //Read FMT chunk
    PARSER_ERRORTYPE  read_fmt_chunk ( uint32 fmt_size);
    //parse the entire file
    PARSER_ERRORTYPE parse_file_header ();
    //parse wav file header
    void  parse_wav_file_header ();
    //parse wav audio data
    void  parse_wav_audio_data ();
    //updates adpcm tag info
    void  update_adpcm_tag_info ();
    //calculates total playback duration
    PARSER_ERRORTYPE  get_duration (uint64* time);
    //get new file position corresponding to the given time
    PARSER_ERRORTYPE  get_seek_position (uint64 time, uint32* file_position);
    //Calculates redundant PCM samples
    PARSER_ERRORTYPE calc_redundant_pcm(uint32* file_position,
                                        uint32* data_offset);
};
#endif //FEATURE_FILESOURCE_WAVADPCM
#endif //WAV_PARSER_H
