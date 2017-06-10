#ifndef AMRWB_PARSER_H
#define AMRWB_PARSER_H

/* =======================================================================
                              amrwbParser.h
DESCRIPTION

  Copyright (c) 2009-2014 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRWBParserLib/main/latest/inc/amrwbparser.h#13 $
========================================================================== */
#include <stdio.h>
#include <stdlib.h>

#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "iaudioparser.h"
#include "AEEStdDef.h"

#include "amrwbheaders.h"
#include "oscl_file_io.h"
#include "amrwbconstants.h"
#include "filesourcetypes.h"

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

extern uint32 AMRWBCallbakGetData (uint64 ullOffset,
                                  uint32 ulNumBytesRequest,
                                  unsigned char* pucData,
                                  uint32 ulMaxSize,
                                  void* pUserData );
/*
*amrwbParser class.
*/
class amrwbParser : public IAudioParser
{
  /*=======================================================================
  * Public Members                                                        *
  * ======================================================================*/
  public:
  //amrwbParser APIs
                         amrwbParser(void* pUserData,
                                     uint64 fsize,
                                     OSCL_FILE *FilePtr);

  virtual                ~amrwbParser();

  virtual PARSER_ERRORTYPE   StartParsing(void);
  virtual PARSER_ERRORTYPE   GetAMRWBHeader(amrwb_header_amrwbh* pAmrHdrPtr);
  virtual PARSER_ERRORTYPE   GetAudioInfo(amrwb_audio_info* pAudioInfo);
  /*!
  @brief      Set Audio O/P Mode
  @details    This API will set audio buffer output mode based on passed
              FileSourceConfigItemEnum type.
  @param[out] eMode FileSourceConfigItemEnum Type
  @return     PARSER_ERRORTYPE status
              PARSER_ErrorNone if success
              PARSER_ErrorDeafult if fail
  @note       None
  */
  virtual PARSER_ERRORTYPE SetAudioOutputMode(FileSourceConfigItemEnum eMode);

  /*!
  @brief      Get Audio O/P Mode
  @details    This API will get audio buffer output mode set after parsing.
  @param[out] pbStatus Ptr to status variable
  @param[in]  eMode    Mode need to checked
  @return     PARSER_ERRORTYPE status
              PARSER_ErrorNone if success
              PARSER_ErrorDeafult if fail
  @note       None
  */
  virtual PARSER_ERRORTYPE GetAudioOutputMode(bool* pbStatus,
                                              FileSourceConfigItemEnum eMode);

  /*!
  @brief      Get Current Sample
  @details    This API will fill data information of current sample.
  @param[out] pucDataBuffer  Ptr to data buffer
  @param[in]  ulMaxBufSize   Data buffer size
  @param[out] pulBytesNeeded Bytes filled in data buffer
  @return     PARSER_ERRORTYPE status
              PARSER_ErrorNone if success
              PARSER_ErrorDeafult if fail
  @note       None
  */
  virtual PARSER_ERRORTYPE   GetCurrentSample(uint8* pucDataBuffer,
                                              uint32 ulMaxBufSize,
                                              uint32 *pulBytesNeeded);

  virtual uint64 GetClipDurationInMsec();
  virtual uint64 Seek(uint64 nReposTime);
  virtual void   set_newfile_position(uint64 file_position);
  virtual void   init_file_position();

  virtual IAudioReturnType parse_frame_header (byte* frame,
                                               uint32* frame_size,
                                               uint32* frame_time);
  virtual uint32 parse_amrwb_frame_header (uint8 frame,
                                           amrwb_format_type format,
                                           amrwb_header_type &header) const;
  virtual PARSER_ERRORTYPE parse_amrwb_fs(uint8 frame,
                                          amrwb_header_type &header) const;

  uint64 GetCurrentPbTime(){return m_ullCurrPbTime;};
  /*=======================================================================
  * Private Members                                                       *
  * ======================================================================*/

  private:
  //Private members/variables used by parser.

  OSCL_FILE*   m_AMRWBFilePtr;
  void*        m_pUserData;
  uint64       m_nCurrOffset;
  uint64       m_nFileSize;
  uint8        m_ReadBuffer[AMRWB_READ_BUFFER_SIZE];
  uint8*       m_dataBuffer;
  uint64       m_ullCurrPbTime;  /* current playback time */

  //Current parser state
  ParserStatusCode   m_CurrentParserState;

  //Poniter to the Amr header structure
  amrwb_header_amrwbh m_amrwb_header_amrwbh;
  amrwb_audio_info m_amrwb_audio_info;

  simple_seektable* psimple_seektable;
  seek*             pseek;

  //private class methods
  void parse_amrwb_file_header();
  void parse_amrwb_audio_data();
  FileSourceConfigItemEnum m_eFrameModeCfg;
  FileSourceConfigItemEnum m_eHeaderModeCfg;

};
#endif
