#ifndef __AAC_PARSER_H__
#define __AAC_PARSER_H__

/* =======================================================================
                              aacParser.h
DESCRIPTION

Copyright 2011-2014 Qualcomm Technologies Incorporated, All Rights Reserved.
QUALCOMM Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AACParserLib/main/latest/inc/aacparser.h#27 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $

========================================================================== */

#include <stdio.h>
#include <stdlib.h>

#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "iaudioparser.h"
#include "oscl_file_io.h"
#include "AEEStdDef.h"

#include "aacconstants.h"
#include "aacheaders.h"
#include "aacmetadata.h"
#include "filesourcetypes.h"

#define OUTPUT_AAC_ON_FRAME_BOUNDARY

//======================================================================
//Foraward declarartion
//====================================================================
class simple_seektable;
class seek;

/*
*Callback function used by parser for reading the file data.
*Parser does not implement this function, it's the responsibility
*of the APP to implement it.
*/

extern uint32 AACCallbakGetData (uint64 ullOffset,
                                 uint32 ulNumBytesRequest,
                                 uint8* pucData,
                                 uint32 ulMaxBufSize,
                                 void*  pUserData,
                                 bool   &bEndofdata);
/*
*aacParser class.
*/
class aacParser : public IAudioParser
{
friend class AACFile;
/*=======================================================================
* Public Members                                                        *
* ======================================================================*/

  public:
  //aacParser APIs
                         aacParser(void* pClientData,
                                   uint64 fsize,
                                   OSCL_FILE *FilePtr,
                                   bool bHttpStreaming = false);

  virtual                ~aacParser();

  virtual PARSER_ERRORTYPE   StartParsing(void);
  virtual PARSER_ERRORTYPE   GetAACHeader(aac_header_aach* pAacHdrPtr);
  virtual PARSER_ERRORTYPE   GetAudioInfo(aac_audio_info* pAudioInfo);

  virtual PARSER_ERRORTYPE   GetCurrentSample(uint8* dataBuffer,
                                              uint32 nMaxBufSize,
                                              uint32 *nBytesNeeded,
                                              bool bUpdateTime = true);

  virtual bool               GetBaseTime(uint64* nBaseTime);
  virtual bool               SetBaseTime(uint64 nBaseTime);
  virtual PARSER_ERRORTYPE   Seek(uint64 nReposTime, uint64 *nSeekedTime);
  virtual uint8              RandomAccessDenied();
  virtual uint64             GetClipDurationInMsec();
  virtual PARSER_ERRORTYPE   GetApproxDuration(uint64 *pDuration);
  virtual uint64             GetCurrentTime(){return m_nCurrentTime;};
  virtual uint64             getMediaTimestampForCurrentSample(uint32 id);

  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum);
  virtual FileSourceStatus GetAudioOutputMode(bool*,FileSourceConfigItemEnum);
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
                {return (void*)m_aac_metadata->get_id3v1();};
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
                {return (void*)m_aac_metadata->get_id3v2(ulIndex);};

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
                {return m_aac_metadata->getTotalID3V2Entries();};

  virtual PARSER_ERRORTYPE  GetAACDecodeInfo(aac_decode_info* pAACDecodeinfo);
  virtual PARSER_ERRORTYPE  GetTrackDecoderSpecificInfoContent(uint8*,uint8*);

  virtual IAudioReturnType parse_frame_header (byte* frame,
                                               uint32* frame_size,
                                               uint32* frame_time);

  virtual PARSER_ERRORTYPE parse_aac_frame_header (uint8 *frame,
                                                   aac_format_type format,
                                                   uint32* frame_size,
                                                   uint32* frame_time);

  virtual void set_newfile_position(uint64 file_position);
  virtual void init_file_position();

  /*=======================================================================
  * Private Members                                                       *
  * ======================================================================*/
private:
  //Class object pointers
  simple_seektable *psimple_seektable;
  seek *pseek;

  AudioTrack      m_audio_track; ///< audio track file location
  aacmetadata*    m_aac_metadata; ///< aac metadata information
  aac_format_type m_aac_format; ///< AAC format

  OSCL_FILE*   m_AACFilePtr;
  int64        m_filereadpos;
  void*        m_pUserData;
  uint64       m_nCurrOffset;
  uint64       m_nFileSize;
  uint8        m_adts_fix_hdr[AAC_ADTS_FIX_HDR_SIZE];
  uint8        m_ReadBuffer[AAC_READ_BUFFER_SIZE];

  uint64       m_aac_duration;//duration of the entire clip
  uint64       m_nCurrentTime;
  uint64       m_nNextFrameTime;
  uint64       m_baseTS;
  uint64       m_firstFrameTS;

  ParserStatusCode m_CurrentParserState;

  //boolean variables
  bool m_isFirstTSValid;
  bool m_SeekDone;
  bool m_seek_function_defined;
  bool m_bCRCPresent;
  bool aac_file_format;
  bool m_id3tagfound;
  bool m_firstFrame;
  bool m_bFixedADTSHdrSet;
  bool m_bEndOfData;
  bool m_id3tagparsed;
  bool m_bHTTPStreaming;

  //Current output mode
  FileSourceConfigItemEnum m_hFrameOutputModeEnum;
  FileSourceConfigItemEnum m_hHeaderOutputModeEnum;

  //Pointer to the AAC header structure
  aac_header_aach m_aac_header_aach;
  aac_audio_info m_aac_audio_info;
  uint32 m_n_adif_hdr_len;

  //private class methods
  void parse_aac_file_header();
  void parse_aac_audio_data();
  uint32 seekandreadfile (uint32 length, int64 position, uint8 *pbuffer,
                          uint32 nbuffmaxsize);

  aac_format_type getaacformattype (const uint8 *file_header) const;

  PARSER_ERRORTYPE parse_adts_frame_header (uint8* frame,
                                            uint32* frame_size,
                                            uint32* frame_time);
  PARSER_ERRORTYPE parse_adts_file_header ( );
  PARSER_ERRORTYPE parse_loas_file_header (uint64 noffset = 0,
                                           uint32* framesize = NULL,
                                           uint32* framehdr = NULL,
                                           uint32* frametime = NULL);
  PARSER_ERRORTYPE parse_adif_file_header ( );
  PARSER_ERRORTYPE parse_generic_file_header();
  PARSER_ERRORTYPE parse_file_header ();
  PARSER_ERRORTYPE parse_frame_metadata ();
  PARSER_ERRORTYPE parse_id3();

  uint32     getbitsfrombuffer(uint32 uNeededBits, uint32 uOffset,
                                uint8 * pucInputBuffer, uint32 const uMaxSize);

  PARSER_ERRORTYPE  get_accurate_seek_position (/*in*/ uint64 RequestedTime,
                                                /*rout*/ uint64* FilePosition,
                                                /*rout*/ uint64* SeekedTime);
  PARSER_ERRORTYPE get_seek_position (/*in*/ uint64 time,
                                      /*rout*/ uint64* file_position);
  bool             is_aac_sync(const uint8* frame) const ;

  bool is_aac_format();

};
#endif
