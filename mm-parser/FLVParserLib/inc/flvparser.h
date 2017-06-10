#ifndef _FLV_PARSER_H
#define _FLV_PARSER_H
/* =======================================================================
                              FLVParser.h
DESCRIPTION
Flash Video Parser Implementation
Implements Flash metadata parsing functions and supports fetching
a/v samples

Copyright (c) 2012-2014 QUALCOMM Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FLVParserLib/main/latest/inc/flvparser.h#6 $
========================================================================== */
#include "flvparserdatadefn.h"
#include "MMDebugMsg.h"
#include "parserinternaldefs.h"
#include "filebase.h"

/*
*Callback function used by parser for reading the data.
*Parser does not implement this function, it's the responsibility
*of the APP to implement it.
*/

extern uint32 FLVFileCallbakGetData (uint64 ullOffset,
                                     uint32 ulNumBytesRequest,
                                     uint8* pucData,
                                     uint32 ulMaxBufSize,
                                     void*  pUserData );
class FLVParser
{
  /*=======================================================================
  * Public Members                                                        *
  * ======================================================================*/
  public:
                       FLVParser(void* pUData,uint64 fsize,bool bAudio);
                       ~FLVParser();
   PARSER_ERRORTYPE    StartParsing(void);
   PARSER_ERRORTYPE    GetCurrentSample(uint32 trackId,uint8* dataBuffer,
                                        uint32 nMaxBufSize, int32* nBytesRead,
                                        FLVStreamSampleInfo*);

   /*
   * Parameter details:
   * uint32 Track Id
   * uint64 Requested reposition time
   * uint32 Current Playback Time
   * mkav_stream_sample_info* SampleInfo filled in if seek is successful
   * bool Specifies if need to do fwd or rwd
   * bool Specifies if it's ok tp sync to non key frame
   * int  Specifies num,ber of frames to skip in fwd/backward direction
   */
   PARSER_ERRORTYPE     Seek(uint32, uint64, uint64, FLVStreamSampleInfo*,
                             bool, bool canSyncToNonKeyFrame=false,
                             int  nSyncFramesToSkip = 0);

   uint32                GetTrackWholeIDList(uint32*);
   PARSER_ERRORTYPE      GetCodecHeader(uint32 ulTrackId, uint8* pCodecBuf,
                                        uint32 *pulBufSize);
   uint32                GetCodecHeaderSize(uint32);

   uint64                GetClipDurationInMsec();
   uint32                GetVideoWidth(uint32);
   uint32                GetVideoHeight(uint32);
   float                 GetVideoFrameRate(uint32);
   uint32                GetAudioSamplingFrequency(uint32);
   uint8                 GetNumberOfAudioChannels(uint32);
   uint32                GetTotalNumberOfTracks(void){return m_ucNoStreams;};
   FLVMediaCodecType     GetTrackCodecType(uint32);
   FLVTrackType          GetTrackType(uint32);
   uint32                GetTrackBufferSize(uint32);

  private:

  PARSER_ERRORTYPE       ParseFLVHeader(uint64 ullOffset);
  PARSER_ERRORTYPE       ParseFLVTag(uint64, FLVTagInfo* ptag = NULL);
  PARSER_ERRORTYPE       ParseAudioInfo(uint64 ullOffset);
  PARSER_ERRORTYPE       ParseVideoInfo(uint64 ullOffset);
  PARSER_ERRORTYPE       ParseMetaData(uint64  ullOffset);
/*! ======================================================================
@brief    InitData

@detail   Initializes class members to their default values

@param[in] None
@return    none
@note      None
========================================================================== */
  void                   InitData();
  /* ==========================================================================
  @brief  Map header byte with Codec Enum

  @details        This function is used to map the header byte with Codec Enum.

  @param[in]      ucHeaderByte       Header byte

  @return     Codec Type.
  @note       Nothing.
  ========================================================================== */
  FLVMediaCodecType      MapHeaderToAudioCodecType(uint8 ucHeaderByte);
  /* ==========================================================================
  @brief  Map header byte with Codec Enum

  @details        This function is used to map the header byte with Codec Enum.

  @param[in]      ucHeaderByte       Header byte

  @return     Codec Type.
  @note       Nothing.
  ========================================================================== */
  FLVMediaCodecType      MapHeaderToVideoCodecType(uint8 ucHeaderByte);
  /* ==========================================================================
    @brief  Function to parse codec config data for H264 codec

    @details        This function is used to parse codec config data buf.
                    It updates codec config data buffer in FLV Parser class.

    @param[in]      pTag             FLV Tag info structure

    @return     Return status
    @note       It is called only if Codec type is H264/AVC.
  ========================================================================== */
  PARSER_ERRORTYPE       PrepareCodecConfigforH264(FLVTagInfo* pTagInfo);
  /* ==========================================================================
    @brief  Function to parse NAL Unit Params

    @details        This function is used to parse Codec config data.
                    Both SPS/PPS has same structure.
                    NAL size of 2 bytes followed by NAL data.

    @param[in]      pucDataBuf       Buffer pointer
    @param[in]      ulBufSize        Buffer size
    @param[in]      pTag             FLV Tag info structure

    @return     NALUDatatype structure pointer. This will be stored in class
    @note       Nothing.
  ========================================================================== */
  NALUDatatype*          UpdateNALUParams(uint8* pTemp, uint32 &ulIndex,
                                          uint32 ulBufSize, uint8 ucNALUCount);
  /* ==========================================================================
  @brief  Function to update Video codec properties

  @details        This function is used to map the header byte with Codec Enum.

  @param[in]      pucDataBuf       Buffer pointer
  @param[in]      ulBufSize        Buffer size
  @param[in]      pTag             FLV Tag info structure

  @return     Codec Type.
  @note       Nothing.
  ========================================================================== */
  uint32                 UpdateVideoProperties(uint8* pucBuf, uint32 ulBufSize,
                                               FLVTagInfo *pTag);

