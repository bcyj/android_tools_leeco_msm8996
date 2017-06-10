#ifndef QCP_PARSER_H
#define QCP_PARSER_H

  /* =======================================================================
                                qcpParser.h
  DESCRIPTION

  Copyright 2009-2014 Qualcomm Technologies, Inc., All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/QCPParserLib/main/latest/inc/qcpparser.h#18 $
  $DateTime: 2014/02/07 02:53:30 $
  $Change: 5240686 $

  ========================================================================== */

//=============================================================================
// INCLUDES
//=============================================================================
#include "qcpconstants.h"
#include "parserdatadef.h"
#include "qcpheaders.h"
#include "qcpfile.h"
#include "filebase.h"
#include "iaudioparser.h"
#include "seektable.h"
#include "seek.h"
#include "filesourcetypes.h"

//======================================================================
//Foraward declarartion
//====================================================================
class simple_seektable;
class seek;

/* Callback function used by parser for reading the file data.
   Parser does not implement this function, it's the responsibility
   of the APP to implement it. */

extern uint32 QCPCallbakGetData (uint64 ullOffset,
                                 uint32 ulNumBytesRequest,
                                 uint8* pData,
                                 uint32 ulMaxBufSize,
                                 void*  pUserData );

//
//qcpParser class.
//
class qcpParser : public IAudioParser
{
//=======================================================================
//Public Members
//=======================================================================
  public:
//qcpParser APIs
  qcpParser(void* pUserData, uint64 fsize, OSCL_FILE *FilePtr);

  virtual  ~qcpParser();

  virtual PARSER_ERRORTYPE   StartParsing(void);
  virtual PARSER_ERRORTYPE   GetQCPHeader(qcp_header_qcph* pQcpHdrPtr);
  virtual PARSER_ERRORTYPE   GetAudioInfo(qcp_audio_info* pAudioInfo);
  virtual uint64             GetClipDurationInMsec();
  virtual PARSER_ERRORTYPE   GetApproxDuration(uint64 *pDuration);

  virtual PARSER_ERRORTYPE   GetCurrentSample(uint8* dataBuffer,
                                              uint32 nMaxBufSize,
                                              uint32 *nBytesNeeded);

  virtual uint64   Seek(uint64 nReposTime);
  virtual uint8        RandomAccessDenied();
  virtual FileSourceStatus SetAudioOutputMode(FileSourceConfigItemEnum henum);
  virtual FileSourceStatus GetAudioOutputMode(bool* bret,
                                              FileSourceConfigItemEnum henum);

  PARSER_ERRORTYPE GetQCPDecodeInfo(qcp_decode_info* pQCPDecodeinfo);
  boolean get_qcp_subtype();
  uint64 GetCurrentTime(){return m_nCurrTime;};

//=======================================================================
//Private Members
//=======================================================================
  private:

  void*              m_pUserData;   /* User callback data */
  uint64             m_nCurrOffset; /* Current file offset */
  uint64             m_nFileSize; /* Total file size */
  ParserStatusCode   m_CurrentParserState; /* Current parser state*/
  uint8              m_ReadBuffer[QCP_READ_BUFFER_SIZE];
  uint8*             m_dataBuffer;
  uint64             m_qcp_duration;/* duration of the entire clip */
  uint64             m_nCurrTime; /* current playback time */
  AudioTrack         m_audio_track; /* audio track properties */
  boolean            m_seek_function_defined;/* flag to enable seek support*/
  OSCL_FILE*         m_QCPFilePtr; /* File pointer */
  simple_seektable*  m_psimple_seektable; /* Seek table object*/
  seek*              m_pseek; /* Seek class object*/

  /* QCP Format related parameters */
  qcp_format_type   m_qcp_format; /* QCP format ENUM */
  qcp_header_qcph   m_qcp_header_qcph;
  qcp_audio_info    m_qcp_audio_info;
  tech_data_qcp     m_qcp_tech_metadata;

  //Current output mode
  FileSourceConfigItemEnum m_hOutputModeEnum;

  //private class methods
  void parse_qcp_file_header();
  void parse_qcp_audio_data();
  PARSER_ERRORTYPE get_seek_function_defined();
  PARSER_ERRORTYPE calculate_get_duration(uint64* uDuration);
  PARSER_ERRORTYPE parse_file_header();
  PARSER_ERRORTYPE read_riff_chunk(uint32 *chunk_size);
  PARSER_ERRORTYPE read_fmt_chunk(uint32 *chunk_size);
  PARSER_ERRORTYPE read_vrat_chunk(uint32 *offset, uint32 *chunk_size);
  PARSER_ERRORTYPE read_labl_chunk(uint32 *offset, uint32 *chunk_size);
  PARSER_ERRORTYPE read_offs_chunk(uint32 *offset, uint32 *chunk_size);
  PARSER_ERRORTYPE read_data_chunk(uint32 *offset, uint32 *chunk_size);
  PARSER_ERRORTYPE read_cnfg_chunk(uint32 *offset, uint32 *chunk_size);
  PARSER_ERRORTYPE read_text_chunk(uint32 *offset, uint32 *chunk_size);
  PARSER_ERRORTYPE get_seek_position (uint64 time,uint64* file_position);

public:
  virtual IAudioReturnType parse_frame_header (uint8* frame, uint32* frame_size,
                                               uint32* frame_time);
  virtual void set_newfile_position(uint64 file_position);
  virtual   void init_file_position();

};
#endif
