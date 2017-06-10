#ifndef AMR_PARSER_H
#define AMR_PARSER_H

/* ============================================================================
                              amrParser.h
DESCRIPTION

  Copyright (c) 2009-2014 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.

==============================================================================*/

/*=============================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRNBParserLib/main/latest/inc/amrparser.h#16 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $
 *=============================================================================*/
#include <stdio.h>
#include <stdlib.h>

#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "iaudioparser.h"
#include "AEEStdDef.h"

#include "amrheaders.h"
#include "oscl_file_io.h"
#include "amrconstants.h"
#include "filesourcetypes.h"

//=============================================================================
//                        FORWARD DECLERATION
//=============================================================================
class simple_seektable;
class seek;

/*!
 * Callback function used by parser for reading the file data.
 * Parser does not implement this function, it's the responsibility
 * of the APP to implement it.
 */

extern uint32 AMRCallbakGetData (uint64 ullOffset,
                                 uint32 ulNumBytesRequest,
                                 uint8* pucData,
                                 uint32 ulMaxBufSize,
                                 void* pUserData );
/*!
 * amrParser class.
 */
class amrParser : public IAudioParser
{
  //===========================================================================
  //                      PUBLIC MEMBERS
  //===========================================================================
  public:

  //CTOR
  amrParser( void* pUserData,
             uint64 fsize,
             OSCL_FILE *FilePtr);
  //DTOR
  virtual  ~amrParser();

  virtual PARSER_ERRORTYPE  StartParsing(void);
  virtual PARSER_ERRORTYPE  GetAMRHeader(amr_header_amrh* pAmrHdrPtr);
  virtual PARSER_ERRORTYPE  GetAudioInfo(amr_audio_info* pAudioInfo);

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
  virtual PARSER_ERRORTYPE GetCurrentSample(uint8* pucDataBuffer,
                                            uint32 ulMaxBufSize,
                                            uint32 *pulBytesNeeded);

  virtual uint64  GetClipDurationInMsec();

  virtual uint64  Seek(uint64 nReposTime);

  virtual IAudioReturnType parse_frame_header (byte* frame,
                                               uint32* frame_size,
                                               uint32* frame_time);

  PARSER_ERRORTYPE parse_amr_frame_header ( uint8 frame,
                                            amr_format_type format,
                                            amr_header_type &header);

  PARSER_ERRORTYPE parse_amr_nb_fs( uint8 frame,
                                    amr_header_type &header);

  virtual void set_newfile_position(uint64 file_position);
  void init_file_position();
  uint64 GetCurrentPbTime(){return m_ullCurrPbTime;};

  //===========================================================================
  //                PRIVATE MEMBERS
  //===========================================================================*/

private:
  simple_seektable* psimple_seektable;
  OSCL_FILE*        m_AMRFilePtr;
  seek*             pseek;

  void*             m_pUserData;
  uint64            m_nCurrOffset;
  uint64            m_nFileSize;
  uint8             m_ReadBuffer[AMR_READ_BUFFER_SIZE];
  uint8*            m_dataBuffer;
  uint64            m_ullCurrPbTime;  /* current playback time */

  //!Current parser state
  ParserStatusCode  m_CurrentParserState;

  //!Variable to store AMR header structure
  amr_header_amrh   m_amr_header_amrh;
  amr_audio_info    m_amr_audio_info;
  amr_header_type   m_amr_current_frame;
  //private class methods
  void parse_amr_file_header();
  void parse_amr_audio_data();
  FileSourceConfigItemEnum m_eFrameModeCfg;
  FileSourceConfigItemEnum m_eHeaderModeCfg;

};
#endif