  /* ==========================================================================
    @brief  Function to update index table

    @details    This function is used to check if the index table is present or
                not. If index table is present, gets the closest entry and
                parses Tag at the offset value.

    @param[in/out]  pTagInfo       Structure which has current tag properties.
    @param[in]      bForward       Flag to indicate seek direction
    @param[in]      ullReposTime   Seek time requested

    @return     PARSER_ErrorNone if closest known sample is found,
                Else returns corresponding error.
    @note       None
  ========================================================================== */
  void                   UpdateIndexTable(uint64 ullFrameTime,
                                          FLVTagInfo *pTagInfo);

  /* ==========================================================================
    @brief  Function to check index table and gets the closest entry

    @details    This function is used to check if the index table is present or
                not. If index table is present, gets the closest entry and parses
                Tag at the offset value.

    @param[in/out]  pTagInfo       Structure which has current tag properties.
    @param[in]      bForward       Flag to indicate seek direction
    @param[in]      ullReposTime   Seek time requested

    @return     PARSER_ErrorNone if closest known sample is found,
                Else returns corresponding error.
    @note       None
  ========================================================================== */
PARSER_ERRORTYPE         GetClosestTagPosn(FLVTagInfo *pTagInfo,
                                           bool &bForward,
                                           uint64 ullReposTime);

  /* ==========================================================================
    @brief  Function to check whether current sample is sync sample or not.

    @details    This function is used to check if the current sample is sync
                sample or not.
                It also checks whether this is the sample required based on
                seek time requested.

    @param[in]      TagInfo        Structure which has current tag properties.
    @param[in]      prevIndexEntry Previous Sync sample properties
    @param[in]      bForward       Flag to indicate seek direction
    @param[in]      ullReposTime   Seek time requested
    @param[in/out]  pSampleInfo    Sample info structure

    @return     true if it is required sync sample, Else false
    @note       None
  ========================================================================== */
bool                     CheckCurrentSample(FLVTagInfo TagInfo,
                                          FLVIndexTable prevIndexEntry,
                                          bool bForward,
                                          uint64 ullReposTime,
                                          FLVStreamSampleInfo *pSampleInfo);

  /* ==========================================================================
    @brief  Function to check whether current sample is sync sample or not.

    @details    This function is used to check if the current sample is sync
                sample or not.

    @param[in]      pDataBuf     Buffer pointer in which media data is read.
    @param[in]      pTagInfo     Structure ptr which has current tag properties
    @param[in]      ulBufSize    Data size that read into buffer

    @return     PARSER_ErrorNone if data processed successfully, Else error
    @note       None
  ========================================================================== */
  PARSER_ERRORTYPE         IsKeyFrame(uint8* pDataBuf, FLVTagInfo *pTagInfo,
                                      uint32 ulBufSize);

  /* ==========================================================================
    @brief  Function to get NALU type and size info

    @details    This function is used to get current NALU properties.

    @param[in]     ulIndex       Buf Index value to indicate amount of data read
    @param[in]     NALUSizeLen   NAL Unit Size field length in bytes
    @param[in]     pDataBuf      Buffer pointer in which media data is read
    @param[in/out] pucNALType    Variable to update NAL type
    @param[in/out] pulNALSize    Variable to store NAL Unit Size in bytes
    @param[in]     ulBufSize     Data size that read into buffer

    @return     true if sufficient data is available, Else false
    @note       None
  ========================================================================== */
bool                     GetNextH264NALUnit(uint32 ulIndex,
                                            uint8 ucNALUSizeLen,
                                            uint8* pDataBuf,
                                            uint8* pucNALType,
                                            uint32* pulNALSize,
                                            uint32 ulBufSize);

  uint8                  m_ucNoStreams;
  uint8                  m_ucNoAudioStreams;
  uint8                  m_ucNoVideoStreams;
  bool                   m_bPlayAudio;
  uint64                 m_ullCurrOffset;
  uint64                 m_ullFileSize;
  uint64                 m_ullClipDuration;
  uint64                 m_ullIndexTableDelta;
  uint32                 m_ulMaxIndexTableEntry;
  void*                  m_pUserData;
  uint8*                 m_pucDataBuffer;
  uint32                 m_ulDataBufSize;
  uint8*                 m_pucSeekDataBuffer;
  FLVHeader*             m_pFLVHdr;
  FLVAudioInfo*          m_pAudioInfo;
  FLVVideoInfo*          m_pVideoInfo;
  FLVMetaDataInfo*       m_pMetadataInfo;
  FLVIndexTable*         m_pIndexTable;
};
#endif//#ifndef _FLV_PARSER_H

