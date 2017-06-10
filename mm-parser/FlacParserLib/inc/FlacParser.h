#ifndef _FLAC_PARSER_H
#define _FLAC_PARSER_H
/* =======================================================================
                              FlacParser.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2009-2014 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FlacParserLib/main/latest/inc/FlacParser.h#10 $
========================================================================== */
#include "FlacParserConstants.h"
#include "FlacParserStatus.h"
#include "FlacParserDataDefn.h"
#include "MMDebugMsg.h"
#include "filebase.h"

#ifdef FEATURE_FILESOURCE_FLAC_PARSER
typedef uint32 (*DataReadCallBack)(uint64 nOffset,
                                   uint32 nNumBytesRequest,
                                   unsigned char* pData,
                                   uint32  nMaxSize,
                                   void*   pClientData );
class FlacParser
{
  /*=======================================================================
  * Public Members                                                        *
  * ======================================================================*/
  public:
                            FlacParser(void* pUData,uint64 fsize,DataReadCallBack );
                            ~FlacParser();
   FlacParserStatus         StartParsing(uint64&);
   FlacParserStatus         StartParsing(uint64&, bool bForceSeek);
   uint32                   GetTrackWholeIDList(uint32*);
   uint8*                   GetCodecHeader(uint32 /*ulTrackID*/)
                            {return m_pCodecHeader;};
   uint32                   GetCodecHeaderSize(uint32 /*ulTrackID*/)
                            {return m_nCodecHeaderSize;};
   uint64                   GetClipDurationInMsec()
                            {return m_nClipDuration;};
   uint32                   GetTotalNumberOfAudioTracks(void)
                            {return m_nAudioStreams;}
   uint32                   GetFlacMaxBufferSize(uint32);

   FlacParserStatus         GetCurrentSample(uint32 trackId,uint8* dataBuffer,
                                             uint32 nMaxBufSize, int32* nBytesRead);
   FlacParserStatus         GetCurrentSampleTimeStamp(uint32,uint64*);

   FlacParserStatus         GetCurrentSample(uint32 trackId,uint8* dataBuffer,
                                              uint32 nMaxBufSize,
                                              int32* nBytesRead,
                                              bool bGetFrameBoundary);
   FlacParserStatus         GetFlacStreamInfo(uint32 id,flac_metadata_streaminfo*);
   /*
   * Parameter details:
   * uint32 Track Id
   * uint64 Requested reposition time
   * uint32 Current Playback Time
   * flac_stream_sample_info* SampleInfo filled in if seek is successful
   * bool Specifies if need to do fwd or rwd
   */

   FlacParserStatus         Seek(uint32, uint64, uint64, flac_stream_sample_info*, bool);
   bool                     IsMetaDataParsingDone();
   /* =========================================================================
     FUNCTION:
     FlacParser::GetClipMetaData

    DESCRIPTION:
    Returns the clip meta data identified via nIndex

    INPUT/OUTPUT PARAMETERS:
        ulMetaIndex     Metadata Index for which data is requested
        pucMetaDataBuf  Buffer to fill metadata
                       (it will be NULL if request is to calculate length)
        pulBufSize      Buffer size pointer

    RETURN VALUE:
      FLACPARSER_SUCCESS if successful, otherwise returns appropriate error.

    SIDE EFFECTS:
      None.
   ==========================================================================*/
   FlacParserStatus         GetClipMetaData(uint32  ulMetaIndex,
                                            uint8*  pucMetaDataBuf,
                                            uint32* pulBufSize);
   /* =========================================================================
    FUNCTION:
     FlacParser::getAlbumArt

    DESCRIPTION:
    Returns the album art if available

    INPUT/OUTPUT PARAMETERS:
        pAlbArt         Buffer to fill metadata
                       (it will be NULL if request is to calculate length)
        pulSize         Buffer size pointer

    RETURN VALUE:
    FLACPARSER_SUCCESS if successful, otherwise returns appropriate error.

    SIDE EFFECTS:
      None.
   ==========================================================================*/
  FlacParserStatus          getAlbumArt(FS_ALBUM_ART_METADATA *pAlbArt,
                                        uint32* pulSize);
  private:

  //Functions/data types internal to FLAC parser
   FlacParserStatus         FindNextFrameOffset(uint8* pBuffer, uint32 nSize, uint32 *nOffset);
   FlacParserStatus         DecodeFrameHeader(uint8* pBuffer, uint32 nSize);
   void                     ReadUTF8_uint64(uint8 *pBuf,
                                             uint64 *nOutVal,
                                             uint8 *nBytes);
   void                     ReadUTF8_uint32(uint8 *pBuf,
                                           uint32 *nOutVal,
                                           uint8 *nBytes);
   uint8                    Calculate_CRC8(uint8 *pData,
                                           uint32 nLen);
  void                      GenerateSeekTable();
  bool                      ParseStreamInfoMetaBlock(uint64&,uint32);
  bool                      ParseSeekTableMetaBlock(uint64&,uint32);
  bool                      ParsePictureMetaBlock(uint64&,uint32);
  bool                      SkipMetaBlock(uint64&,uint32);
  void                      ParseCommentHdr(uint32,uint32);
  uint8*                    m_pCodecHeader;
  uint8*                    m_pDataBuffer;
  void*                     m_pUserData;
  uint64*                   m_pSeekInfoArray;
  flac_metadata_streaminfo* m_pStreamInfoMetaBlock;
  flac_metadata_seektable*  m_pSeekTableMetaBlock;
  flac_metadata_picture*    m_pPictureMetaBlock;
  flac_frame_header*        m_pCurrentFrameHdr;
  flac_meta_data*           m_pMetaData;
  uint64                    m_nCurrOffset;
  uint64                    m_nCurrentTimeStamp;
  uint64                    m_nFileSize;
  uint64                    m_nClipDuration;
  FlacParserStatus          m_eParserStatus;
  uint32                    m_nCodecHeaderSize;
  uint32                    m_nFlacDataBufferSize;
  uint32                    m_nMetaData;
  uint32                    m_nSeekInfoArraySize;
  uint32                    m_nSeekInfoValidEntries;
  uint32                    m_nFirstFrameOffset;
  uint8                     m_nAudioStreams;
  bool                      m_bFlacMetaDataParsed;
  DataReadCallBack          m_pReadCallback;
};
#endif//#ifndef _FLAC_PARSER_H
#endif //#ifdef FEATURE_FILESOURCE_FLAC_PARSER

