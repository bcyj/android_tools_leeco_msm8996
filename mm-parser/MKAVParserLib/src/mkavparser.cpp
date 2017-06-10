/* =======================================================================
                              MKAVParser.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2011-2015 Qualcomm Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MKAVParserLib/main/latest/src/mkavparser.cpp#69 $
$DateTime: 2014/03/03 03:30:30 $
$Change: 5375327 $
========================================================================== */
#include "mkavparserconstants.h"
#include "mkavparser.h"
#include "string.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "MMMalloc.h"
#include "math.h"
#include <stdio.h>
#include "parserdatadef.h"
#include "filebase.h"
#include "filesourcetypes.h"

#ifdef FEATURE_FILESOURCE_MKV_PARSER

#define MAX_META_STR_SIZE    256
#define DEF_AUDIO_BUFF_SIZE  128000
#define AVC1_START_CODE_SIZE 4
const uint8 AVC1_START_CODE[AVC1_START_CODE_SIZE] ={0x00,0x00,0x00,0x01};
//#define MKAV_PARSER_DEBUG 1
/* Integers are stored in EBML  using variable Integer.
   To extract integer values from EBML values, we need to subtract with the max
   value. But in EBML, max value varies depending on number of bytes used to
   store any integer value. Number of bytes used will be indicated by number of
   leading ZEROs in first byte of the number.
   For eg, 0b1Sxxxxxx - 1Byte
           0b01Sxxxxx - 2Bytes
           0b001Sxxxx - 3Bytes.
   In case of Signed Integers, one more additional bit is used to store flag
   which contains whether number is Positive or Negative.
   Thats the reason to ignore most significant bits dependending on number of
   bytes used to store in EBML variable integer format and extra bit for
   signed bit.
*/
const int64 VINT_MAX [] = { 0x3F, 0x1FFF, 0x0FFFFF, 0x07FFFFFF,
                            0x03FFFFFFFF, 0x01FFFFFFFFFF, 0x00FFFFFFFFFFFFFF,
                            0x007FFFFFFFFFFFFF };

inline void MKV_REVERSE_ENDIAN(uint8* ptr, size_t ullDataSize)
{
  //! Number of bytes used for size and type fields are max 8
  //! It can be type casted to uint8 without losing any precision
  uint8 ucEndPos = (uint8)(ullDataSize-1);
  //! This parameter max takes 8 bytes,
  ucEndPos = FILESOURCE_MIN(ucEndPos, 7);
  if(ptr && (ullDataSize >1))
  {
    for(int i =0; (i < (int)ullDataSize) && (i <= (int)ucEndPos); i++)
    {
      uint8 temp = ptr[i];
      *(ptr+i) = *(ptr+ucEndPos);
      *(ptr+ucEndPos) = temp;
      ucEndPos--;
    }
  }
}

void convertEBMLtoINT(uint8* pBuf, size_t ullSize, int64 *pValue,
                      bool *pbIsNegative)
{
  uint8 *temp = (uint8 *)pValue;
  if(pbIsNegative)
  {
    *pbIsNegative = false;
  }

  /* If the input number is negative, do 2's compliment. */
  if(pbIsNegative && pBuf && pValue && (*pBuf & 0x80))
  {
    *pbIsNegative = true;
    for(int nCount = 0; nCount < (int)ullSize; nCount++)
    {
      //1's Compliment byte after  byte.
      temp[nCount] = uint8(~temp[nCount]);
    }
  }
}

uint64 AtomSize(uint8 *pBuf, uint8 *pSizeBytes)
{
  uint64 ullSize = 0;
  uint8 ebmlsizeid = pBuf[0];

  *pSizeBytes = EBML_LENGTH_TBLE[ebmlsizeid][1];

  /* Most Significant bits will indicate number of bytes used to store
     size value. While calculating the size, Parser will mask the most
     significant bits with ZERO.
     If size is 0x04, it will be stored as 0x84 in the file.
     Now Parser will mask MSB value by anding with 0x7F.
  */
  pBuf[0] = pBuf[0] & EBML_LENGTH_TBLE[ebmlsizeid][2];
  memcpy(&ullSize, pBuf, *pSizeBytes);
  //Reverse byte order for Size Bytes
  MKV_REVERSE_ENDIAN((uint8*)&ullSize, *pSizeBytes);
  return ullSize;
}

uint8 AtomIdBytes(uint8 *pBuf)
{
  uint8 ebmlId = pBuf[0];
  uint8 IdBytes = EBML_LENGTH_TBLE[ebmlId][1];

  //Reverse byte order for ID Bytes
  MKV_REVERSE_ENDIAN(pBuf, IdBytes);
  return IdBytes;
}

/*! ======================================================================
@brief    MapParserStatetoStatus

@detail  This function used to Map MKV Parser State to Error Status.

@param[in]      eState      MKV Parser state
@param[in/out]  eRetStatus  MKV Parser Error Enum

@return    None
@note      None
========================================================================== */
void MapParserStatetoStatus(MKAV_PARSER_STATE eState,
                            MKAV_API_STATUS &eRetStatus)
{
  switch(eState)
  {
    case MKAVPARSER_READ_ERROR:
      eRetStatus = MKAV_API_READ_FAIL;
      break;
    case MKAVPARSER_DATA_UNDERRUN:
      eRetStatus = MKAV_API_DATA_UNDERRUN;
      break;
    case MKAVPARSER_EOF:
      eRetStatus = MKAV_API_EOF;
      break;
    case MKAVPARSER_OUT_OF_MEMORY:
      eRetStatus = MKAV_API_OUT_OF_MEMORY;
      break;
    case MKAVPARSER_INSUFFICIENT_BUFFER:
      eRetStatus = MKAV_API_INSUFFICIENT_BUFFER;
      break;
    case MKAVPARSER_PARSE_ERROR  :
    case MKAVPARSER_DEFAULT_ERROR:
      eRetStatus = MKAV_API_FAIL;
      break;
    default:
      eRetStatus = MKAV_API_SUCCESS;
      break;
  }
}

/*! ======================================================================
@brief    CalcAudioFrameDuration

@detail  This function used to calculate audio frame duration.

@param[in]  pTrackEntry      Ptr to Track properties.
@param[in]  eCodecType       Codec Type Enum.
@param[in]  pucFrameBuf     Frame Data buffer (Default NULL).
@param[in]  bIsFrameHdrPrsnt Flag to indicate whether frame header is present
                             or not.
@return    Frame duration value in Nano sec units
@note      If number of samples per frame is not known, then Frame Data is
           required to calculate number of samples in each frame.
========================================================================== */
uint64 CalcAudioFrameDuration(mkav_track_entry_info* pTrackEntry,
                              mkav_media_codec_type  eCodecType,
                              uint8* pucFrameBuf = NULL,
                              bool   bIsFrameHdrPrsnt = false)
{
  uint64 ullFrameDuration = 0;
  //validate input parameters
  if(NULL == pTrackEntry || NULL == pTrackEntry->pAudioInfo)
  {
    return ullFrameDuration;
  }
  mkav_audio_info* pAudioInfo = pTrackEntry->pAudioInfo;
  if(MKAV_AAC_AUDIO == eCodecType)
  {
    ullFrameDuration = (uint64)((AAC_SAMPLES_PER_DATA_BLOCK *
                                 NANOSEC_TIMESCALE_UNIT) /
                                pAudioInfo->SamplingFrequency);
  }
  else if(MKAV_MP3_AUDIO == eCodecType)
  {
    ullFrameDuration = (uint64)((MP3_SAMPLES_TABLE[1][1] *
                                 NANOSEC_TIMESCALE_UNIT) /
                                pAudioInfo->SamplingFrequency);
  }
  else if(MKAV_AC3_AUDIO_CODEC == eCodecType )
  {
    ullFrameDuration = (uint64)((AC3_MAX_NUM_BLOCKS * AC3_SAMPLE_BLOCK_SIZE *
                                 NANOSEC_TIMESCALE_UNIT) /
                                pAudioInfo->SamplingFrequency);
  }
  else if((MKAV_DOLBY_AC3_CODEC == eCodecType ||
           MKAV_EAC3_AUDIO_CODEC == eCodecType) && (pucFrameBuf))
  {
    uint32 ulIndex = 0;
    /* If header is present, then first two bytes contains frame signature. */
    if(true == bIsFrameHdrPrsnt)
    {
      ulIndex = 2;
    }
    //Next two bytes used to store CRC info.
    ulIndex += 2;
    /* In MKV, it is assumed that Data will always be in Big Endian format.
       Parser need this assumption because if frame sync marker is not
       present in some cases, then Parser cannot identify whether i/p data
       is in BE or LE format. */
    uint8 ucFsCod   = uint8((pucFrameBuf[ulIndex] & 0xC0 ) >> 6 );
    uint8 ucNumBlks = uint8((pucFrameBuf[ulIndex] & 0x30 ) >> 4 );
    if(3 == ucFsCod)
    {
      ucNumBlks = AC3_MAX_NUM_BLOCKS;
    }
    ullFrameDuration = (uint64)((ucNumBlks * AC3_SAMPLE_BLOCK_SIZE *
                                 NANOSEC_TIMESCALE_UNIT) /
                                 pAudioInfo->SamplingFrequency);
  }

  return ullFrameDuration;
}

/*! ======================================================================
@brief    MKAVParser constructor

@detail  Instantiates MKAVParser to MKAV file.

@param[in]  pUData    APP's UserData.This will be passed back when this parser invokes the callbacks.
@return    N/A
@note      None
========================================================================== */
MKAVParser::MKAVParser(void* pUData,uint64 fsize,bool bAudio,bool bhttp)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"MKAVParser");
  InitData();
  m_pUserData = pUData;
  m_nFileSize = fsize;
  m_bPlayAudio = bAudio;
  m_bHttpPlay = bhttp;
  //Mark End of Data flag as true in local playback scenario.
  if(!bhttp)
  {
    m_bEndOfData = true;
  }
}
/*! ======================================================================
@brief    InitData

@detail  Initializes class members to their default values

@param[in] None
@return    none
@note      None
========================================================================== */
void MKAVParser::InitData()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"InitData");
  m_pUserData = NULL;
  m_nCurrOffset = 0;
  m_nFileSize = 0;
  m_nSizeReq  = 0;
  m_nAudioStreams = m_nVideoStreams = 0;
  m_nClipDuration = 0;
  m_nstreams      = 0;
  m_eParserState = MKAVPARSER_INIT;
  m_eParserPrvState = MKAVPARSER_INIT;
  m_pDataBuffer = NULL;
  m_bPlayAudio = false;
  m_nMetaData = 0;
  m_pEBMlDocHdr = NULL;
  m_pTrackEntry = NULL;
  m_pSeekHeadInfo = NULL;
  m_nTrackEntry = 0;
  m_nSegmentPosn = 0;
  m_pAllClustersInfo = NULL;
  m_nCurrCluster = 0;
  m_pTempBuffer = NULL;
  m_nTempBuffSize = 0;
  m_pSegmentInfo = NULL;
  m_pSampleInfo = NULL;
  m_pCurrCluster = NULL;
  m_pVFWHeader = NULL;
  m_pAllCuesInfo = NULL;
  m_nCodecHdrToSend = 0;
  m_nTagInfoCount   = 0;
  m_bHttpPlay       = false;
  m_bEndOfData      = false;
  m_bIsWebm         = false;
  m_pSegmentElementInfo   = NULL;
  m_eFrameOutputModeEnum  = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM;
  m_eHeaderOutputModeEnum = FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER;
  memset(&m_nCodecHdrSizes,0,sizeof(uint32)*MKAV_VORBIS_CODEC_HDRS);
  memset(&m_SeekTableInfo, 0, sizeof(seek_table_info));
  m_SeekTableInfo.ulLowerBound = (uint32)-1;
  m_SeekTableInfo.ulUpperBound = 0;
}
/*! ======================================================================
@brief    MKAVParser destructor

@detail  MKAVParser destructor

@param[in] N/A
@return    N/A
@note      None
========================================================================== */
MKAVParser::~MKAVParser()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"~MKAVParser");
  if(m_pDataBuffer)
  {
    MM_Free(m_pDataBuffer);
  }
  if(m_pEBMlDocHdr)
  {
    if(m_pEBMlDocHdr->pDocType)
    {
      MM_Free(m_pEBMlDocHdr->pDocType);
    }
    MM_Free(m_pEBMlDocHdr);
  }
  if(m_pTrackEntry)
  {
    for(uint32 i = 0; i < m_nTrackEntry; i++)
    {
      if(m_pTrackEntry[i].pName)
      {
        MM_Free(m_pTrackEntry[i].pName);
      }
      if(m_pTrackEntry[i].pLanguage)
      {
        MM_Free(m_pTrackEntry[i].pLanguage);
      }
      if(m_pTrackEntry[i].pCodecPvt)
      {
        MM_Free(m_pTrackEntry[i].pCodecPvt);
      }
      if(m_pTrackEntry[i].pCodecName)
      {
        MM_Free(m_pTrackEntry[i].pCodecName);
      }
      if(m_pTrackEntry[i].pVideoInfo)
      {
        if(m_pTrackEntry[i].pVideoInfo->pCodecHdr)
        {
          MM_Free(m_pTrackEntry[i].pVideoInfo->pCodecHdr);
        }
        MM_Free(m_pTrackEntry[i].pVideoInfo);
      }
      if(m_pTrackEntry[i].pAudioInfo)
      {
        MM_Free(m_pTrackEntry[i].pAudioInfo);
      }
    }
    MM_Free(m_pTrackEntry);
  }
  if(m_pTempBuffer)
  {
    MM_Free(m_pTempBuffer);
  }
  if(m_pSeekHeadInfo)
  {
    if(m_pSeekHeadInfo->pSeekHead)
    {
      for(uint32 i = 0; i < m_pSeekHeadInfo->nSeekHead; i++)
      {
        if(m_pSeekHeadInfo->pSeekHead[i].pSeekInfo)
        {
          MM_Free(m_pSeekHeadInfo->pSeekHead[i].pSeekInfo);
        }
      }
      MM_Free(m_pSeekHeadInfo->pSeekHead);
    }
    MM_Free(m_pSeekHeadInfo);
  }
  FreeClustersInfo();
  if(m_pSegmentInfo)
  {
    FreeUpSegmentInfoMemory(m_pSegmentInfo);
  }
  if(m_pSampleInfo)
  {
    MM_Free(m_pSampleInfo);
  }
  if(m_pCurrCluster)
  {
    if(m_pCurrCluster->pBlockGroup)
    {
      MM_Free(m_pCurrCluster->pBlockGroup);
    }
    if(m_pCurrCluster->pSimpleBlocks)
    {
      MM_Free(m_pCurrCluster->pSimpleBlocks);
    }
    MM_Free(m_pCurrCluster);
  }
  if(m_pAllCuesInfo)
  {
    for(uint32 j = 0; (j< m_pAllCuesInfo->nCuePoints) && m_pAllCuesInfo->pCuePointInfo ;j++)
    {
      if(m_pAllCuesInfo->pCuePointInfo[j].nCueTrackPosnInfo && m_pAllCuesInfo->pCuePointInfo[j].pCueTrackPosnInfo)
      {
        for(uint32 k =0; k< m_pAllCuesInfo->pCuePointInfo[j].nCueTrackPosnInfo;k++)
        {
          if(m_pAllCuesInfo->pCuePointInfo[j].pCueTrackPosnInfo[k].pCueRefInfo)
          {
            MM_Free(m_pAllCuesInfo->pCuePointInfo[j].pCueTrackPosnInfo[k].pCueRefInfo);
          }
        }
        if(m_pAllCuesInfo->pCuePointInfo[j].pCueTrackPosnInfo)
        {
          MM_Free(m_pAllCuesInfo->pCuePointInfo[j].pCueTrackPosnInfo);
        }
      }
    }
    if(m_pAllCuesInfo->pCuePointInfo)
    {
      MM_Free(m_pAllCuesInfo->pCuePointInfo);
    }
    MM_Free(m_pAllCuesInfo);
  }
  if(m_pSegmentElementInfo)
  {
    MM_Free(m_pSegmentElementInfo);
  }
  if(m_SeekTableInfo.pSeekTableEntries)
  {
    MM_Free(m_SeekTableInfo.pSeekTableEntries);
  }
  if (m_nTagInfoCount)
  {
    for (uint32 ulIndex = 0; ulIndex < m_nTagInfoCount; ulIndex++)
    {
      tag_info_type *pTagInfo = TagInfoArray[ulIndex];
      if (pTagInfo->pTagName)
      {
        MM_Free(pTagInfo->pTagName);
      }

      if (pTagInfo->pTagLang)
      {
        MM_Free(pTagInfo->pTagLang);
      }
      if (pTagInfo->pTagString)
      {
        MM_Free(pTagInfo->pTagString);
      }
      MM_Free(pTagInfo);
    }
    m_nTagInfoCount = 0;
  }

}
/*! ======================================================================
@brief  Starts Parsing the MKAV file.

@detail  This function starts parsing the mkav file.
        Upon successful parsing, user can retrieve total number of streams and
        stream specific information.

@param[in] N/A
@return    MKAVPARSER_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      StartParsing needs to be called before retrieving any elementary stream specific information.
========================================================================== */
MKAV_API_STATUS MKAVParser::StartParsing(void)
{
  MKAV_API_STATUS retError = MKAV_API_FAIL;
  bool bContinue           = true;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"StartParsing");
  m_pDataBuffer = (uint8*)MM_Malloc(DEF_DATA_BUF_SIZE);
  if(m_pDataBuffer)
  {
    m_nCurrOffset  = 0;
    m_eParserState = MKAVPARSER_READ_DATA;
    uint32 ulIndex = 0;
    uint32 ulBytesToRead                = DEF_EBML_MAX_SIGNATURE_LENGTH_VAL;
    MKAV_PARSER_STATE eNextParserStatus = MKAVPARSER_LOCATE_EBML_DOC_HDR;
    uint32 ulEBMLId        = 0;
    uint8  ucEBMLSizeBytes = 0;
    uint8  ucEBMLIdBytes   = 0;
    uint64 ullAtomSize     = 0;

    while(bContinue)
    {
      switch(m_eParserState)
      {
        case MKAVPARSER_READ_DATA:
        {
          if( !GetDataFromSource(m_nCurrOffset, ulBytesToRead, m_pDataBuffer,
                                 DEF_DATA_BUF_SIZE) )
          {
            MapParserStatetoStatus(m_eParserState, retError);
            bContinue = false;
          }
          else
          {
            ucEBMLIdBytes = AtomIdBytes(m_pDataBuffer);
            ullAtomSize   = AtomSize(m_pDataBuffer + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);
            ulIndex = ucEBMLSizeBytes + ucEBMLIdBytes;
            if (ullAtomSize >= m_nFileSize)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                "ParseByteStream corrupted atom size @ %llu", m_nCurrOffset);
              bContinue      = false;
              m_eParserState = MKAVPARSER_PARSE_ERROR;
              retError       = MKAV_API_FAIL;
            }
            else if(!memcmp(m_pDataBuffer, &EBML_DOC_ID, sizeof(EBML_DOC_ID)) )
            {
              m_eParserState = MKAVPARSER_LOCATE_EBML_DOC_HDR;
            }
            else if(!memcmp(m_pDataBuffer, &EBML_SEGMENT_ID,
                            sizeof(EBML_SEGMENT_ID)))
            {
              m_eParserState = MKAVPARSER_LOCATE_SEGMENT_HDR;
            }
            else
            {
              m_eParserState = MKAVPARSER_PARSE_SEGMENT_HDR;
            }
          }
        }
        break;
        case MKAVPARSER_LOCATE_EBML_DOC_HDR:
        {
          if(memcmp(m_pDataBuffer, &EBML_DOC_ID, sizeof(EBML_DOC_ID)) == 0)
          {
            #ifdef MKAV_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                       "StartParsing Located EBML DOC ID %llu", m_nCurrOffset);
            #endif
            #ifdef MKAV_PARSER_DEBUG
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
              "StartParsing ucEbmlSizeBytes %d size %llu",
              ucEBMLSizeBytes, ullAtomSize);
            #endif
            m_nCurrOffset += ulIndex;
            retError = ParseEBMLDocHeader(m_nCurrOffset, ullAtomSize);
            if(MKAV_API_SUCCESS == retError)
            {
              m_eParserState    = MKAVPARSER_READ_DATA;
              eNextParserStatus = MKAVPARSER_LOCATE_SEGMENT_HDR;
              m_nCurrOffset    += ullAtomSize ;
            }
            else
            {
              m_eParserState = MKAVPARSER_PARSE_ERROR;
              bContinue      = false;
              m_nCurrOffset -= ulIndex;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                          "ParseEBMLDocHeader failed..");
            }
          }
          else
          {
            m_eParserState = MKAVPARSER_PARSE_ERROR;
          }
        }//case MKAVPARSER_LOCATE_EBML_DOC_HDR:
        break;
        case MKAVPARSER_LOCATE_SEGMENT_HDR:
        {
          if(!memcmp(m_pDataBuffer, &EBML_SEGMENT_ID, sizeof(EBML_SEGMENT_ID)))
          {
            #ifdef MKAV_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "StartParsing Located Segment Hdr %llu", m_nCurrOffset);
            #endif
            #ifdef MKAV_PARSER_DEBUG
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "StartParsing ebmlsizebytes %d size %llu",
                         ucEBMLSizeBytes , ullAtomSize);
            #endif
            m_nCurrOffset += ulIndex;
            uint64 ullEndOffset = m_nCurrOffset + ullAtomSize;
            retError = ParseSegmentElement(m_nCurrOffset, ullEndOffset);
            if((MKAV_API_SUCCESS == retError) ||
               (MKAV_API_EOF == retError))
            {
              m_eParserState = MKAVPARSER_READY;
            }
            else
            {
              m_eParserState = MKAVPARSER_PARSE_ERROR;
            }
            m_nCurrOffset += ullAtomSize;
          }
          else
          {
            m_eParserState = MKAVPARSER_PARSE_ERROR;
          }
        }//case MKAVPARSER_LOCATE_EBML_DOC_HDR:
        break;
        case MKAVPARSER_READY:
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "StartParsing MKAVPARSER_READY..");
          retError  = MKAV_API_SUCCESS;
          bContinue = false;
        }
        break;
        default:
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                      "Could not move into Known state,corrupt file....");
          bContinue = false;
        }
        break;
      }//switch(m_eParserState)
    }//while(bContinue)
  }//if(m_pDataBuffer)
  else
  {
    retError = MKAV_API_OUT_OF_MEMORY;
  }
  if(MKAV_API_SUCCESS == retError)
  {
    retError = UpdateCodecDetails();
  }
  return retError;
}
/*! ======================================================================
@brief  Starts Parsing byte stream

@detail  This function starts parsing the byte stream.
        Upon successful parsing, user can retrieve total number of streams and
        stream specific information.This routine can handle data under-run.

@param[in] N/A
@return    MKAVPARSER_SUCCESS is parsing is successful
           otherwise returns appropriate error.
@note      ParseByteStream needs to be called before retrieving any
           elementary stream specific information.
========================================================================== */
MKAV_API_STATUS MKAVParser::ParseByteStream(void)
{
  MKAV_API_STATUS retError = MKAV_API_FAIL;
  bool bContinue           = true;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseByteStream @offset %llu",
               m_nCurrOffset);
  if(!m_pDataBuffer)
  {
    m_pDataBuffer = (uint8*)MM_Malloc(DEF_DATA_BUF_SIZE);
  }
  if(m_pDataBuffer)
  {
    uint64 ullOffset       = 0;
    uint64 ullAtomSize     = 0;
    uint32 ulBytesToRead   = DEF_EBML_MAX_SIGNATURE_LENGTH_VAL;
    uint8  ucEBMLSizeBytes = 0;
    uint8 ucEBMLIdBytes    = 0;
    m_eParserState         = m_eParserPrvState;
    m_eParserState         = MKAVPARSER_READ_DATA;
    while(bContinue)
    {
      switch(m_eParserState)
      {
        case MKAVPARSER_READ_DATA:
        {
          #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "ParseByteStream MKAVPARSER_INIT");
          #endif
          if( !GetDataFromSource(m_nCurrOffset, ulBytesToRead, m_pDataBuffer,
                                 DEF_DATA_BUF_SIZE) )
          {
            MapParserStatetoStatus(m_eParserState, retError);
            bContinue = false;
          }
          else
          {
            ucEBMLIdBytes = AtomIdBytes(m_pDataBuffer);
            ullAtomSize   = AtomSize(m_pDataBuffer + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);
            if (ullAtomSize >= m_nFileSize)
            {
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
                  "MKVPARSER atom size %llu should be less than fileSize %llu",
                  ullAtomSize,m_nFileSize);
              bContinue      = false;
              m_eParserState = MKAVPARSER_PARSE_ERROR;
              retError       = MKAV_API_FAIL;
            }
            else if(!memcmp(m_pDataBuffer, &EBML_DOC_ID, sizeof(EBML_DOC_ID)) )
            {
              m_eParserState = MKAVPARSER_PARSE_EBML_DOC_HDR;
            }
            else if(!memcmp(m_pDataBuffer, &EBML_SEGMENT_ID,
                            sizeof(EBML_SEGMENT_ID)))
            {
              m_eParserState = MKAVPARSER_LOCATE_SEGMENT_HDR;
            }
            else
            {
              m_eParserState = MKAVPARSER_PARSE_SEGMENT_HDR;
            }
          }
          /* Check whether first atom is EBML DOC header or not.
             If not, error out. */
          if ((!m_nCurrOffset) &&
              (MKAVPARSER_PARSE_EBML_DOC_HDR != m_eParserState))
          {
            m_eParserState = MKAVPARSER_PARSE_ERROR;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                      "ParseByteStream failed can't locate EBML DOC header..");
            bContinue = false;
          }
        }//case MKAVPARSER_READ_DATA:
        break;
        case MKAVPARSER_PARSE_EBML_DOC_HDR:
        {
          #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "ParseByteStream MKAVPARSER_PARSE_EBML_DOC_HDR");
          #endif
          ullOffset = m_nCurrOffset + ucEBMLIdBytes + ucEBMLSizeBytes;
          retError = ParseEBMLDocHeader(ullOffset, (uint32)m_nSizeReq);
          //Data is available, read and parse EBML DOC header
          if(MKAV_API_SUCCESS == retError)
          {
            m_eParserState = MKAVPARSER_READ_DATA;
            m_nCurrOffset += ullAtomSize + ucEBMLIdBytes + ucEBMLSizeBytes;
          }
          else
          {
            bContinue = false;
            if (MKAV_API_DATA_UNDERRUN != retError)
            {
              m_eParserState = MKAVPARSER_PARSE_ERROR;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                          "ParseByteStream ParseEBMLDocHeader failed..");
            }
          }
        }//case MKAVPARSER_PARSE_EBML_DOC_HDR:
        break;
        case MKAVPARSER_LOCATE_SEGMENT_HDR:
        {
          m_nCurrOffset += ucEBMLIdBytes + ucEBMLSizeBytes;
          if(!m_pSegmentElementInfo)
          {
            m_pSegmentElementInfo = (segment_element_info*)
                                   MM_Malloc(sizeof(segment_element_info));
            if(m_pSegmentElementInfo)
            {
              memset(m_pSegmentElementInfo,0,sizeof(segment_element_info));
              m_pSegmentElementInfo->nDataOffset = ullOffset;
              m_pSegmentElementInfo->nDataSize   = ullAtomSize;
              m_eParserState = MKAVPARSER_PARSE_SEGMENT_HDR;
            }
            else
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "ParseByteStream Malloc failed for m_pSegmentElementInfo");
              m_eParserState = MKAVPARSER_OUT_OF_MEMORY;
              bContinue      = false;
            }
          }
          else
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                      "ParseByteStream Found > 1 segment element, offset %llu",
                      m_nCurrOffset);
            bContinue = false;
          }
        }//case MKAVPARSER_LOCATE_SEGMENT_HDR:
        break;
        case MKAVPARSER_PARSE_SEGMENT_HDR:
        case MKAVPARSER_PARSE_SEGMENT_INFO_HDR:
        case MKAVPARSER_PARSE_SEEK_HEAD_HDR:
        case MKAVPARSER_PARSE_CLUSTER_HDR:
        case MKAVPARSER_PARSE_TRACKS_HDR:
        case MKAVPARSER_PARSE_CUES_HDR:
        {
          #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                       "ParseByteStream m_eParserState %d",m_eParserState);
          #endif
          uint64 ullEndOffset = m_pSegmentElementInfo->nDataOffset+
                                m_pSegmentElementInfo->nDataSize;
          retError = ParseSegmentElement(m_nCurrOffset, ullEndOffset, true);
          #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                    "ParseSegmentElement returned status %d m_eParserState %d",
                    retError, m_eParserState);
          #endif
          if(MKAV_API_SUCCESS == retError)
          {
            m_eParserState = MKAVPARSER_READY;
          }
          else
          {
            bContinue = false;
          }
        }
        break;
        case MKAVPARSER_READY:
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "ParseByteStream MKAVPARSER_READY..");
          retError  = MKAV_API_SUCCESS;
          bContinue = false;
        }
        break;
        default:
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                      "Could not move into Known state,corrupt file....");
          bContinue = false;
        }
        break;
      }//switch(m_eParserState)
    }//while(bContinue)
  }//if(m_pDataBuffer)
  if(MKAV_API_SUCCESS == retError)
  {
    retError = UpdateCodecDetails();
  }
  return retError;
}
/* =============================================================================
FUNCTION:
 MKAVParser::ParseEBMLDocHeader

DESCRIPTION:
Parse and stores EBML DOC header

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
MKAVPARSER_SUCCESS if successful in parsing the header,
otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS MKAVParser::ParseEBMLDocHeader(uint64 ullOffset,
                                               uint64 ullEBMLSize)
{
  MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_MEDIUM,"ParseEBMLDocHeader %llu",ullOffset);
  MKAV_API_STATUS status = MKAV_API_FAIL;
  uint32 ulBytesToRead   = DEF_DATA_BUF_SIZE;
  uint32 ulIndex         = 0;
  uint16 ulMatchedId     = 0;
  uint8* pMemory         = NULL;
  uint64 ullEndOffset    = ullOffset + ullEBMLSize;
  uint32 ulDataRead      = 0;
  bool bok               = true;

  m_pEBMlDocHdr = (ebml_doc_hdr*)MM_Malloc(sizeof(ebml_doc_hdr));

  if(m_bHttpPlay && (!EnsureDataCanBeRead(ullOffset, ullEBMLSize)))
  {
    return MKAV_API_DATA_UNDERRUN;
  }
  if(m_pEBMlDocHdr)
  {
    memset(m_pEBMlDocHdr, 0, sizeof(ebml_doc_hdr));

    while(ullOffset < ullEndOffset && bok)
    {
      if(!ulDataRead ||
         ((ulDataRead - ulIndex) < DEF_EBML_MX_ID_LENGTH_VAL*2 &&
          (ullEndOffset - ullOffset) > (ulDataRead - ulIndex)))
      {
        ulBytesToRead = (uint32)(ullEndOffset - ullOffset);
        ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                       m_pDataBuffer, DEF_DATA_BUF_SIZE);
        if( !ulDataRead )
        {
          status = MKAV_API_FAIL;
          bok = false;
          break;
        }
        ulIndex = 0;
      }

      uint8 ucEBMLIDBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
      uint8 ucEBMLSizeBytes = 0;
      uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIDBytes,
                                       &ucEBMLSizeBytes);

      if(!memcmp(m_pDataBuffer + ulIndex, &EBML_VER_ID, sizeof(EBML_VER_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_VER_ID");
        #endif
        ulMatchedId = EBML_VER_ID;
        pMemory = &(m_pEBMlDocHdr->nVersion);
      }
      else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_READ_VER_ID,
              sizeof(EBML_READ_VER_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_READ_VER_ID");
        #endif
        ulMatchedId = EBML_READ_VER_ID;
        pMemory = &(m_pEBMlDocHdr->nReadVersion);
      }
      else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_MX_ID_LENGTH_ID,
                      sizeof(EBML_MX_ID_LENGTH_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_MX_ID_LENGTH_ID");
        #endif
        ulMatchedId = EBML_MX_ID_LENGTH_ID;
        pMemory = &(m_pEBMlDocHdr->nMxIdLength);
      }
      else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_MX_SZ_LENGTH_ID,
                      sizeof(EBML_MX_SZ_LENGTH_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_MX_SZ_LENGTH_ID");
        #endif
        ulMatchedId = EBML_MX_SZ_LENGTH_ID;
        pMemory = &(m_pEBMlDocHdr->nMxSzLength);
      }
      else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_DOCTYPE_ID,
                      sizeof(EBML_DOCTYPE_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_DOCTYPE_ID");
        #endif
        ulMatchedId = EBML_DOCTYPE_ID;
        pMemory = &(m_pEBMlDocHdr->nDocTypeLength);
      }
      else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_DOCTYPE_VER_ID,
                      sizeof(EBML_DOCTYPE_VER_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_DOCTYPE_VER_ID");
        #endif
        ulMatchedId = EBML_DOCTYPE_VER_ID;
        pMemory = &(m_pEBMlDocHdr->nDocTypeVersion);
      }
      else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_DOCTYPE_READ_VER_ID,
                      sizeof(EBML_DOCTYPE_READ_VER_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_DOCTYPE_READ_VER_ID");
        #endif
        ulMatchedId = EBML_DOCTYPE_READ_VER_ID;
        pMemory = &(m_pEBMlDocHdr->nDocTypeRead);
      }
      else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_VOID_ID,
                      sizeof(EBML_VOID_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"VOID_ID %llu",ullOffset);
        #endif
        ulMatchedId = EBML_VOID_ID;
      }
      else if(!memcmp(m_pDataBuffer+ ulIndex,&EBML_CRC_ID,sizeof(EBML_CRC_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_MEDIUM,"EBML_CRC_ID %llu",ullOffset);
        #endif
        ulMatchedId = EBML_CRC_ID;
      }

      // update index to end of signature
      ulIndex += ucEBMLIDBytes + ucEBMLSizeBytes;
      if(ulMatchedId != 0)
      {
        if( pMemory)
        {
          if(ulMatchedId == EBML_DOCTYPE_ID)
          {
            m_pEBMlDocHdr->pDocType = (uint8*)MM_Malloc((uint32)ullSize + 1);
            if(m_pEBMlDocHdr->pDocType)
            {
              memcpy(m_pEBMlDocHdr->pDocType, m_pDataBuffer + ulIndex,
                     (uint32)ullSize);
              m_pEBMlDocHdr->pDocType[ullSize] = '\0';
              m_pEBMlDocHdr->nDocTypeLength = (uint8)ullSize;
              if( 0 == memcmp(m_pEBMlDocHdr->pDocType , "webm",
                              m_pEBMlDocHdr->nDocTypeLength ))
              {
                m_bIsWebm = true;
              }
            }
          }
          else
          {
           memcpy(pMemory, m_pDataBuffer + ulIndex, sizeof(uint16));
          }
        }//else if( pMemory)
        ulMatchedId  = 0;
        pMemory      = NULL;
        ullOffset   += ucEBMLIDBytes + ucEBMLSizeBytes + ullSize;
        ulIndex     += (uint32)ullSize;
      }//if(ulMatchedId != 0)
      else
      {
        ullOffset = ullEndOffset;
        bok = false;
      }
    }//while(noffset < nEndOffset)
  }//if(m_pEBMlDocHdr)
  if(bok)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* =============================================================================
FUNCTION:
 MKAVParser::ResetCurrentClusterInfo

DESCRIPTION:
Reset the current cluster information

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
None

SIDE EFFECTS:
  None.
=============================================================================*/
void MKAVParser::ResetCurrentClusterInfo()
{
  if(m_pCurrCluster && m_pCurrCluster->pBlockGroup)
  {
    if(m_pCurrCluster->pBlockGroup->nBlockinfo)
    {
      for(uint32 nCount = 0; nCount < m_pCurrCluster->pBlockGroup->nBlockinfo;
          nCount++)
      {
        MM_Free(m_pCurrCluster->pBlockGroup->pBlockInfo[nCount].pFramesSize);
      }
      MM_Free(m_pCurrCluster->pBlockGroup->pBlockInfo);
    }
    MM_Free(m_pCurrCluster->pBlockGroup);
    m_pCurrCluster->pBlockGroup = NULL;
    m_pCurrCluster->nBlockGroup = 0;
  }
  if(m_pCurrCluster && m_pCurrCluster->pSimpleBlocks)
  {
    if(m_pCurrCluster->pSimpleBlocks->pFramesSize)
    {
      MM_Free(m_pCurrCluster->pSimpleBlocks->pFramesSize);
    }
    MM_Free(m_pCurrCluster->pSimpleBlocks);
    m_pCurrCluster->pSimpleBlocks = NULL;
    m_pCurrCluster->nSimpleBlocks = 0;
  }
}
/* =============================================================================
FUNCTION:
 MKAVParser::FreeUpSegmentInfoMemory

DESCRIPTION:
Free up the memory used by segment info

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
None

SIDE EFFECTS:
  None.
=============================================================================*/
void  MKAVParser::FreeUpSegmentInfoMemory(segment_info* pseginfo)
{
  if(pseginfo)
  {
    if(pseginfo->pMuxingApp)
    {
      MM_Free(pseginfo->pMuxingApp);
    }
    if(pseginfo->pNextFileName)
    {
      MM_Free(pseginfo->pNextFileName);
    }
    if(pseginfo->pPrvFileName)
    {
      MM_Free(pseginfo->pPrvFileName);
    }
    if(pseginfo->pTitle)
    {
      MM_Free(pseginfo->pTitle);
    }
    if(pseginfo->pSegFileName)
    {
      MM_Free(pseginfo->pSegFileName);
    }
    if(pseginfo->pWritingApp)
    {
      MM_Free(pseginfo->pWritingApp);
    }
    MM_Free(pseginfo);
  }
}

/* =============================================================================
FUNCTION:
 MKAVParser::ParseSegmentInfo

DESCRIPTION:
Parse and stores segment information

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
MKAVPARSER_SUCCESS if successful in parsing the header,
otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS  MKAVParser::ParseSegmentInfo(uint64 ullOffset,
                                              uint64 ullElementSize)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"Entered ParseSegmentInfo %llu",
               ullOffset);
  #endif
  uint32 ulBytesToRead = 0;
  uint32 ulIndex       = 0;
  uint64 ullEndOffset  = ullOffset + ullElementSize;
  uint32 ulDataRead    = 0;
  bool noerror         = true;

  if(m_pSegmentInfo && m_pSegmentInfo->nDuration)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "ParseSegmentInfo m_pSegmentInfo->duration available!!");
    return MKAV_API_SUCCESS;
  }
  if(m_bHttpPlay && !EnsureDataCanBeRead(ullOffset, ullElementSize))
  {
    return MKAV_API_DATA_UNDERRUN;
  }

  if(m_pSegmentInfo)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "ParseSegmentInfo m_pSegmentInfo != NULL");
    FreeUpSegmentInfoMemory(m_pSegmentInfo);
    m_pSegmentInfo = NULL;
  }
  if(!m_pSegmentInfo)
  {
    m_pSegmentInfo = (segment_info*)MM_Malloc(sizeof(segment_info));
  }
  if(m_pSegmentInfo)
  {
    memset(m_pSegmentInfo, 0, sizeof(segment_info));

    //default value for the timecode scale
    m_pSegmentInfo->nTimeCodeScale = MKAV_MILLISECONDS_TIMECODE;
  }
  else
  {
    status  = MKAV_API_OUT_OF_MEMORY;
    noerror = false;
  }

  while( (ullOffset < ullEndOffset) && (noerror) )
  {
    bool bok             = true;
    bool bNeedAllocation = false;
    uint8* pMemory       = NULL;
    size_t ulMaxCopySize = 0;
    uint32 ulMatchedId   = 0;

    if(!ulDataRead ||
      (ulDataRead < uint64(ullEndOffset - ullOffset) &&
      (ulDataRead - ulIndex) < DEF_EBML_MX_SZ_LENGTH_VAL))
    {
      ullOffset += ulIndex;
      ulIndex = 0;
      ulBytesToRead = uint32(ullEndOffset - ullOffset);
      ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                     m_pDataBuffer, DEF_DATA_BUF_SIZE);
      if((ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL) &&
         (ulDataRead < ulBytesToRead) )
      {
        status =  MKAV_API_FAIL;
        noerror = false;
        break;
      }
    }
    else if(ulDataRead == ulIndex)
    {
      status = MKAV_API_SUCCESS;
      break;
    }
    uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
    uint8 ucEBMLSizeBytes = 0;
    uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);

    // Compare ID with standard IDs that occur as part of SegmentInfo chunk
    if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TIMECODESCLE_ID,
               EBML_ELEMENT_ID_SIZE_THREE))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_TIMECODESCLE_ID %llu",
                   ullOffset);
#endif
      ulMatchedId   = EBML_TIMECODESCLE_ID;
      pMemory       = (uint8*)&m_pSegmentInfo->nTimeCodeScale;
      m_pSegmentInfo->nTimeCodeScale = 0;
      ulMaxCopySize = sizeof(m_pSegmentInfo->nTimeCodeScale);
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_PRVFILENAME_ID,
                    EBML_ELEMENT_ID_SIZE_THREE))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_PRVFILENAME_ID %llu",
                   ullOffset);
#endif
      ulMatchedId     = EBML_PRVFILENAME_ID;
      bNeedAllocation = true;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_NEXTUID_ID,
                    EBML_ELEMENT_ID_SIZE_THREE))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_NEXTUID_ID %llu",
                   ullOffset);
#endif
      ulMatchedId   = EBML_NEXTUID_ID;
      ulMaxCopySize = sizeof(m_pSegmentInfo->nextUID);
      pMemory       = (uint8*)m_pSegmentInfo->nextUID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_NXTFILENAME_ID,
                    EBML_ELEMENT_ID_SIZE_THREE))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_NXTFILENAME_ID %llu",
                   ullOffset);
#endif
      ulMatchedId     = EBML_NXTFILENAME_ID;
      bNeedAllocation = true;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_SEGMENTUID_ID,
                    sizeof(EBML_SEGMENTUID_ID)))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_SEGMENTUID_ID %llu",
                   ullOffset);
#endif
      ulMatchedId   = EBML_SEGMENTUID_ID;
      ulMaxCopySize = sizeof(m_pSegmentInfo->SegUID);
      pMemory       = (uint8*)m_pSegmentInfo->SegUID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_SEGFILENAME_ID,
                    sizeof(EBML_SEGFILENAME_ID)))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_SEGFILENAME_ID %llu",
                   ullOffset);
#endif
      ulMatchedId     = EBML_SEGFILENAME_ID;
      bNeedAllocation = true;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TITLE_ID,
                    sizeof(EBML_TITLE_ID)))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_TITLE_ID %llu",
                   ullOffset);
#endif
      ulMatchedId     = EBML_TITLE_ID;
      bNeedAllocation = true;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_MUX_APP_ID,
                    sizeof(EBML_MUX_APP_ID)))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_MUX_APP_ID %llu",
                   ullOffset);
#endif
      ulMatchedId     = EBML_MUX_APP_ID;
      bNeedAllocation = true;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_WRITER_APP_ID,
                    sizeof(EBML_WRITER_APP_ID)))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_WRITER_APP_ID %llu",
                   ullOffset);
#endif
      ulMatchedId     = EBML_WRITER_APP_ID;
      bNeedAllocation = true;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_DATE_UTC_ID,
                    sizeof(EBML_DATE_UTC_ID)))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_DATE_UTC_ID %llu",
                   ullOffset);
#endif
      ulMatchedId   = EBML_DATE_UTC_ID;
      pMemory       = (uint8*)&m_pSegmentInfo->nDate;
      ulMaxCopySize = sizeof(m_pSegmentInfo->nDate);
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_PRVUID_ID,
                    sizeof(EBML_PRVUID_ID)))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_MEDIUM,"EBML_PRVUID_ID %llu",ullOffset);
#endif
      ulMatchedId   = EBML_PRVUID_ID;
      pMemory       = (uint8*)m_pSegmentInfo->PrvUID;
      ulMaxCopySize = sizeof(m_pSegmentInfo->PrvUID);
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_DURATION_ID,
                    sizeof(EBML_DURATION_ID)))
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_DURATION_ID %llu",
                   ullOffset);
#endif
      ulMatchedId   = EBML_DURATION_ID;
      pMemory       = (uint8*)&m_pSegmentInfo->nDuration;
      ulMaxCopySize = sizeof(m_pSegmentInfo->nDuration);
    }
    else
    {
      bok = false;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseSegmentInfo encountered unsupported ID @offset %llu", ullOffset);
      #endif
    }
    //Update index to end of Atom signature
    ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
    if(true == bok)
    {
      if(pMemory)
      {
        /* Duration can be either stored in double or float format.
           If field size is equivalent to float, then store duration
           value in local parameter and typecast as double value. */
        if(EBML_DURATION_ID == ulMatchedId)
        {
          if(sizeof(double) == (uint8)ullSize)
          {
            double Duration = 0;
            memcpy(&Duration, m_pDataBuffer + ulIndex, (uint32)ullSize);
            MKV_REVERSE_ENDIAN( (uint8*)&Duration, (uint8)ullSize);
            m_pSegmentInfo->nDuration = (uint64)Duration;
          }
          else
          {
            float Duration = 0;
            memcpy(&Duration, m_pDataBuffer + ulIndex, (uint32)ullSize);
            MKV_REVERSE_ENDIAN( (uint8*)&Duration, (uint8)ullSize);
            m_pSegmentInfo->nDuration = (uint64)Duration;
          }
        }
        else
        {
          memcpy(pMemory, m_pDataBuffer + ulIndex,
                 FILESOURCE_MIN((uint32)ullSize, ulMaxCopySize));
          MKV_REVERSE_ENDIAN( pMemory,
                             (uint8)FILESOURCE_MIN(ullSize, ulMaxCopySize));
        }
      }
      else if(bNeedAllocation && ullSize)
      {
        uint32 nAllocnSize = (uint32)(ullSize + 1);
        uint8* ptemp = (uint8*)MM_Malloc(nAllocnSize);
        if(ptemp)
        {
          memset(ptemp, 0, nAllocnSize);
          ptemp[ullSize] = '\0';
          memcpy(ptemp, m_pDataBuffer + ulIndex, (size_t)ullSize);
          switch(ulMatchedId)
          {
          case EBML_PRVFILENAME_ID:
            m_pSegmentInfo->pPrvFileName = ptemp;
            break;
          case EBML_NXTFILENAME_ID:
            m_pSegmentInfo->pNextFileName = ptemp;
            break;
          case EBML_SEGFILENAME_ID:
            m_pSegmentInfo->pSegFileName = ptemp;
            break;
          case EBML_MUX_APP_ID:
            m_pSegmentInfo->pMuxingApp = ptemp;
            break;
          case EBML_WRITER_APP_ID:
            m_pSegmentInfo->pWritingApp = ptemp;
            break;
          default:
            MM_Free(ptemp);
            break;
          }//switch(nMatchedId)
        }//if(ptemp)
      }//else if(bNeedAllocation && nBytesToRead)
    }
    //Update index to end of the child atom
    ulIndex += (uint32)ullSize;
  }//while((noffset < nEndOffset)&&(noerror)

  if(m_pSegmentInfo && m_pSegmentInfo->nDuration)
  {
    if(m_pSegmentInfo->nTimeCodeScale == MKAV_MILLISECONDS_TIMECODE)
    {
      //When time code scale is 1000000, all the time codes are in milliseconds
      m_nClipDuration = (uint64)m_pSegmentInfo->nDuration;
    }
    else
    {
      //convert from nanoseconds to milliseconds.
      m_nClipDuration =
        ((uint64)m_pSegmentInfo->nDuration * m_pSegmentInfo->nTimeCodeScale) /
         MKAV_MILLISECONDS_TIMECODE;
    }
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"file duration value is %llu",
                 m_nClipDuration);
  }//if(m_pSegmentInfo)
  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* =============================================================================
FUNCTION:
 MKAVParser::ParserSegmentElement

DESCRIPTION:
Parse and stores EBML segment header

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
MKAVPARSER_SUCCESS if successful in parsing the header, otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS  MKAVParser::ParseSegmentElement(uint64 ullOffset,
                                                 uint64 ullEndOffset,
                                                 bool   bupdateoffset)
{
  MKAV_API_STATUS eStatus = MKAV_API_FAIL;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"Entered ParseSegmentElement %llu",
               ullOffset);
  if(!m_nSegmentPosn)
  {
    m_nSegmentPosn = ullOffset;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"m_nSegmentPosn %llu",
               m_nSegmentPosn);
  uint32 ulBytesToRead = DEF_EBML_MAX_SIGNATURE_LENGTH_VAL;
  uint32 ulIndex       = 0;
  bool noerror         = true;

  while( (ullOffset < ullEndOffset)&&(noerror) )
  {
    ulIndex = 0;
    /* In case of streaming, Parser will not parse entire file.
       It parses mandatory fields, Segment Info and Tracks and one cluster.
       If CUE info is available, then Parser will directly go to start of
       CUES from current position. Thats why local offset param is updated
       with class variable. */
    if(m_bHttpPlay)
    {
      if(IsMetaDataParsingDone())
      {
        break;
      }
      ullOffset = m_nCurrOffset;
    }
    if( !GetDataFromSource(ullOffset, ulBytesToRead, m_pDataBuffer,
                           DEF_DATA_BUF_SIZE) )
    {
      MapParserStatetoStatus(m_eParserState, eStatus);
      noerror = false;
    }
    else
    {
      bool bMaster          = true;
      bool bok              = true;
      uint8 ucebmlId        = m_pDataBuffer[ulIndex];
      uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
      uint8 ucEBMLSizeBytes = 0;
      uint32 ulMatchedId    = 0;
      uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIdBytes,
                                       &ucEBMLSizeBytes);

      if(!memcmp(m_pDataBuffer, &EBML_SEGMENT_INFO_ID,
                 sizeof(EBML_SEGMENT_INFO_ID)) )
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "EBML_SEGMENT_INFO_ID %llu",
                       ullOffset);
        #endif
        ulMatchedId    = EBML_SEGMENT_INFO_ID;
        m_eParserState = MKAVPARSER_PARSE_SEGMENT_INFO_HDR;
      }
      else if(!memcmp(m_pDataBuffer, &EBML_SEEK_HEAD_ID,
                      sizeof(EBML_SEEK_HEAD_ID)) )
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "EBML_SEEK_HEAD_ID %llu",
                       ullOffset);
        #endif
        ulMatchedId    = EBML_SEEK_HEAD_ID;
        m_eParserState = MKAVPARSER_PARSE_SEEK_HEAD_HDR;
      }
      else if(!memcmp(m_pDataBuffer, &EBML_CLUSTER_ID,
                      sizeof(EBML_CLUSTER_ID)) )
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "EBML_CLUSTER_ID %llu",
                       ullOffset);
        #endif
        ulMatchedId    = EBML_CLUSTER_ID;
        m_eParserState = MKAVPARSER_PARSE_CLUSTER_HDR;
      }
      else if(!memcmp(m_pDataBuffer, &EBML_TRACKS_ID,
                      sizeof(EBML_TRACKS_ID)) )
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "EBML_TRACKS_ID %llu",
                       ullOffset);
        #endif
        ulMatchedId    = EBML_TRACKS_ID;
        m_eParserState = MKAVPARSER_PARSE_TRACKS_HDR;
      }
      else if(!memcmp(m_pDataBuffer, &EBML_CUES_ID, sizeof(EBML_CUES_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "EBML_CUES_ID %llu",
                       ullOffset);
        #endif
        ulMatchedId    = EBML_CUES_ID;
        m_eParserState = MKAVPARSER_PARSE_CUES_HDR;
      }
      else if(!memcmp(m_pDataBuffer, &EBML_ATTACHMENTS_ID,
                     sizeof(EBML_ATTACHMENTS_ID)) )
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "EBML_ATTACHMENTS_ID %llu",
                       ullOffset);
        #endif
        ulMatchedId    = EBML_ATTACHMENTS_ID;
        m_eParserState = MKAVPARSER_SKIP_EBML_ID;
      }
      else if(!memcmp(m_pDataBuffer, &EBML_CHAPTERS_ID,
                      sizeof(EBML_CHAPTERS_ID)) )
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "EBML_CHAPTERS_ID %llu",
                       ullOffset);
        #endif
        ulMatchedId    = EBML_CHAPTERS_ID;
        m_eParserState = MKAVPARSER_SKIP_EBML_ID;
      }
      else if(!memcmp(m_pDataBuffer, &EBML_TAGS_ID, sizeof(EBML_TAGS_ID)) )
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_HIGH,"EBML_TAGS_ID %llu",ullOffset);
        #endif
        ulMatchedId    = EBML_TAGS_ID;
        m_eParserState = MKAVPARSER_SKIP_EBML_ID;
      }
      else if(EBML_VOID_ID == ucebmlId)
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "VOID_ID %llu", ullOffset);
        #endif
        bMaster        = false;
        ulMatchedId    = EBML_VOID_ID;
        m_eParserState = MKAVPARSER_SKIP_EBML_ID;
      }
      else if(EBML_CRC_ID == ucebmlId)
      {
        #ifdef MKAV_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CRC_ID %llu",ullOffset);
        #endif
        ulMatchedId    = EBML_CRC_ID;
        bMaster        = false;
        m_eParserState = MKAVPARSER_SKIP_EBML_ID;
      }
      //! If element is unknown, do not break the loop. Just skip it.
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "Unknown element at level#0 is found at offset %llu",ullOffset);
        bok     = true;
        bMaster = false;
      }
      //Update Index to start of data
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
      if(bok)
      {
        if(bMaster)
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "ParseSegmentElement ebmlSizeBytes %d size %llu",
            ucEBMLSizeBytes, ullSize);
          //Initialize status as success
          MKAV_API_STATUS retStatus = MKAV_API_SUCCESS;

          if(ulMatchedId == EBML_TRACKS_ID)
          {
            retStatus = ParseTracksElement(ullOffset + ulIndex, ullSize);
          }
          else if(ulMatchedId == EBML_SEEK_HEAD_ID)
          {
            retStatus = ParseSeekHeadElement(ullOffset + ulIndex, ullSize);
          }
          else if(ulMatchedId == EBML_CLUSTER_ID)
          {
            retStatus = ParseClusterElement(ullOffset + ulIndex, ullSize,
                                            NULL, (uint8)ulIndex);
          }
          else if(ulMatchedId == EBML_SEGMENT_INFO_ID)
          {
            retStatus = ParseSegmentInfo(ullOffset + ulIndex, ullSize);
          }
          else if(ulMatchedId == EBML_CUES_ID)
          {
            retStatus = ParseCuesInfo(ullOffset + ulIndex, ullSize);
          }
          else if(EBML_TAGS_ID == ulMatchedId)
          {
            retStatus = ParseTagsElement(ullOffset + ulIndex, ullSize);
          }

          if(MKAV_API_SUCCESS != retStatus)
          {
            bok     = false;
            noerror = false;
            eStatus = retStatus;
          }
        }//if(bMaster)

        //Update to start of the next master element
        ullOffset = ullOffset + ulIndex + ullSize;
        if(bupdateoffset && m_bHttpPlay && noerror)
        {
          m_nCurrOffset = ullOffset;
        }
      }//if(bok)
      //! If corrupted atom is found in between, then break loop
      else if ((0 == ulIndex) || (0 == ullSize))
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "ParseSegmentElement corrupted element @ %llu",
                     ullOffset);
        break;
      }
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                     "ParseSegmentElement unsupported element @ %llu",
                     ullOffset);
        //Unsupported/unknown element is found, skip it and continue parsing
        m_nCurrOffset = ullOffset = ullOffset + ulIndex + ullSize;
      }
    }
  }//! while( (ullOffset < ullEndOffset)&&(noerror) )
  if(noerror)
  {
    eStatus = MKAV_API_SUCCESS;
    if((m_bHttpPlay) && (IsMetaDataParsingDone()) )
    {
      m_eParserPrvState = m_eParserState;
      m_eParserState    = MKAVPARSER_READY;
    }
  }
  return eStatus;
}

/* =============================================================================
FUNCTION:
 MKAVParser::ParseTrackElement

DESCRIPTION:
Parse and stores track element header

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
MKAVPARSER_SUCCESS if successful in parsing the header,
otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS  MKAVParser::ParseTracksElement(uint64 ullOffset,
                                                uint64 ullElementSize)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  uint32 ulIndex       = 0;
  uint32 ulDataRead    = 0;
  uint32 ulBytesToRead = DEF_EBML_MAX_SIGNATURE_LENGTH_VAL;
  uint64 ullEndOffset  = ullOffset + ullElementSize;
  bool noerror         = true;
  MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_MEDIUM,"ParseTracksElement %llu",ullOffset);
  /* If Tracks element is already parsed, then skip.
     It reduces unnecessary additional overhead. */
  if (m_pTrackEntry)
  {
    status  = MKAV_API_SUCCESS;
    noerror = false;
  }
  if(m_bHttpPlay && !EnsureDataCanBeRead(ullOffset, ullElementSize))
  {
    noerror = false;
    status  = MKAV_API_DATA_UNDERRUN;
  }
  while( (ullOffset < ullEndOffset)&& (noerror) )
  {
    ulIndex    = 0;
    ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                   m_pDataBuffer, DEF_DATA_BUF_SIZE);
    if( ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL)
    {
      status  = MKAV_API_FAIL;
      noerror = false;
    }
    else
    {
      uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
      uint8 ucEBMLSizeBytes = 0;
      uint64 ullSize        = AtomSize(m_pDataBuffer + ucEBMLIdBytes,
                                       &ucEBMLSizeBytes);

      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM, "ebmlSizeBytes %d size %llu",
                   ucEBMLSizeBytes, ullSize);
      #endif

      ulIndex = ucEBMLIdBytes + ucEBMLSizeBytes;
      if(EBML_TRACK_ENTRY_ID == m_pDataBuffer[0] )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "ParseTrackEntryElement called at offset %llu",
                     ullOffset);
        #endif
        status = ParseTrackEntryElement(ullOffset + ulIndex, ullSize);
        if(MKAV_API_SUCCESS != status)
        {
          noerror = false;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
           "ParserTracksElement ParseTrackEntryElement failed at offset %llu",
               ullOffset);
        }
      }
      //Update offset to start of next atom
      ullOffset = ullOffset + ulIndex + ullSize;
    }
  }//while( (ullOffset < ullEndOffset)&& (noerror) )
  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* =============================================================================
FUNCTION:
 MKAVParser::ParseSeekHeadElement

DESCRIPTION:
Parse and stores SEEK HEAD information

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
MKAVPARSER_SUCCESS if successful in parsing the header, otherwise,
returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS  MKAVParser::ParseSeekHeadElement(uint64 ullOffset,
                                                  uint64 ullElemSize)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseSeekHeadElement %llu",
               ullElemSize);
  #endif
  uint32 ulBytesToRead = DEF_EBML_MAX_SIGNATURE_LENGTH_VAL;
  uint32 ulIndex       = 0;
  uint64 ullEndOffset  = ullOffset + ullElemSize;
  seekhead* pSeekHead  = NULL;
  bool noerror         = true;
  if(m_bHttpPlay && !EnsureDataCanBeRead(ullOffset, ullElemSize))
  {
    return MKAV_API_DATA_UNDERRUN;
  }
  if(!m_pSeekHeadInfo)
  {
    m_pSeekHeadInfo = (seekheadinfo*)MM_Malloc( sizeof(seekheadinfo) );
    if(m_pSeekHeadInfo)
    {
      memset(m_pSeekHeadInfo,0,sizeof(seekheadinfo));
      m_pSeekHeadInfo->nSeekHead++;
      if(!m_pSeekHeadInfo->pSeekHead)
      {
        m_pSeekHeadInfo->pSeekHead = (seekhead*)MM_Malloc(sizeof(seekhead));
        if(m_pSeekHeadInfo->pSeekHead)
        {
          memset(m_pSeekHeadInfo->pSeekHead,0,sizeof(seekhead));
          pSeekHead = m_pSeekHeadInfo->pSeekHead;
        }
      }
    }
  }
  else
  {
    seekhead* temp = (seekhead*)MM_Realloc(m_pSeekHeadInfo->pSeekHead,
                           (sizeof(seekhead)*(m_pSeekHeadInfo->nSeekHead+1)) );
    if(temp)
    {
      m_pSeekHeadInfo->pSeekHead = temp;
      memset(m_pSeekHeadInfo->pSeekHead+m_pSeekHeadInfo->nSeekHead, 0,
             sizeof(seekhead));
      pSeekHead = m_pSeekHeadInfo->pSeekHead+m_pSeekHeadInfo->nSeekHead;
      m_pSeekHeadInfo->nSeekHead++;
    }
  }

  if((!m_pSeekHeadInfo) || (!pSeekHead))
  {
    status  = MKAV_API_OUT_OF_MEMORY;
    noerror = false;
  }

  while( (ullOffset < ullEndOffset) && (noerror) )
  {
    ulIndex = 0;
    uint32 ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                          m_pDataBuffer,DEF_DATA_BUF_SIZE);
    if( ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL)
    {
      status  = MKAV_API_FAIL;
      noerror = false;
    }
    else
    {
      uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
      uint8 ucEBMLSizeBytes = 0;
      uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIdBytes,
                                       &ucEBMLSizeBytes);
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "ParseSeekHeadElement ebmlSizeBytes %d size %llu",
                   ucEBMLSizeBytes, ullSize);
      #endif

      //Update index to start of data
      ulIndex = ucEBMLIdBytes + ucEBMLSizeBytes;
      if(!memcmp(m_pDataBuffer, &EBML_SEEK_ELEMENT_ID,
                 sizeof(EBML_SEEK_ELEMENT_ID)) )
      {
        status = ParseSeekElement(ullOffset + ulIndex, ullSize, pSeekHead);
        if(MKAV_API_SUCCESS != status)
        {
          noerror = false;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                      "ParseSeekHeadElement failed at offset %llu", ullOffset);
        }
      }
      //Go to start of next chunk/atom
      ullOffset += (ulIndex + ullSize);
    }//end of else
  }//while( (ullOffset < nEndOffset)&&(noerror) )
  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* ============================================================================
  @brief  Parse and stores cluster element header.

  @details    This function is used to parse Cluster element.

  @param[in]      ullOffset      Start Offset from which we need to read data.
  @param[in]      ullClusterSize Cluster element size.
  @param[in/out]  pCluster       Current cluster struct pointer.
                                 If this pointer is NULL, then this function
                                 will allocate memory to one similar structure
                                 and store in class object.
  @param[in/out]  nhdrSize       Cluster header signature length.

  @return     MKAV_API_SUCCESS indicating sample read successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
MKAV_API_STATUS  MKAVParser::ParseClusterElement(uint64        ullOffset,
                                                 uint64        ullClusterSize,
                                                 cluster_info* pCluster,
                                                 uint8         nhdrSize)
{
  MKAV_API_STATUS status = MKAV_API_OUT_OF_MEMORY;
  #ifdef MKAV_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseClusterElement noffset %llu",
                 ullOffset);
  #endif
  uint32 ulBytesToRead       = MAX_ELEMENT_SIZE;
  uint32 ulDataRead          = 0;
  uint32 ulIndex             = 0;
  uint64 ullEndOffset        = ullOffset + ullClusterSize;
  cluster_info* pClusterInfo = NULL;
  bool noerror               = false;
#ifndef _ANDROID_
  /* Validate whether complete Cluster is downloaded or not, during media
     sample processing. pCluster will be NULL during Metadata processing. */
  if((m_bHttpPlay) && (!EnsureDataCanBeRead(ullOffset, ullClusterSize)) &&
     (pCluster))
  {
    return MKAV_API_DATA_UNDERRUN;
  }
#endif
  if(!m_pAllClustersInfo)
  {
    m_pAllClustersInfo = (all_clusters_info*)
                         MM_Malloc( sizeof(all_clusters_info) );
    if(m_pAllClustersInfo)
    {
      memset(m_pAllClustersInfo, 0, sizeof(all_clusters_info));
      m_pAllClustersInfo->nClusters++;
      if(!m_pAllClustersInfo->pClusterInfo)
      {
        m_pAllClustersInfo->pClusterInfo = (cluster_info*)
                                           MM_Malloc(sizeof(cluster_info));
        if(m_pAllClustersInfo->pClusterInfo)
        {
          memset(m_pAllClustersInfo->pClusterInfo, 0, sizeof(cluster_info));
          pClusterInfo = m_pAllClustersInfo->pClusterInfo;
          pClusterInfo->nOffset  = ullOffset;
          pClusterInfo->nSize    = ullClusterSize;
          pClusterInfo->nHdrSize = nhdrSize;
          noerror = true;
        }
      }
    }
  }
  else if(pCluster == NULL)
  {
    uint32 ulNewClusterSize = (int)sizeof(cluster_info)*
                              (m_pAllClustersInfo->nClusters + 1);
    cluster_info* temp = (cluster_info*)
                          MM_Realloc(m_pAllClustersInfo->pClusterInfo,
                                     ulNewClusterSize);
    if(temp)
    {
      m_pAllClustersInfo->pClusterInfo = temp;
      memset(m_pAllClustersInfo->pClusterInfo +
             m_pAllClustersInfo->nClusters,
             0, sizeof(cluster_info));
      pClusterInfo = m_pAllClustersInfo->pClusterInfo +
                     m_pAllClustersInfo->nClusters;
      pClusterInfo->nOffset  = ullOffset;
      pClusterInfo->nSize    = ullClusterSize;
      pClusterInfo->nHdrSize = nhdrSize;
      m_pAllClustersInfo->nClusters++;
      noerror = true;
    }
  }
  else if(pCluster)
  {
    /* If cluster to parse is known, then calculate endOffset for cluster. */
    ullEndOffset = m_pAllClustersInfo->pClusterInfo[m_nCurrCluster].nOffset +
                   ullClusterSize;
    pClusterInfo = pCluster;
    noerror      = true;
    /* In streaming case, Data under-run may come at Cluster boundary. In such
       case, Offset value will not account for cluster header, so update offset
       to start of first child atom/object in the present cluster. */
    if (ullOffset < m_pAllClustersInfo->pClusterInfo[m_nCurrCluster].nOffset)
    {
      ullOffset = m_pAllClustersInfo->pClusterInfo[m_nCurrCluster].nOffset;
    }
  }
  /* If Offset value is greater than Cluster End offset value, then exit. */
  if(ullOffset >= ullEndOffset)
  {
    noerror = false;
  }

  /* Parse Cluster media data to get Timestamp. Typically Timestamp available
     as part of first few atoms only. */
  while(((ullOffset + ulIndex) < ullEndOffset) && (noerror) )
  {
    /* Read data from file only if all the data is consumed or for the first
       time. */
    if(!ulDataRead ||
      (ulDataRead < uint64(ullEndOffset - ullOffset) &&
      (ulDataRead - ulIndex) < DEF_EBML_MX_SZ_LENGTH_VAL))
    {
      ullOffset += ulIndex;
      ulIndex    = 0;
      ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                     m_pDataBuffer, DEF_DATA_BUF_SIZE);
      if(ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL)
      {
        MapParserStatetoStatus(m_eParserState, status);
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                     "ParseClusterElement failed at noffset %llu, error %d",
                     ullOffset, status);
        noerror = false;
        break;
      }
    }
    else if(ulDataRead == ulIndex)
    {
      status = MKAV_API_SUCCESS;
      break;
    }
    uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
    uint8 ucEBMLSizeBytes = 0;
    uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);

    if(ullSize > ullClusterSize)
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
          "ParseClusterElement: atom size %llu is wrong at offset %llu",
          ullSize, ullOffset);
      noerror = false;
      break;
    }
    /* Check whether complete atom is read into local buffer or not*/
    if(ullSize > (ulDataRead - ulIndex) && ullSize < MAX_ELEMENT_SIZE)
    {
      ulDataRead = 0;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseClusterElement: complete atom is not read");
      #endif
      continue;
    }

    //! If any atom ID or Size bytes are ZERO, then report EOF and exit playback
    if ((0 == ucEBMLIdBytes) || (0 == ucEBMLIdBytes))
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "ParseClusterElement corrupted at noffset %llu",
                   ullOffset + ulIndex);
      status            = MKAV_API_EOF;
      m_eParserPrvState = m_eParserState;
      m_eParserState    = MKAVPARSER_EOF;
      noerror = false;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_CLUSTER_TIMECODE_ID,
                    sizeof(EBML_CLUSTER_TIMECODE_ID)) )
    {
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "ParseClusterElement EBML_CLUSTER_TIMECODE_ID %llu", ullOffset);
      memcpy(&pClusterInfo->nTimeCode,m_pDataBuffer+ ulIndex, (size_t)ullSize);
      MKV_REVERSE_ENDIAN((uint8*)&pClusterInfo->nTimeCode,(size_t)ullSize);
      /* If pCluster is NULL, it indicates Metadata Processing. Parser requires
         Timestamp only as part of metadata along with Cluster Header and Size
         fields. Break the while loop.
      */
      if (!pCluster)
      {
        break;
      }
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_CLUSTER_POSN_ID,
                    sizeof(EBML_CLUSTER_POSN_ID)) )
    {
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "ParseClusterElement EBML_CLUSTER_POSN_ID %llu", ullOffset);
      memcpy(&pClusterInfo->nPosn, m_pDataBuffer+ ulIndex, (size_t)ullSize);
      MKV_REVERSE_ENDIAN((uint8*)&pClusterInfo->nPosn, (size_t)ullSize);
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_CLUSTER_PRVSIZE_ID,
                    sizeof(EBML_CLUSTER_PRVSIZE_ID)) )
    {
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "ParseClusterElement EBML_CLUSTER_PRVSIZE_ID %llu",ullOffset);
      memcpy(&pClusterInfo->nPrevSize,m_pDataBuffer + ulIndex,(size_t)ullSize);
      MKV_REVERSE_ENDIAN((uint8*)&pClusterInfo->nPrevSize,(size_t)ullSize);
    }
    /* If pCluster is not NULL, then it indicates Media Data processing.
       Parse Simple Block or Block Group elements only in case of media data
       processing. */
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_CLUSTER_BLOCKGROUP_ID,
                    sizeof(EBML_CLUSTER_BLOCKGROUP_ID)) && (pCluster))
    {
      uint64 ullBlockGroupOffset = ullOffset + ulIndex;
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
             "ParseClusterElement EBML_CLUSTER_BLOCKGROUP_ID %llu", ullOffset);

      status = ParseBlockGroupElement(ullOffset+ ulIndex,ullSize,pClusterInfo);
      if (MKAV_API_SUCCESS != status)
      {
        noerror = false;
      }
      //! Store Block Group Start offset value.
      else
      {
        pClusterInfo->pBlockGroup->nBlockStartOffset = ullBlockGroupOffset;
      }
      //When retrieving samples, we should o/p only one block at a time.
      break;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_CLUSTER_SIMPLE_BLOCK_ID,
                   sizeof(EBML_CLUSTER_SIMPLE_BLOCK_ID)) && (pCluster))
    {
      uint64 ullBlockOffset = ullOffset + ulIndex;
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                   "ParseClusterElement EBML_CLUSTER_SIMPLE_BLOCK_ID %llu",
                   ullOffset);
      status = ParseBlockElement(ullOffset + ulIndex, ullSize, pClusterInfo);
      if (MKAV_API_SUCCESS != status)
      {
        noerror = false;
      }
      else
      {
        //! Store block header size. This is used to calculate the sync sample
        //! offset in case CUES table is not present.
        pClusterInfo->pSimpleBlocks->nBlockHeaderSize = (uint8)
          (pClusterInfo->pSimpleBlocks->nDataOffset - ullBlockOffset);
      }
      //When retrieving samples, we should o/p only one block at a time.
      break;
    }
    else
    {
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
      /* If sufficient data is not available in the local buffer, then reset
         DataRead value, so that Parser will read one more time from new offset
         value. */
      if(ullSize > (ulDataRead - ulIndex))
      {
        ulDataRead = 0;
      }
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "ParseClusterElement don't find related element, skip");
    }
    /* If any element size is more than cluster, then clip is corrupted.
       Exit gracefully in such scenario. */
    if (ullSize > ullClusterSize)
    {
      noerror = false;
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
           "ParseClusterElement ullSize %llu > ullClusterSize %llu, corrupted",
           ullSize, ullClusterSize);
      status = MKAV_API_EOF;
    }

    ulIndex += (uint32)ullSize;
  }//while( (noffset < nEndOffset)&&(noerror) )

  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* =============================================================================
FUNCTION:
 MKAVParser::ParseBlockGroupElement

DESCRIPTION:
Parse and stores BLOCKGROUP information

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
MKAVPARSER_SUCCESS if successful in parsing, otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS MKAVParser::ParseBlockGroupElement(uint64 ullOffset,
                                                   uint64 ullElemize,
                                                   cluster_info* pClusterInfo)
{
#ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseBlockGroupElement %llu",
               ullOffset);
#endif
  MKAV_API_STATUS status       = MKAV_API_SUCCESS;
  uint32 ulIndex               = 0;
  uint32 ulNumBlocks           = 0;
  uint32 ulDataRead            = 0;
  uint32 ulBytesToRead         = MAX_ELEMENT_SIZE;
  uint64 ullEndOffset          = ullOffset + ullElemize;
  blockgroup_info *pBlockGroup = NULL;
  blockinfo       *pBlockInfo  = NULL;
  bool noerror = true;

  if(pClusterInfo)
  {
    /* If Memory is allocated to Simple Block, free that memory.
       We use single pointer for both BlockGroup and SimpleBlock while parsing
       block data. To avoid any confusion, free memory which is not required.
    */
    if(pClusterInfo->pSimpleBlocks)
    {
      ResetCurrentClusterInfo();
    }
    if(!pClusterInfo->pBlockGroup)
    {
      pClusterInfo->pBlockGroup = (blockgroup_info*)
                                   MM_Malloc(sizeof(blockgroup_info));
      if(pClusterInfo->pBlockGroup)
      {
        memset(pClusterInfo->pBlockGroup,0,sizeof(blockgroup_info));
        pClusterInfo->nBlockGroup++;
      }
    }
    pBlockGroup = pClusterInfo->pBlockGroup;
  }
  if(!pClusterInfo || !pClusterInfo->pBlockGroup)
  {
    status  = MKAV_API_OUT_OF_MEMORY;
    noerror = false;
  }
  //! Update Bock group end offset value
  else if(pBlockGroup)
  {
    pBlockGroup->nBlockEndOffset = ullEndOffset;
    pBlockGroup->nRefBlock       = 0;
  }
  while( ((ullOffset + ulIndex) < ullEndOffset) && (noerror))
  {
    /* Read data from file only if all the data is consumed or for the first
       time. */
    if((!ulDataRead) || (ulDataRead < ulIndex) ||
       (ulDataRead < uint32(ullEndOffset - ullOffset) &&
       (ulDataRead - ulIndex) < DEF_EBML_MX_SZ_LENGTH_VAL))
    {
      ullOffset += ulIndex;
      ulIndex    = 0;
      ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                     m_pDataBuffer, DEF_DATA_BUF_SIZE);
      if(ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL)
      {
        MapParserStatetoStatus(m_eParserState, status);
        noerror = false;
        break;
      }
    }
    else if(ulDataRead == ulIndex)
    {
      status = MKAV_API_SUCCESS;
      break;
    }
    uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
    uint8 ucEBMLSizeBytes = 0;
    uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);

    /* Check whether complete atom is read into local buffer or not*/
    if(ullSize > (ulDataRead - ulIndex) && ullSize < MAX_ELEMENT_SIZE)
    {
      ulDataRead = ulIndex;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseBlockGroupElement: complete atom is not read");
      #endif
      continue;
    }

    if(memcmp(m_pDataBuffer + ulIndex, &EBML_CLUSTER_BLOCK_DUR_ID,
              sizeof(EBML_CLUSTER_BLOCK_DUR_ID)) == 0)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseBlockGroupElement EBML_CLUSTER_BLOCK_DUR_ID %llu", ullOffset);
      #endif
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
      memcpy(&pBlockGroup->nBlockDuration, m_pDataBuffer + ulIndex,
             (size_t)ullSize);
      MKV_REVERSE_ENDIAN((uint8*)&pBlockGroup->nBlockDuration,(size_t)ullSize);
    }
    else if(memcmp(m_pDataBuffer + ulIndex, &EBML_CLUSTER_REFERENCE_BLOCK_ID,
                   sizeof(EBML_CLUSTER_REFERENCE_BLOCK_ID)) == 0)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
      "ParseBlockGroupElement EBML_CLUSTER_REFERENCE_BLOCK_ID %llu",ullOffset);
      #endif
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;

      bool isNegative = false;
      memcpy(&pBlockGroup->nRefBlock, m_pDataBuffer + ulIndex,(size_t)ullSize);
      MKV_REVERSE_ENDIAN((uint8*)&pBlockGroup->nRefBlock, (size_t)ullSize);
      convertEBMLtoINT(m_pDataBuffer + ulIndex, (size_t)ullSize,
                       &pBlockGroup->nRefBlock, &isNegative);
      if(!isNegative)
      {
        pBlockGroup->nRefBlock = pClusterInfo->nTimeCode +
                                 pBlockGroup->nRefBlock;
      }
      else
      {
        pBlockGroup->nRefBlock = pClusterInfo->nTimeCode -
                                 (pBlockGroup->nRefBlock + 1);
      }
    }
    else if(memcmp(m_pDataBuffer + ulIndex,&EBML_CLUSTER_BLOCK_ID,
                   sizeof(EBML_CLUSTER_BLOCK_ID)) == 0)
    {
      /* Read Block Data to reset Index value. */
      ulIndex   += ucEBMLIdBytes + ucEBMLSizeBytes;
      ullOffset += ulIndex;
      ulIndex = 0;
      ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                     m_pDataBuffer, DEF_DATA_BUF_SIZE);
      if(ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL)
      {
        MapParserStatetoStatus(m_eParserState, status);
        noerror = false;
        break;
      }
      /* Counter to check number of block available in one blockGroup */
      ulNumBlocks++;
      bool bMemAllocated = false;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseBlockGroupElement EBML_CLUSTER_BLOCK_ID %llu",ullOffset);
      #endif
      if(!pBlockGroup->pBlockInfo)
      {
        pBlockGroup->pBlockInfo = (blockinfo*)MM_Malloc(sizeof(blockinfo));
        bMemAllocated = true;
      }
      else if(ulNumBlocks > pBlockGroup->nBlockinfo)
      {
        pBlockGroup->pBlockInfo = (blockinfo*)MM_Realloc(
                                  pBlockGroup->pBlockInfo,
                                  (ulNumBlocks) * sizeof(blockinfo));
        bMemAllocated = true;
      }
      if(pBlockGroup->pBlockInfo)
      {
        pBlockInfo = &pBlockGroup->pBlockInfo[ulNumBlocks - 1];
        pBlockGroup->nBlocksParsed = 0;
        if(bMemAllocated)
        {
          memset(pBlockGroup->pBlockInfo + (ulNumBlocks - 1), 0,
                 sizeof(blockinfo));
          pBlockGroup->nBlockinfo++;
        }
      }
      else
      {
        noerror = false;
        status  = MKAV_API_OUT_OF_MEMORY;
        continue;
      }

      pBlockInfo->nFramesParsed = 0;
      pBlockInfo->nTrackNumber  = (uint32)AtomSize(m_pDataBuffer + ulIndex,
                                                   &ucEBMLSizeBytes);
      ulIndex += ucEBMLSizeBytes;
      memcpy(&pBlockInfo->nTimeCode, m_pDataBuffer + ulIndex, 2);
      MKV_REVERSE_ENDIAN((uint8*)&pBlockInfo->nTimeCode, 2);
      ulIndex += 2;
      pBlockInfo->nFlags = m_pDataBuffer[ulIndex];
      ulIndex++;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"FLAGS %x",
                   pBlockInfo->nFlags);
      #endif
      pBlockInfo->nFrames = 1;
      /* If seek operation is going on, then Parser will not extra the sample
         properties. This is to reduce the overhead on seek operation. */
      if( (MKAVPARSER_SEEKING != m_eParserState) &&
          (EBML_LACING_ID == (pBlockInfo->nFlags & EBML_LACING_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_LACING");
        #endif
        //Get the number of frames.
        memcpy(&pBlockInfo->nFrames, m_pDataBuffer + ulIndex, sizeof(uint8));
        pBlockInfo->nFrames++;
        ulIndex++;

        status = CalcFrameSizes(pBlockInfo, ullOffset, ulDataRead, ullSize,
                                ulIndex);
      }
      else if( FIXED_SIZE_LACING_ID ==
               (pBlockInfo->nFlags & FIXED_SIZE_LACING_ID) )
      {
        //Get num of frames. In fixed size lacing, all frames are of same size
        memcpy(&pBlockInfo->nFrames,
               m_pDataBuffer + ulIndex,sizeof(uint8));
        pBlockInfo->nFrames++;
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"FIXED_SIZE_LACING #Frames %d",
                      pBlockInfo->nFrames);
        #endif
        ulIndex++;
        /* If memory is already allocated to store frame size info.
           Free that memory, as it is not required for FIXED size lacing mode.
        */
        if(pBlockInfo->pFramesSize)
        {
          MM_Free(pBlockInfo->pFramesSize);
          pBlockInfo->pFramesSize = NULL;
        }
      }
      /* If seek operation is going on, then Parser will not extra the sample
         properties. This is to reduce the overhead on seek operation. */
      else if( (MKAVPARSER_SEEKING != m_eParserState) &&
               (XIPH_LACING_ID == (pBlockInfo->nFlags & XIPH_LACING_ID)))
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"XIPH_LACING");
        #endif
        //Get the number of frames.
        memcpy(&pBlockInfo->nFrames, m_pDataBuffer + ulIndex, sizeof(uint8));
        pBlockInfo->nFrames++;
        ulIndex++;

        status = CalcFrameSizes(pBlockInfo, ullOffset, ulDataRead, ullSize,
                                ulIndex);
      }
      else if( (pBlockInfo->nFlags & INVISIBLE_FRAME_ID)== INVISIBLE_FRAME_ID)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"INVISIBLE_FRAME");
      }
      pBlockInfo->nDataSize   = (uint32)ullSize - ulIndex;
      pBlockInfo->nDataOffset = ullOffset + ulIndex;
      //Reset Index to ZERO, as Block group consumed all metadata related info.
      ulIndex = 0;
    }
    else
    {
      /* In case of unsupported atoms, skip the header part. */
      ulIndex   += ucEBMLIdBytes + ucEBMLSizeBytes;
    }
    ulIndex += (uint32)ullSize;
  }
  return status;
}

/* =============================================================================
FUNCTION:
 MKAVParser::ParseBlockElement

DESCRIPTION:
Parse and stores individual BLOCK information

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
MKAVPARSER_SUCCESS if successful in parsing, otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS MKAVParser::ParseBlockElement(uint64 ullOffset,
                                              uint64 ullElemSize,
                                              cluster_info* pClusterInfo)
{
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "ParseBlockElement %llu", ullOffset);
  #endif
  MKAV_API_STATUS status  = MKAV_API_SUCCESS;
  int ulBytesToRead       = MAX_ELEMENT_SIZE;
  uint32 ulDataRead       = 0;
  uint32 ulIndex          = 0;
  blockinfo *pSimpleBlock = NULL;
  bool noerror            = true;

  if(pClusterInfo)
  {
    /* If Memory is allocated to BlockGroup, free that memory.
       We use single pointer for both BlockGroup and SimpleBlock while parsing
       block data. To avoid any confusion, free memory which is not required.
       */
    if(pClusterInfo->pBlockGroup)
    {
      ResetCurrentClusterInfo();
    }
    if(!pClusterInfo->pSimpleBlocks)
    {
      pClusterInfo->pSimpleBlocks = (blockinfo*)MM_Malloc(sizeof(blockinfo));
      if(pClusterInfo->pSimpleBlocks)
      {
        memset(pClusterInfo->pSimpleBlocks, 0, sizeof(blockinfo));
        pClusterInfo->nSimpleBlocks++;
      }
    }
    pSimpleBlock = &pClusterInfo->pSimpleBlocks[pClusterInfo->nSimpleBlocks-1];
  }
  if(!pClusterInfo || !pSimpleBlock)
  {
    status  = MKAV_API_OUT_OF_MEMORY;
    noerror = false;
  }
  else
  {
    pSimpleBlock->nFramesParsed = 0;
    /* Read MAX_ELEMENT_SIZE bytes of data to parse Block header data. */
    ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                   m_pDataBuffer, DEF_DATA_BUF_SIZE);
    if( !ulDataRead )
    {
      MapParserStatetoStatus(m_eParserState, status);
      noerror = false;
    }
  }
  if(noerror)
  {
    uint8 ucEBMLSizeBytes = 0;
    pSimpleBlock->nTrackNumber = (uint32)AtomSize(m_pDataBuffer + ulIndex,
                                                  &ucEBMLSizeBytes);
    ulIndex += ucEBMLSizeBytes;
    memcpy(&pSimpleBlock->nTimeCode, m_pDataBuffer + ulIndex, 2);
    MKV_REVERSE_ENDIAN((uint8*)&pSimpleBlock->nTimeCode, 2);
    ulIndex += 2;
    pSimpleBlock->nFlags = m_pDataBuffer[ulIndex];
    ulIndex++;
    #ifdef MKAV_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"FLAGS %x", pSimpleBlock->nFlags);
    #endif

    /* If no lacing method is not used, Parser will assume entire data in block
       belongs one frame only.
    */
    pSimpleBlock->nFrames = 1;
    /* If seek operation is going on, then Parser will not extra the sample
       properties. This is to reduce the overhead on seek operation. */
    if( (MKAVPARSER_SEEKING != m_eParserState) &&
        (EBML_LACING_ID == (pSimpleBlock->nFlags & EBML_LACING_ID)))
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_LACING");
      #endif
      //Get the number of frames.
      memcpy(&pSimpleBlock->nFrames, m_pDataBuffer + ulIndex, sizeof(uint8));
      pSimpleBlock->nFrames++;
      ulIndex++;

      status = CalcFrameSizes(pSimpleBlock, ullOffset, ulDataRead,
                              ullElemSize, ulIndex);
    }
    else if( FIXED_SIZE_LACING_ID ==
             (pSimpleBlock->nFlags & FIXED_SIZE_LACING_ID))
    {
      //Get number of frames. In fixed size lacing, all frames are of same size
      memcpy(&pSimpleBlock->nFrames, m_pDataBuffer + ulIndex, sizeof(uint8));
      pSimpleBlock->nFrames++;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FIXED_SIZE_LACING #Frames %d",
                    pSimpleBlock->nFrames);
      #endif
      ulIndex++;
      /* If memory is already allocated to store frame size info.
         Free that memory, as it is not required for FIXED size lacing mode.
      */
      if(pSimpleBlock->pFramesSize)
      {
        MM_Free(pSimpleBlock->pFramesSize);
        pSimpleBlock->pFramesSize = NULL;
      }
    }
    /* If seek operation is going on, then Parser will not extra the sample
       properties. This is to reduce the overhead on seek operation. */
    else if( (MKAVPARSER_SEEKING != m_eParserState) &&
             (XIPH_LACING_ID == (pSimpleBlock->nFlags & XIPH_LACING_ID)))
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "XIPH_LACING");
      #endif
      //Get the number of frames.
      memcpy(&pSimpleBlock->nFrames, m_pDataBuffer + ulIndex, sizeof(uint8));
      pSimpleBlock->nFrames++;
      ulIndex++;

      status = CalcFrameSizes(pSimpleBlock, ullOffset, ulDataRead,
                              ullElemSize, ulIndex);
    }
    else if( (pSimpleBlock->nFlags & INVISIBLE_FRAME_ID)== INVISIBLE_FRAME_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"INVISIBLE_FRAME");
      #endif
    }
    pSimpleBlock->nDataSize   = (uint32)ullElemSize - ulIndex;
    pSimpleBlock->nDataOffset = ullOffset + ulIndex;
  }
  return status;
}

/* =============================================================================
FUNCTION:
 MKAVParser::ParseSeekElement

DESCRIPTION:
Parse and stores seek element information

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
MKAVPARSER_SUCCESS if successful in parsing,
otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS  MKAVParser::ParseSeekElement(uint64 ullOffset,
                                              uint64 ullElemSize,
                                              seekhead* pseekhead)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  uint32 ulIndex         = 0;
  uint32 ulBytesToRead   = (uint32)ullElemSize;
  uint32 ulDataRead      = 0;
  uint64 ullEndOffset    = ullOffset + ullElemSize;
  seek_info* pSeekInfo   = NULL;
  bool noerror           = false;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "ParseSeekElement %llu", ullOffset);
  #endif
  if(!pseekhead->pSeekInfo)
  {
    //we will be parsing first seek element in the given seekhead
    pseekhead->pSeekInfo = (seek_info*)MM_Malloc(sizeof(seek_info));
    if(pseekhead->pSeekInfo)
    {
      memset(pseekhead->pSeekInfo,0,sizeof(seek_info));
      pSeekInfo = pseekhead->pSeekInfo;
      pseekhead->nSeekInfo++;
      noerror = true;
    }
  }
  else
  {
    //we encountered additional seek element in given seekhead
    seek_info* temp = (seek_info*)MM_Realloc(pseekhead->pSeekInfo,
                       sizeof(seek_info)*(pseekhead->nSeekInfo + 1));
    if(temp)
    {
      pseekhead->pSeekInfo = temp;
      memset(pseekhead->pSeekInfo+pseekhead->nSeekInfo,0,sizeof(seek_info));
      pSeekInfo = pseekhead->pSeekInfo+pseekhead->nSeekInfo;
      pseekhead->nSeekInfo++;
      noerror = true;
    }
  }

  while( (ullOffset < ullEndOffset) && (noerror) )
  {
    if(!ulDataRead ||
       ((ulDataRead - ulIndex) < DEF_EBML_MX_ID_LENGTH_VAL*2 &&
        (ullEndOffset - ullOffset) > (ulDataRead - ulIndex)))
    {
      ulBytesToRead = (uint32)(ullEndOffset - ullOffset);
      ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                     m_pDataBuffer, DEF_DATA_BUF_SIZE);
      if( !ulDataRead )
      {
        status  = MKAV_API_FAIL;
        noerror = false;
        break;
      }
      ulIndex = 0;
    }
    uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
    uint8 ucEBMLSizeBytes = 0;
    uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);

    #ifdef MKAV_PARSER_DEBUG
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "ParseSeekElement EBML_SEEK_ID ebmlSizeBytes %d size %llu",
                 ucEBMLSizeBytes, ullSize);
    #endif

    /* Check whether complete data is read into local buffer or not*/
    if(ullSize > (ulDataRead - ulIndex))
    {
      ulDataRead = ulIndex;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseSeekElement: complete atom is not read");
      #endif
      continue;
    }

    if(!memcmp(m_pDataBuffer + ulIndex, &EBML_SEEK_ID,
               sizeof(EBML_SEEK_ID)) )
    {
      ulIndex += (ucEBMLIdBytes + ucEBMLSizeBytes);
      memcpy(&pSeekInfo->ebml_id, m_pDataBuffer + ulIndex,(size_t)ullSize);
      MKV_REVERSE_ENDIAN((uint8*)&pSeekInfo->ebml_id, (size_t)ullSize);
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "ParseSeekElement EBML_SEEK_ID %lu ", pSeekInfo->ebml_id);
      #endif
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_SEEK_POSN_ID,
                    sizeof(EBML_SEEK_POSN_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseSeekElement EBML_SEEK_POSN_ID");
      #endif
      ulIndex += (ucEBMLIdBytes + ucEBMLSizeBytes);
      memcpy(&pSeekInfo->ebml_id_file_posn, m_pDataBuffer + ulIndex,
             (size_t)ullSize);
      MKV_REVERSE_ENDIAN((uint8*)&pSeekInfo->ebml_id_file_posn,
                         (size_t)ullSize);
      pSeekInfo->ebml_id_file_posn += (uint32)m_nSegmentPosn;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "ParseSeekElement EBML_SEEK_POSN_ID %lu ",
                   pSeekInfo->ebml_id_file_posn);
      #endif
    }
    else
    {
      ulIndex += (ucEBMLIdBytes + ucEBMLSizeBytes);
    }
    ulIndex   += (uint32)ullSize;
    ullOffset += ucEBMLIdBytes + ucEBMLSizeBytes + ullSize;
  }//while( (ullOffset < nEndOffset)&&(noerror) )
  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* ============================================================================
  @brief  Parse and stores Track Entry element header.

  @details    This function is used to parse Track entry element.

  @param[in]      ullOffset      Start Offset from which we need to read data.
  @param[in]      ullElementSize Clster element size.

  @return     MKAV_API_SUCCESS indicating parsing done successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
MKAV_API_STATUS  MKAVParser::ParseTrackEntryElement(uint64 ullOffset,
                                                    uint64 ullElementSize)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"ParseTrackEntryElement %llu",
               ullOffset);
  mkav_track_entry_info hTempTrackEntryInfo;
  mkav_video_info hTempVideoInfo;
  mkav_audio_info hTempAudioInfo;
  mkav_encode_info hTempContentInfo;

  uint32 ulIndex       = 0;
  uint32 ulDataRead    = 0;
  uint64 ullEndOffset  = ullOffset + ullElementSize;
  uint32 ulBytesToRead = uint32(ullEndOffset - ullOffset);
  bool noerror         = true;

  memset(&hTempTrackEntryInfo, 0, sizeof(mkav_track_entry_info));
  memset(&hTempVideoInfo, 0, sizeof(mkav_video_info));
  memset(&hTempAudioInfo, 0, sizeof(mkav_audio_info));
  memset(&hTempContentInfo, 0, sizeof(mkav_encode_info));
  hTempContentInfo.eEncodeType = UNKNOWN_ENCODE_TYPE;

  while( (ullOffset < ullEndOffset) && (noerror) )
  {
    bool bNeedAllocation   = false;
    bool bMaster           = false;
    bool bok               = true;
    size_t ulMaxTargetSize = 0;
    uint8* pMemory         = NULL;

    /* Read data from file only if all the data is consumed or for the first
       time. */
    if(!ulDataRead ||
      (ulDataRead < uint32(ullEndOffset - ullOffset) &&
      (ulDataRead - ulIndex) < DEF_EBML_MX_SZ_LENGTH_VAL))
    {
      ullOffset    += ulIndex;
      ulIndex       = 0;
      ulBytesToRead = uint32(ullEndOffset - ullOffset);
      ulDataRead    = GetDataFromSource(ullOffset, ulBytesToRead,
                                        m_pDataBuffer, DEF_DATA_BUF_SIZE);
      if((ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL) &&
         (ulDataRead < ulBytesToRead) )
      {
        status  = MKAV_API_FAIL;
        noerror = false;
        break;
      }
    }
    else if(ulDataRead == ulIndex)
    {
      status = MKAV_API_SUCCESS;
      break;
    }
    uint8 ucEBMLID        = m_pDataBuffer[ulIndex];
    uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
    uint8 ucEBMLSizeBytes = 0;
    uint32 ulMatchedId    = 0;
    uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);

    /* Check whether complete atom is read into local buffer or not. */
    if((ullSize > (ulDataRead - ulIndex)) &&
       (memcmp(m_pDataBuffer + ulIndex, &EBML_TRACK_CODEC_PVT_ID,
               sizeof(EBML_TRACK_CODEC_PVT_ID)) ) )
    {
      ulDataRead = 0;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseTrackEntryElement: complete atom is not read");
      #endif
      continue;
    }

    //Compare ID against standard indexes that can occur as part of tracks atom
    if(!memcmp(m_pDataBuffer + ulIndex, &EBML_DEF_DURATION_ID,
               EBML_ELEMENT_ID_SIZE_THREE) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_DEF_DURATION_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.DefaultDuration);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.DefaultDuration);
      ulMatchedId = EBML_DEF_DURATION_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TRACK_TIMECODESCALE_ID,
                    EBML_ELEMENT_ID_SIZE_THREE) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_TIMECODESCALE_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.TrackTimeCodeScale);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.TrackTimeCodeScale);
      ulMatchedId = EBML_TRACK_TIMECODESCALE_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TRACK_CODEC_NAME_ID,
                    EBML_ELEMENT_ID_SIZE_THREE) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_CODEC_NAME_ID");
      #endif
      bNeedAllocation = true;
      ulMatchedId = EBML_TRACK_CODEC_NAME_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TRACK_LANGUAGE_ID,
                    EBML_ELEMENT_ID_SIZE_THREE) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_LANGUAGE_ID");
      #endif
      bNeedAllocation = true;
      ulMatchedId = EBML_TRACK_LANGUAGE_ID;
    }
    //16 bit ids
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_MIN_CACHE_ID,
                    sizeof(EBML_MIN_CACHE_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_MIN_CACHE_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.MinCache);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.MinCache);
      ulMatchedId = EBML_MIN_CACHE_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_MX_CACHE_ID,
                    sizeof(EBML_MX_CACHE_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_MX_CACHE_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.MaxCache);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.MaxCache);
      ulMatchedId = EBML_MX_CACHE_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_FLG_FLG_FORCED_ID,
                    sizeof(EBML_FLG_FLG_FORCED_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_FLG_FLG_FORCED_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.FlagForced);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.FlagForced);
      ulMatchedId = EBML_FLG_FLG_FORCED_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TRACKUID_ID,
                    sizeof(EBML_TRACKUID_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACKUID_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.TrackUID);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.TrackUID);
      ulMatchedId = EBML_TRACKUID_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TRACK_CODEC_PVT_ID,
                    sizeof(EBML_TRACK_CODEC_PVT_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_CODEC_PVT_ID");
      #endif
      bNeedAllocation = true;
      ulMatchedId = EBML_TRACK_CODEC_PVT_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex,&EBML_TRACK_CONTENTS_ENCODING_ID,
                    sizeof(EBML_TRACK_CONTENTS_ENCODING_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_CONTENTS_ENCODING_ID");
      #endif
      bMaster = true;
      ulMatchedId = EBML_TRACK_CONTENTS_ENCODING_ID;
    }
    else if(!memcmp(m_pDataBuffer+ulIndex, &EBML_TRACK_ATTACHMENT_LINK_ID,
                    sizeof(EBML_TRACK_ATTACHMENT_LINK_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_ATTACHMENT_LINK_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.AttachmentLink);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.AttachmentLink);
      ulMatchedId = EBML_TRACK_ATTACHMENT_LINK_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TRACK_NAME_ID,
                    sizeof(EBML_TRACK_NAME_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_NAME_ID");
      #endif
      bNeedAllocation = true;
      ulMatchedId = EBML_TRACK_NAME_ID;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TRACK_CODEC_DELAY,
                    sizeof(EBML_TRACK_CODEC_DELAY)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_CODEC_DELAY");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.nCodecDelay);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.nCodecDelay);
      ulMatchedId = EBML_TRACK_CODEC_DELAY;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TRACK_SEEK_PREROLL,
                    sizeof(EBML_TRACK_SEEK_PREROLL)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_SEEK_PREROLL");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.nSeekPreRoll);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.nSeekPreRoll);
      ulMatchedId = EBML_TRACK_SEEK_PREROLL;
    }
    //8 bit ids
    else if(ucEBMLID == EBML_TRACKNO_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACKNO_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.TrackNo);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.TrackNo);
      ulMatchedId = EBML_TRACKNO_ID;
    }
    else if(ucEBMLID == EBML_TRACKTYPE_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACKTYPE_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.TrackType);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.TrackType);
      ulMatchedId = EBML_TRACKTYPE_ID;
    }
    else if(ucEBMLID == EBML_FLG_ENABLE_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_FLG_ENABLE_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.FlagEnabled);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.FlagEnabled);
      ulMatchedId = EBML_FLG_ENABLE_ID;
    }
    else if(ucEBMLID == EBML_FLG_DEF_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_FLG_DEF_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.FlagDefault);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.FlagDefault);
      ulMatchedId = EBML_FLG_DEF_ID;
    }
    else if(ucEBMLID == EBML_FLG_LACING_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_FLG_LACING_ID");
      #endif
      pMemory = (uint8*)&(hTempTrackEntryInfo.FlagLacing);
      ulMaxTargetSize = sizeof(hTempTrackEntryInfo.FlagLacing);
      ulMatchedId = EBML_FLG_LACING_ID;
    }
    else if(ucEBMLID == EBML_TRACK_CODEC_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_CODEC_ID");
      #endif
      ulMatchedId = EBML_TRACK_CODEC_ID;
    }
    else if(ucEBMLID == EBML_TRACK_VIDEO_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_VIDEO_ID");
      #endif
      ulMatchedId = EBML_TRACK_VIDEO_ID;
      bMaster = true;
    }
    else if(ucEBMLID == EBML_TRACK_AUDIO_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement EBML_TRACK_AUDIO_ID");
      #endif
      bMaster = true;
      ulMatchedId = EBML_TRACK_AUDIO_ID;
    }
    else if(ucEBMLID == EBML_VOID_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTrackEntryElement VOID_ID");
      #endif
      bok = false;
      ulMatchedId = EBML_VOID_ID;
    }
    else if(ucEBMLID == EBML_CRC_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "EBML_CRC_ID %llu",ullOffset);
      #endif
      bok = false;
      ulMatchedId = EBML_CRC_ID;
    }
    else
    {
      bok = false;
    }
    //Update Index to end of atom type and size fields
    ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
    if( (bok) && (ulMatchedId != 0) )
    {
      if(!bMaster)
      {
        //store read value here
#ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ebmlSizeBytes %d atom size %llu", ucEBMLSizeBytes, ullSize);
#endif
        if(!pMemory)
        {
          bool bAppendZero = true;
          pMemory = (uint8*)MM_Malloc((size_t)ullSize + 1);
          ulMaxTargetSize = (size_t)ullSize + 1;
          if(ulMatchedId == EBML_TRACK_CODEC_NAME_ID)
          {
            hTempTrackEntryInfo.pCodecName = pMemory;
          }
          else if(ulMatchedId == EBML_TRACK_LANGUAGE_ID)
          {
            hTempTrackEntryInfo.pLanguage = pMemory;
          }
          else if(ulMatchedId == EBML_TRACK_CODEC_PVT_ID)
          {
            //No need to append \0 for codec data
            hTempTrackEntryInfo.pCodecPvt = pMemory;
            hTempTrackEntryInfo.nCodecPvtSize = (uint32)ullSize;
            bAppendZero = false;
          }
          else if(ulMatchedId == EBML_TRACK_NAME_ID)
          {
            hTempTrackEntryInfo.pName = pMemory;
          }
          if(pMemory && bAppendZero)
          {
            pMemory[ullSize] = '\0';
          }
          if(ulMatchedId == EBML_TRACK_CODEC_ID)
          {
            /* MapMKAVCodecID will map the codec ID from m_pDataBuffer to
            mkav_media_codec_type */
            MapMKAVCodecID(m_pDataBuffer + ulIndex, (uint8)ullSize,
                           &hTempTrackEntryInfo);
            if(pMemory)
            {
              MM_Free(pMemory);
              pMemory = NULL;
            }
            ulMaxTargetSize = 0;
          }
        }
        if(pMemory)
        {
          if(ullSize <= (uint64)ulMaxTargetSize)
          {
            if(ulMatchedId == EBML_TRACK_TIMECODESCALE_ID)
            {
              //Special case for handling 32/64 bit floating point values.
              if(ullSize <= sizeof(float))
              {
                float floatval = 0.0;
                memcpy(&floatval, m_pDataBuffer + ulIndex, (size_t)ullSize);
                MKV_REVERSE_ENDIAN((uint8*)&floatval, (uint8)ullSize);
                hTempTrackEntryInfo.TrackTimeCodeScale = floatval;
              }//if(ullSize <= sizeof(float))
            }//if(ulMatchedId == EBML_TRACK_TIMECODESCALE_ID)
            else
            {
              //! If enough data is available, then copy from the local cache
              if (ullSize <= (ulDataRead - ulIndex))
              {
                memcpy(pMemory,m_pDataBuffer + ulIndex,(size_t)ullSize);
              }
              else
              {
                //! Update offset to the start of the data
                ullOffset  += ulIndex;
                ulDataRead = GetDataFromSource(ullOffset, (uint32)ullSize,
                                               pMemory,   (uint32)ullSize);
                if(ulDataRead < (uint32)ullSize)
                {
                  status  = MKAV_API_FAIL;
                  noerror = false;
                  break;
                }
                else
                {
                  ulDataRead = 0;
                  ulIndex    = 0;
                }
              }
              if(!bNeedAllocation)
              {
                MKV_REVERSE_ENDIAN((uint8*)pMemory,(size_t)ullSize);
              }
            }
          }
          else
          {
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
            "ParseTrackEntryElement failed. ullSize %llu > ulMaxTargetSize %lu",
              ullSize, ulMaxTargetSize);
          }
        }//if(pMemory)
      }//if(!bMaster)
      else
      {
        if(ulMatchedId == EBML_TRACK_VIDEO_ID)
        {
          status = ParseVideoInfo(m_pDataBuffer + ulIndex, (uint32)ullSize,
                                  &hTempVideoInfo);
          if(MKAV_API_SUCCESS != status)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                         "ParseVideoInfo failed @ offset %llu", ullOffset);
            noerror = false;
          }
        }
        else if(ulMatchedId == EBML_TRACK_AUDIO_ID)
        {
          status = ParseAudioInfo(m_pDataBuffer + ulIndex, (uint32)ullSize,
                                  &hTempAudioInfo);
          if(MKAV_API_SUCCESS != status)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                         "ParseAudioInfo failed @ offset %llu", ullOffset);
            noerror = false;
          }
        }
        else if(EBML_TRACK_CONTENTS_ENCODING_ID == ulMatchedId)
        {
          status = ParseContentEncodeInfo(m_pDataBuffer + ulIndex,
                                         (uint32)ullSize,
                                         &hTempContentInfo);
          if(MKAV_API_SUCCESS != status)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "ParseContentEncodeInfo failed @ offset %llu", ullOffset);
            noerror = false;
          }
        }
      }
    }//if(bok)
    else
    {
      //we may not recognized any valid chunk after this as ID size is unknown.
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "parseTrackEntryElement encountered unknown ID @ offset %llu",
                 ullOffset);
    }
    ulIndex += (uint32)ullSize;
  }//while( (noffset < nEndOffset)&&(noerror) )

  if(noerror)
  {
    int index = m_nstreams;
    m_nTrackEntry++;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "ParseTrackEntryElement m_nTrackEntry %lu", m_nTrackEntry);
    status = MKAV_API_SUCCESS;
    if(m_nstreams == 0)
    {
      /* This is first track entry element, allocate the memory for storing
         track information */
      m_nstreams++;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
              "ParseTrackEntryElement Allocating m_pTrackEntry m_nstreams %d",
              m_nstreams);
      m_pTrackEntry = (mkav_track_entry_info*)
                      MM_Malloc(sizeof(mkav_track_entry_info) * m_nstreams);
      if(m_pTrackEntry)
      {
        memset(m_pTrackEntry,0,sizeof(mkav_track_entry_info)*m_nstreams);
        memcpy(m_pTrackEntry,&hTempTrackEntryInfo,sizeof(hTempTrackEntryInfo));
        if(hTempTrackEntryInfo.TrackType == MKAV_TRACK_TYPE_VIDEO)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "ParseTrackEntryElement storing video information");
          m_nVideoStreams++;
          m_pTrackEntry->pVideoInfo = (mkav_video_info*)
                                      MM_Malloc(sizeof(mkav_video_info));
          if(m_pTrackEntry->pVideoInfo)
          {
            memcpy(m_pTrackEntry->pVideoInfo, &hTempVideoInfo,
                   sizeof(hTempVideoInfo));
            #ifndef FEATURE_DIVXHD_PLUS
            IsDivxEnabled();
            #endif
          }
        }//if(hTempTrackEntryInfo.TrackType == MKAV_TRACK_TYPE_VIDEO)
        else if(hTempTrackEntryInfo.TrackType == MKAV_TRACK_TYPE_AUDIO)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "ParseTrackEntryElement storing audio information");
          m_nAudioStreams++;
          m_pTrackEntry->pAudioInfo = (mkav_audio_info*)
                                      MM_Malloc(sizeof(mkav_audio_info));
          if(m_pTrackEntry->pAudioInfo)
          {
            memcpy(m_pTrackEntry->pAudioInfo, &hTempAudioInfo,
                   sizeof(hTempAudioInfo));
          }
        }//else if(hTempTrackEntryInfo.TrackType == MKAV_TRACK_TYPE_AUDIO)
      }//if(m_pTrackEntry)
    }//if(m_nstreams == 0)
    else
    {
      //re allocate the memory for storing track information
      m_nstreams++;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
            "ParseTrackEntryElement ReAllocating m_pTrackEntry m_nstreams %d",
            m_nstreams);
      mkav_track_entry_info* pInfo = (mkav_track_entry_info*)
                        MM_Realloc(m_pTrackEntry,
                                   sizeof(mkav_track_entry_info) * m_nstreams);
      if(pInfo)
      {
        m_pTrackEntry = pInfo;
        memset(m_pTrackEntry+index,0,sizeof(mkav_track_entry_info));
        memcpy(m_pTrackEntry+index, &hTempTrackEntryInfo,
               sizeof(hTempTrackEntryInfo));
        if(hTempTrackEntryInfo.TrackType == MKAV_TRACK_TYPE_VIDEO)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "ParseTrackEntryElement storing video information");
          m_nVideoStreams++;
          (m_pTrackEntry+index)->pVideoInfo = (mkav_video_info*)
                                          MM_Malloc(sizeof(mkav_video_info));
          if((m_pTrackEntry+index)->pVideoInfo)
          {
            memcpy((m_pTrackEntry+index)->pVideoInfo, &hTempVideoInfo,
                   sizeof(hTempVideoInfo));
            #ifndef FEATURE_DIVXHD_PLUS
            IsDivxEnabled();
            #endif
          }
        }//if(hTempTrackEntryInfo.TrackType == MKAV_TRACK_TYPE_VIDEO)
        else if(hTempTrackEntryInfo.TrackType == MKAV_TRACK_TYPE_AUDIO)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "ParseTrackEntryElement storing audio information");
          (m_pTrackEntry+index)->pAudioInfo = (mkav_audio_info*)
                                            MM_Malloc(sizeof(mkav_audio_info));
          m_nAudioStreams++;
          if((m_pTrackEntry+index)->pAudioInfo)
          {
            memcpy((m_pTrackEntry+index)->pAudioInfo, &hTempAudioInfo,
                   sizeof(hTempAudioInfo));
            if(!hTempTrackEntryInfo.DefaultDuration)
            {
              hTempTrackEntryInfo.DefaultDuration = CalcAudioFrameDuration(
                                              (m_pTrackEntry + index),
                                              hTempTrackEntryInfo.CodecType);
            }
          }
        }//else if(hTempTrackEntryInfo.TrackType == MKAV_TRACK_TYPE_AUDIO)
      }//if(m_pTrackEntry)
    }//end of else of if(m_nstreams == 0)

    if((UNKNOWN_ENCODE_TYPE != hTempContentInfo.eEncodeType) &&
       (true == noerror) && (m_pTrackEntry))
    {
      uint32 ulIndex = m_nstreams - 1;
      (m_pTrackEntry + ulIndex)->pEncodeInfo = (mkav_encode_info*)
                                           MM_Malloc(sizeof(mkav_encode_info));
      if((m_pTrackEntry + ulIndex)->pEncodeInfo)
      {
        memcpy((m_pTrackEntry + ulIndex)->pEncodeInfo, &hTempContentInfo,
               sizeof(hTempContentInfo));
      }
      else
      {
        (m_pTrackEntry + ulIndex)->pAudioInfo = NULL;
        (m_pTrackEntry + ulIndex)->pVideoInfo = NULL;
        (m_pTrackEntry+index)->CodecType = MKAV_UNKNOWN_CODEC;
        (m_pTrackEntry+index)->TrackType = MKAV_TRACK_TYPE_UNKNOWN;
        noerror = false;
        status  = MKAV_API_OUT_OF_MEMORY;
      }
    }
  }//if(noerror)

  /* Memory allocated for different structures in Track Entry, Video, Audio and
     Content encode structures will be copied into Class Structures. */
  if(!noerror)
  {
    if(hTempTrackEntryInfo.pLanguage)
      MM_Free(hTempTrackEntryInfo.pLanguage);
    if(hTempTrackEntryInfo.pCodecName)
      MM_Free(hTempTrackEntryInfo.pCodecName);
    if(hTempTrackEntryInfo.pCodecPvt)
      MM_Free(hTempTrackEntryInfo.pCodecPvt);
    if(hTempTrackEntryInfo.pName)
      MM_Free(hTempTrackEntryInfo.pName);
    if(hTempVideoInfo.pCodecHdr)
      MM_Free(hTempVideoInfo.pCodecHdr);
    if(hTempTrackEntryInfo.pAudioInfo)
      MM_Free(hTempTrackEntryInfo.pAudioInfo);
    if(hTempTrackEntryInfo.pVideoInfo)
      MM_Free(hTempTrackEntryInfo.pVideoInfo);
    if(hTempTrackEntryInfo.pEncodeInfo)
      MM_Free(hTempTrackEntryInfo.pEncodeInfo);
    if(hTempContentInfo.pCompSettings)
      MM_Free(hTempContentInfo.pCompSettings);
  }
  return status;
}

#ifndef FEATURE_DIVXHD_PLUS
/* ============================================================================
  @brief  Function to check the tools used to generate content.

  @details    This function is used to check whether DivxPlus tool has been
              used to generate test content or not.
              If this tool is used and customer do not have Certification then
              the content will be marked as unsupported.

  @param[in]  None.

  @return     None.
  @note       None.
============================================================================ */
void MKAVParser::IsDivxEnabled()
{
  //! Check the recently added entry codec type
  uint32 ulIndex = m_nstreams - 1;
  mkav_track_entry_info *pTrackInfo = m_pTrackEntry + ulIndex;
  if ((MKAV_TRACK_TYPE_VIDEO == pTrackInfo->TrackType) &&
      (MKAV_AVC1_VIDEO_CODEC == pTrackInfo->CodecType ||
       MKAV_HEVC_VIDEO_CODEC == pTrackInfo->CodecType))
  {
    bool bIsDivxPlus = false;
    char *pStr       = NULL;
    if (m_pSegmentInfo->pMuxingApp)
    {
      pStr = (char*)m_pSegmentInfo->pMuxingApp;
      ConvertToUpperCase(m_pSegmentInfo->pMuxingApp, strlen(pStr));
      if (!strncmp(pStr, DIVX_MUXING_APP, strlen(DIVX_MUXING_APP)))
      {
        bIsDivxPlus = true;
      }
    }
    if ((m_pSegmentInfo->pWritingApp) && (!bIsDivxPlus))
    {
      pStr = (char*)m_pSegmentInfo->pWritingApp;
      ConvertToUpperCase(m_pSegmentInfo->pWritingApp, strlen(pStr));
      if (!strncmp(pStr, DIVX_WRITING_APP, strlen(DIVX_WRITING_APP)))
      {
        bIsDivxPlus = true;
      }
    }
    if ((pTrackInfo->pCodecName) && (!bIsDivxPlus))
    {
      pStr = (char*)pTrackInfo->pCodecName;
      ConvertToUpperCase(pTrackInfo->pCodecName, strlen(pStr));
      if (!strncmp(pStr, DIVX_PLUS, strlen(DIVX_PLUS)))
      {
        bIsDivxPlus = true;
      }
    }
    if (bIsDivxPlus)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
        "ParseTrackEntryElement AVC/HEVC codec support disabled for Divx+");
      pTrackInfo->CodecType = MKAV_UNKNOWN_CODEC;
    }
  }
  return;
}
#endif
/* ============================================================================
  @brief  MapFileOffsetToCluster

  @details    Maps given file offset to cluster number.

  @param[in]      ullOffset      Start Offset from which we need to read data.
  @param[in/out]  pulClusterNo   Cluster Number.
  @param[in/out]  pucHdrSize     Cluster Header size.

  @return     MKAV_API_SUCCESS indicating parsing done successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
bool MKAVParser::MapFileOffsetToCluster(uint64  ullOffset,
                                        uint32* pulClusterNo,
                                        uint8*  pucHdrSize)
{
  bool   bRet    = false;
  uint32 ulCount = 0;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"MapFileOffsetToCluster noffset %llu",
               ullOffset);
  #endif
  if(m_pAllClustersInfo && pulClusterNo && pucHdrSize)
  {
    uint64 ullClusterOffset = 0;
    uint64 ullClusterSize   = 0;
    uint8  ucHdrsize        = 0;
    for(ulCount = 0; ulCount < m_pAllClustersInfo->nClusters;ulCount++)
    {
      ullClusterOffset = m_pAllClustersInfo->pClusterInfo[ulCount].nOffset;
      ullClusterSize   = m_pAllClustersInfo->pClusterInfo[ulCount].nSize;
      ucHdrsize        = m_pAllClustersInfo->pClusterInfo[ulCount].nHdrSize;
      if( ( (ullOffset + ucHdrsize) >= ullClusterOffset)&&
            ((ullClusterOffset + ullClusterSize) > (ullOffset + ucHdrsize) ) )
      {
        *pulClusterNo = ulCount;
        *pucHdrSize   = ucHdrsize;
        bRet          = true;
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                     "MapFileOffsetToCluster noffset %llu Cluster# %lu",
                     ullOffset, ulCount);
        break;
      }
    }
    if (false == bRet)
    {
      MKAV_API_STATUS retError =  MKAV_API_FAIL;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "MapFileOffsetToCluster does not find desired cluster");
      ullClusterOffset += ullClusterSize;
      do
      {
        retError = ParseNextClusterHeader(ullClusterOffset, ullClusterSize);
        if(MKAV_API_SUCCESS == retError)
        {
          ucHdrsize = m_pAllClustersInfo->pClusterInfo[ulCount].nHdrSize;
          if( ((ullOffset + ucHdrsize) >= ullClusterOffset)&&
              ((ullClusterOffset + ullClusterSize) > (ullOffset + ucHdrsize)))
          {
            *pulClusterNo = ulCount;
            *pucHdrSize   = ucHdrsize;
            bRet          = true;
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "MapFileOffsetToCluster noffset %llu Cluster# %lu",
                         ullClusterOffset, ulCount);
            break;
          }
          ulCount++;
        }
        else
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                  "MapFileOffsetToCluster failed at noffset %llu retError %d",
                  ullClusterOffset, retError);
          break;
        }
        ullClusterOffset += ullClusterSize;
      } while (false == bRet);
    }
  }
  return bRet;
}

/* =============================================================================
FUNCTION:
 MKAVParser::GetDataFromSource

DESCRIPTION:
Checks if data is available before attempting to read from it.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
Number of bytes read

SIDE EFFECTS:
  None.
=============================================================================*/
uint32 MKAVParser::GetDataFromSource(uint64 ullOffset, uint32 ulBytesNeed,
                                     uint8* pBuff,     uint32 ulBufMaxSize)
{
  uint32 ulBytesRead = 0;
  void* pUserData = m_pUserData;
  if(m_bEndOfData && m_nFileSize <= ullOffset)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "GetDataFromSource reached EOF");
    m_eParserPrvState = m_eParserState;
    m_eParserState    = MKAVPARSER_EOF;
  }
  else if(!m_bHttpPlay || m_bEndOfData)
  {
    ulBytesRead = MKAVFileCallbakGetData(ullOffset, ulBytesNeed, pBuff,
                                         ulBufMaxSize, pUserData);
    if(!ulBytesRead)
    {
      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_FATAL,
        "GetDataFromSource failed. noffset %llu nbytesneed# %lu FileSize %llu",
                   ullOffset, ulBytesNeed, m_nFileSize);
      m_eParserPrvState = m_eParserState;
      m_eParserState    = MKAVPARSER_READ_ERROR;
    }
  }
  else
  {
    //Check if sufficient data is available to read
    uint64 ullAvailoffset = 0;
    bool bret = MKAVCheckAvailableData(&ullAvailoffset, &m_bEndOfData,
                                       pUserData);
    if (m_bEndOfData)
    {
      m_nFileSize = ullAvailoffset;
    }

    /* In Android, system expects to request data even if it is not available.
       Whenever we request data which is not available, downloader will block
       read request and returns it after downloading the data from new offset.
       Sometimes it may result in delay in read if previous request and new
       offset required are very apart. */
#ifdef _ANDROID_
    ullAvailoffset = ullOffset + ulBytesNeed + 1;
#endif
    if(bret)
    {
      if((ullOffset + ulBytesNeed) <= ullAvailoffset)
      {
        ulBytesRead = MKAVFileCallbakGetData(ullOffset, ulBytesNeed, pBuff,
                                             ulBufMaxSize, pUserData);
        if(!ulBytesRead)
        {
          MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_FATAL,
          "GetDataFromSource failed. offset %llu bytesneed# %lu FileSize %llu",
                       ullOffset, ulBytesNeed, m_nFileSize);
          m_eParserPrvState = m_eParserState;
          m_eParserState    = MKAVPARSER_READ_ERROR;
        }
      }
      else
      {
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
          "DATA UNDER-RUN noffset %llu nbytesneed# %lu navailoffset %llu",
                     ullOffset, ulBytesNeed, ullAvailoffset);
        m_eParserPrvState = m_eParserState;
        m_eParserState    = MKAVPARSER_DATA_UNDERRUN;
        if (m_bEndOfData)
        {
          m_eParserState = MKAVPARSER_EOF;
        }

      }
    }
    else
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                "MKAVCheckAvailableData failed... ullOffset %llu ", ullOffset);
      m_eParserPrvState = m_eParserState;
      m_eParserState    = MKAVPARSER_READ_ERROR;
    }
  }
  return ulBytesRead;
}

/* =============================================================================
FUNCTION:
 MKAVParser::MapMKAVCodecID

DESCRIPTION:
Maps the codecID to codec type

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
None

SIDE EFFECTS:
  None.
=============================================================================*/
void MKAVParser::MapMKAVCodecID(uint8* buff,uint8 ndatasize,mkav_track_entry_info* pTrackEntryInfo)
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"MapMKAVCodecID ndatasize %u",ndatasize);
  if(buff && pTrackEntryInfo && ndatasize)
  {
    if(memcmp(buff,MKAV_AVC1_VIDEO,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_AVC1_VIDEO_CODEC;
    }
    else if(memcmp(buff,MKAV_MPEG4_ISO_SIMPLE_PROFILE,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MPEG4_ISO_SIMPLE_PROFILE_CODEC;
    }
    else if(memcmp(buff,MKAV_MPEG4_ISO_ADVANCE_SIMPLE_PROFILE,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MPEG4_ISO_ADVANCE_SIMPLE_PROFILE_CODEC;
    }
    else if(memcmp(buff,MKAV_MPEG4_ISO_ADVANCE_PROFILE,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MPEG4_ISO_ADVANCE_PROFILE_CODEC;
    }
    else if(memcmp(buff,MKAV_MPEG2_VIDEO,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MPEG2_VIDEO_CODEC;
    }
    else if(memcmp(buff,MKAV_MPEG1_VIDEO,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MPEG1_VIDEO_CODEC;
    }
    else if(memcmp(buff,MKAV_MSFT_CODEC_MGR,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MSFT_MGR_CODEC;
    }
    else if(memcmp(buff,MKAV_VP8_VIDEO_CODEC,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_VP8_VIDEO;
    }
    else if(memcmp(buff,MKAV_VP9_VIDEO_CODEC,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_VP9_VIDEO;
    }
    else if (memcmp(buff, MKAV_HEVC_VIDEO, ndatasize) == 0)
    {
      pTrackEntryInfo->CodecType = MKAV_HEVC_VIDEO_CODEC;
    }
    else if(
       (memcmp(buff,MKAV_AAC_MPEG2_MAIN_PROFILE,ndatasize)==0)      ||
       (memcmp(buff,MKAV_AAC_MPEG2_LOW_COMPLEXITY,ndatasize)==0)    ||
       (memcmp(buff,MKAV_AAC_MPEG2_LOW_COMPLEXITY_SBR,ndatasize)==0)||
       (memcmp(buff,MKAV_AAC_MPEG2_SSR,ndatasize)==0)               ||
       (memcmp(buff,MKAV_AAC_MPEG4_MAIN_PROFILE,ndatasize)==0)      ||
       (memcmp(buff,MKAV_AAC_MPEG4_LOW_COMPLEXITY,ndatasize)==0)    ||
       (memcmp(buff,MKAV_AAC_MPEG4_LOW_COMPLEXITY_SBR,ndatasize)==0)||
       (memcmp(buff,MKAV_AAC_MPEG4_SSR,ndatasize)==0)               ||
       (memcmp(buff,MKAV_AAC_MPEG4_LTP,ndatasize)==0) )
    {
      pTrackEntryInfo->CodecType = MKAV_AAC_AUDIO;
    }
    else if(memcmp(buff,MKAV_AC3_AUDIO,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_AC3_AUDIO_CODEC;
    }
    else if(memcmp(buff,MKAV_DOLBY_AC3,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_DOLBY_AC3_CODEC;
    }
    else if(memcmp(buff,MKAV_EAC3_AUDIO,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_EAC3_AUDIO_CODEC;
    }
    else if (memcmp(buff,MKAV_DTS_AUDIO,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_DTS_AUDIO_CODEC;
    }
    else if(memcmp(buff,MKAV_MPEG_AUDIO_L3,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MP3_AUDIO;
    }
    else if(memcmp(buff,MKAV_MPEG_AUDIO_L2,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MP2_AUDIO;
    }
    else if(memcmp(buff,MKAV_MPEG_AUDIO_L1,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MP1_AUDIO;
    }
    else if(memcmp(buff,MKAV_VORBIS_AUDIO,ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_VORBIS_AUDIO_CODEC;
      if(m_bPlayAudio)
      {
        m_nCodecHdrToSend = 1;
      }
    }
    else if(memcmp(buff, MKAV_MSFT_AUDIO_CODEC_MGR, ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_MSFT_MGR_AUDIO_CODEC;
    }
    else if(memcmp(buff, MKAV_OPUS_AUDIO, ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_OPUS;
    }
    else if(memcmp(buff, MKAV_ST_UTF8, ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_UTF8;
    }
    else if(memcmp(buff, MKAV_ST_USF, ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_USF;
    }
    else if(memcmp(buff, MKAV_ST_ASS, ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_ASS;
    }
    else if(memcmp(buff, MKAV_ST_SSA, ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_SSA;
    }
    else if(memcmp(buff, MKAV_ST_VOBSUB, ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_VOBSUB;
    }
    else if(memcmp(buff, MKAV_ST_KARAOKE, ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_KARAOKE;
    }
    else if(memcmp(buff, MKAV_ST_BMP, ndatasize)==0)
    {
      pTrackEntryInfo->CodecType = MKAV_BMP;
    }
  }
}
/* =============================================================================
FUNCTION:
 MKAVParser::PrepareAVCCodecInfo

DESCRIPTION:
Parses the avc information to prepare the sps/pps by prefixing each param with the start code.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
None

SIDE EFFECTS:
  None.
=============================================================================*/
void MKAVParser::PrepareAVCCodecInfo(mkav_avc1_info* avcinfo)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"PrepareAVCCodecInfo");
  if(avcinfo && avcinfo->nOutData)
  {
    avcinfo->pOutData = (uint8*)MM_Malloc(avcinfo->nOutData);
    if(avcinfo->pOutData)
    {
      int nMaxSize = (int)avcinfo->nOutData;
      int outbuffoffset = 0;
      int nspdataoffset = 0;
      int nppdataoffset = 0;
      for(int i = 0; (i < avcinfo->nSP) && (avcinfo->pSPLengths) && (avcinfo->pSPData); i++)
      {
        if( (outbuffoffset+AVC1_START_CODE_SIZE) < nMaxSize)
        {
          memcpy(avcinfo->pOutData+outbuffoffset,AVC1_START_CODE,AVC1_START_CODE_SIZE);
          outbuffoffset+=AVC1_START_CODE_SIZE;
          int length = avcinfo->pSPLengths[i];
          if( ((outbuffoffset + length) < nMaxSize) && (nspdataoffset < (int)avcinfo->nSPData) )
          {
            memcpy(avcinfo->pOutData+outbuffoffset,avcinfo->pSPData+nspdataoffset,length);
            outbuffoffset+= length;
            nspdataoffset+= length;
          }
        }
      }
      for(int j = 0; (j < avcinfo->nPP) && (avcinfo->pPPLengths) && (avcinfo->pPPData); j++)
      {
        if( (outbuffoffset+AVC1_START_CODE_SIZE) < nMaxSize)
        {
          memcpy(avcinfo->pOutData+outbuffoffset,AVC1_START_CODE,AVC1_START_CODE_SIZE);
          outbuffoffset+=AVC1_START_CODE_SIZE;
          int length = avcinfo->pPPLengths[j];
          if( ((outbuffoffset + length) <= nMaxSize) && (nppdataoffset < (int)avcinfo->nPPData) )
          {
            memcpy(avcinfo->pOutData+outbuffoffset,avcinfo->pPPData+nppdataoffset,length);
            outbuffoffset+= length;
            nppdataoffset+= length;
          }
        }
      }
    }//if(avcinfo->pOutData)
  }//if(avcinfo && avcinfo->nOutData)
}

/*! ===========================================================================
@brief  Prepares codec config data for HEVC codec

@detail  Reads codec private data atom from hevc track and prepares codec
         config data in the form of sequence of NALs

@param[in/out]
   pucCodecPvt         : Codec private data pointer.
   ulCodecSize         : Private data size
   pulNALULenMinusOne  : Pointer to store NAL Unit Size
   pHEVCInfo           : Buffer to store HEVC Codec Config data

@return    Total size for codec config data
@note      None.
============================================================================ */
uint32 MKAVParser::PrepareHEVCCodecInfo(uint8*  pucCodecPvt,
                                        uint32  ulCodecSize,
                                        uint32* pulNALULenMinusOne,
                                        uint8*  pHEVCInfo)
{
  uint32 ulCodecConfigBufSize = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"PrepareHEVCCodecInfo");
  /* Codec private data for HEVC contains hvcC atom similar to mp4/3gp.
     hvcC atom structure is as follows:

     Number of bits -- Bit Counter  -- Description
     8              --  8           -- Config Version
     2              --  10          -- General Profile Space
     1              --  11          -- General Tier flag
     5              --  16          -- General Profile IDC
     32             --  48          -- Profile Compatibility Flag
     1              --  49          -- Gen Progressive Source Flag
     1              --  50          -- Gen Interlace Source Flag
     1              --  51          -- Gen Non-Packed Constraint Flag
     1              --  52          -- Gen Frame only Constraint Flag
     44             --  96          -- Reserved
     8              --  104         -- Gen Level IDC Flag
     4              --  108         -- Gen Progressive Source Flag
     12             --  120         -- Min Spatial Segmentation IDC
     6              --  126         -- Reserved value
     2              --  128         -- Parallelism type
     6              --  130         -- Reserved value
     2              --  136         -- Chroma format IDC
     5              --  141         -- Reserved value
     3              --  144         -- Bit Depth Luma Minus
     5              --  147         -- Reserved value
     3              --  152         -- Bit Depth Chroma Minus
     16             --  168         -- Reserved value
     2              --  170         -- Reserved value
     3              --  173         -- Maximum Sub Layers
     1              --  174         -- Temporal ID nesting flag
     2              --  176         -- Size NALU Minus One
     8              --  184         -- No. of parameter sets (num_param_sets)
     for (i=0;i<num_param_sets;i++) {
     1              --  ---         -- Array completeness flag
     1              --  ---         -- Reserved
     6              --  ---         -- NAL Unit Type
     16             --  ---         -- No. of NAL Units (nalu_unit_count)
     for (j=0;j<nalu_unit_count;j++) {
     16             --  ---         -- Size of each NALU (size)
     for(k=0;k<size;k++) {
     8              --  ---         -- NALU data (data[k]
        }
      }
    }

     Currently we need NALU size field and NALU data. So first 168 bits (21B)
     is not required. MKAV_HEVC_CODEC_RESERVED_DATA_SIZE is 21 Bytes.
  */
  if((pucCodecPvt) && (ulCodecSize > MKAV_HEVC_CODEC_RESERVED_DATA_SIZE) )
  {
    /* MKAV_HEVC_CODEC_RESERVED_DATA_SIZE are not required.
       Decoder needs SPS/PPS/VPS/SEI messages info only.
       These fields are available the fixed data size defined as macro.
    */
    uint32 ulBufIndex        = MKAV_HEVC_CODEC_RESERVED_DATA_SIZE;
    uint8  ucNumParamSets    = 0;
    uint16 usNALSets         = 0;
    uint32 ulcodecIndex      = 0;
    uint32 ulNALULenMinusOne = pucCodecPvt[ulBufIndex++] & 0x03;
    const uint32 ulSC        = 0x01000000;

    //! Update NALU Length Minus One value
    if (pulNALULenMinusOne)
    {
      *pulNALULenMinusOne = ulNALULenMinusOne;
    }

    ucNumParamSets = pucCodecPvt[ulBufIndex++];

    //! Ensure minimum 3  bytes of data is available
    for(int i = 0; (i < ucNumParamSets) && (ulBufIndex + 3 <= ulCodecSize); i++)
    {
      uint32 ulCount    = 0;
      uint16 usNALUSize = 0;
      //! Ignore this byte, this contains NALU Type.
      ulBufIndex++;
      //! Calculate total number of set
      memcpy(&usNALSets, pucCodecPvt + ulBufIndex, 2);
      MKV_REVERSE_ENDIAN((uint8*)&usNALSets, 2);
      ulBufIndex += 2;
      //! Ensure minimum two bytes of data is available
      while((ulCount < usNALSets) && (ulBufIndex + 2 <= ulCodecSize))
      {
        //! Calculate NALU Size
        memcpy(&usNALUSize, pucCodecPvt + ulBufIndex, 2);
        MKV_REVERSE_ENDIAN((uint8*)&usNALUSize, 2);
        ulBufIndex += 2;

        ulCodecConfigBufSize += FOURCC_SIGNATURE_BYTES + usNALUSize;
        //! If buffer is allocated already, then copy data
        if (pHEVCInfo)
        {
          if((ulBufIndex + usNALUSize) <= ulCodecSize)
          {
            //! Copy start code
            memcpy(pHEVCInfo + ulcodecIndex, &ulSC, sizeof(uint32));
            ulcodecIndex += FOURCC_SIGNATURE_BYTES;

            memcpy(pHEVCInfo + ulcodecIndex,
                   pucCodecPvt + ulBufIndex, usNALUSize);
            ulcodecIndex += usNALUSize;
          }
          else
          {
            break;
          }
        }
        ulBufIndex   += usNALUSize;
        //! Update loop variable
        ulCount++;
      } //while((ulCount < usNalNALSets) && (ulBufIndex + 2 < ulCodecSize))
    }//main for loop
  }//if(pucCodecPvt && ...)
  return ulCodecConfigBufSize;
}

/* ======================================================================
FUNCTION:
  GetClipMetaData

DESCRIPTION:
  Used to extract metadata

INPUT/OUTPUT PARAMETERS:
  ulMetaDataIndex       MetaData Index value

RETURN VALUE:
 tag_info_type ptr value if successful
 else returns NULL pointer

SIDE EFFECTS:
  None.
======================================================================*/
tag_info_type* MKAVParser::GetClipMetaData(uint32 ulMetaDataIndex)
{
  tag_info_type *pTagInfo = NULL;

  uint32 ulIndex = 0;
  while(ulIndex < m_nTagInfoCount)
  {
    pTagInfo = TagInfoArray[ulIndex++];
    if(!memcmp(pTagInfo->pTagName, ucMETADATASTR[ulMetaDataIndex],
               pTagInfo->ulTagNameLen) )
    {
      break;
    }
  }
  if(ulIndex == m_nTagInfoCount)
  {
    pTagInfo = NULL;
  }
  return pTagInfo;
}

/* ============================================================================
  @brief  Function to provide Metadata strings available in SegmentInfo.

  @details    This function is used to provide metadata available in SegmentIno
              and Tracks elements.

  @param[in/out]  pucDataBuf    Buffer to read metadata.
  @param[in/out]  pulBufSize    Size of the buffer.
  @param[in]      ienumData     Metadata type.

  @return     PARSER_ErrorNone indicating metadata read successfully.
              Else, it will report corresponding error.
  @note       BufferSize should be more than metadata size value.
============================================================================ */
PARSER_ERRORTYPE MKAVParser::GetSegmentInfo(wchar_t* pucDataBuf,
                                            uint32*  pulDatabufLen,
                                            FileSourceMetaDataType ienumData)
{
  PARSER_ERRORTYPE eRetStatus = PARSER_ErrorDefault;
  uint8* pStr = NULL;

  if ((m_pSegmentInfo) && (pulDatabufLen) )
  {
    if(FILE_SOURCE_MD_MUXING_APP  == ienumData)
    {
      pStr = m_pSegmentInfo->pMuxingApp;
    }
    else if(FILE_SOURCE_MD_WRITING_APP  == ienumData)
    {
      pStr = m_pSegmentInfo->pWritingApp;
    }
    else if(FILE_SOURCE_MD_CODEC_NAME  == ienumData)
    {
      uint32 ulIndex = 0;
      mkav_track_entry_info *pTrackInfo = m_pTrackEntry + ulIndex;
      for(ulIndex = 0; (ulIndex < m_nstreams) && pTrackInfo; ulIndex++)
      {
        pTrackInfo = m_pTrackEntry + ulIndex;
        if (MKAV_TRACK_TYPE_VIDEO == pTrackInfo->TrackType)
        {
          pStr = pTrackInfo->pCodecName;
          break;
        }
      }
    }

    if (pStr)
    {
      uint32 ulStrLen = (uint32)strlen((const char*)pStr);
      if( (pucDataBuf) && (*pulDatabufLen >= ulStrLen) )
      {
        ConvertToUpperCase(pStr, ulStrLen);
        memset(pucDataBuf, 0, *pulDatabufLen);
        memcpy(pucDataBuf, pStr,
               FILESOURCE_MIN(*pulDatabufLen - 1, ulStrLen));
      }
      else
      {
        *pulDatabufLen = ulStrLen;
      }
      eRetStatus = PARSER_ErrorNone;
    }
  }
  return eRetStatus;
}

/* =============================================================================
FUNCTION:
 MKAVParser::GetTrackWholeIDList

DESCRIPTION:
Returns trackId list for all the tracks in given clip.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
uint32 MKAVParser::GetTrackWholeIDList(uint32* idList)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetTrackWholeIDList");
  if(!idList)
  {
    return 0;
  }
  for(uint8 j = 0; j < m_nstreams; j++)
  {
    *(idList+j) = (uint32)(m_pTrackEntry[j].TrackNo);
  }
  return m_nstreams;
}
/* =============================================================================
FUNCTION:
 MKAVParser::GetTrackCodecType

DESCRIPTION:
Returns track codec type for given track id

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Success if track id is valid otherwise, returns invalid parameter.
SIDE EFFECTS:
  None.
=============================================================================*/
mkav_media_codec_type MKAVParser::GetTrackCodecType(uint32 id)
{
  mkav_media_codec_type codec = MKAV_UNKNOWN_CODEC;
#ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetTrackCodecType");
#endif
  for(int i = 0; (i< m_nstreams) && m_pTrackEntry; i++)
  {
    if(m_pTrackEntry[i].TrackNo == id)
    {
      codec = m_pTrackEntry[i].CodecType;
      break;
    }
  }
  return codec;
}
/* =============================================================================
FUNCTION:
 MKAVParser::GetTrackType

DESCRIPTION:
Returns track type for given track id

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Success if track id is valid otherwise, returns invalid parameter.
SIDE EFFECTS:
  None.
=============================================================================*/
mkav_track_type  MKAVParser::GetTrackType(uint32 id)
{
  mkav_track_type trktype = MKAV_TRACK_TYPE_UNKNOWN;
#ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetTrackType");
#endif
  for(int i = 0; (i< m_nstreams) && m_pTrackEntry; i++)
  {
    if(m_pTrackEntry[i].TrackNo == id)
    {
      trktype = m_pTrackEntry[i].TrackType;
      break;
    }
  }
  return trktype;
}
/* =============================================================================
FUNCTION:
 MKAVParser::GetTrackBufferSize

DESCRIPTION:
Returns the buffer size needed for given track id

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 returns buffer size needed.
SIDE EFFECTS:
  None.
=============================================================================*/
uint32  MKAVParser::GetTrackBufferSize(uint32 id)
{
  uint32 buffsize = 0;
  for(int i = 0; (i< m_nstreams) && m_pTrackEntry; i++)
  {
    if( (m_pTrackEntry[i].TrackNo == id)&&
        (m_pTrackEntry[i].TrackType == MKAV_TRACK_TYPE_VIDEO) )
    {
      if(m_pTrackEntry[i].pVideoInfo)
      {
        if(m_pTrackEntry[i].pVideoInfo->DisplayHeight &&
           m_pTrackEntry[i].pVideoInfo->DisplayWidth)
        {
          //Reducing parser input buffer size by 2 as i/p buffer size becoming
          //greater than then YUV frame size for 720p/1080p content
          buffsize = (uint32)(m_pTrackEntry[i].pVideoInfo->DisplayHeight *
                              m_pTrackEntry[i].pVideoInfo->DisplayWidth );
        }
        else
        {
          //Reducing parser input buffer size by 2 as i/p buffer size becoming
          //greater than then YUV frame size for 720p/1080p content
          buffsize = (uint32)(m_pTrackEntry[i].pVideoInfo->PixelHeight *
                              m_pTrackEntry[i].pVideoInfo->PixelWidth );
        }
        //! We need 1.5 times of H*W as max buffer size. To make calculation
        //! simpler, following right shift operation is used.
        buffsize += (buffsize >> 2);
        //! Other than HEVC, we do not need big buffers
        //! 75% of (height * width) as buffer is would be sufficient
        if (MKAV_HEVC_VIDEO_CODEC != m_pTrackEntry[i].CodecType)
        {
          buffsize /= 2;
        }
      }
      break;
    }
    else if( (m_pTrackEntry[i].TrackNo == id)&&
             (m_pTrackEntry[i].TrackType == MKAV_TRACK_TYPE_AUDIO) )
    {
      buffsize = DEF_AUDIO_BUFF_SIZE;
      break;
    }
  }
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
               "GetTrackBufferSize returning buffsize %lu for id %lu",
               buffsize, id);
  return buffsize;
}
/* =============================================================================
FUNCTION:
  MKAVParser::GetClipDurationInMsec

DESCRIPTION:
 Returns clip duration from the current parsed data.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
uint64 MKAVParser::GetClipDurationInMsec()
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"GetClipDurationInMsec %llu",
               m_nClipDuration);
  return m_nClipDuration;
}

/* ============================================================================
  @brief  Reads one Sample/Frame at a time.

  @details    This function is used to read one media sample/frame at a time
              from Cluster. Parser will support both bitstream and frame modes.
              In frame mode, it will give only one frame, where as in
              bit Stream mode, it give one Access Unit (set of frames).

  @param[in]      trackId       Track Id number.
  @param[in/out]  pucDataBuf    Buffer to read sample.
  @param[in]      ulBufSize     Size of the input buffer.
  @param[in/out]  plBytesRead   Number of bytes read into buffer.
  @param[in/out]  psampleInfo   Structure to store sample properties.

  @return     MKAV_API_SUCCESS indicating sample read successfully.
              Else, it will report corresponding error.
  @note       BufferSize should be more than maximum frame size value.
============================================================================ */
MKAV_API_STATUS MKAVParser::GetCurrentSample(uint32 trackId,uint8* pucDataBuf,
                                         uint32 ulBufSize, int32* plBytesRead,
                                         mkav_stream_sample_info* psampleInfo)
{
  MKAV_API_STATUS retError    = MKAV_API_INVALID_PARAM;
  mkav_media_codec_type codec = GetTrackCodecType(trackId);
  bool bok                    = true;
  bool bIsFrameAvailable      = false;
  bool bIsClusterStart        = false;
  uint32 ulTempBufSize        = ulBufSize;
  //Validate input params
  if((NULL == plBytesRead) || (NULL == psampleInfo) )
  {
    return retError;
  }

  /* Buffer may not be provided during seek call. This is to avoid the data
     processing operation during seek call. */
  if( (m_nCodecHdrToSend)                              &&
      ((m_nCodecHdrToSend -1)< MKAV_VORBIS_CODEC_HDRS) &&
      (codec != MKAV_UNKNOWN_CODEC)                    &&
      (m_nCodecHdrSizes[m_nCodecHdrToSend-1] ) &&
      (pucDataBuf && ulBufSize))
  {
    uint32 ulCodecDataSize = GetCodecHeaderSize(trackId);
    if(ulBufSize >= ulCodecDataSize)
    {
      uint8* pcodec  = GetCodecHeader(trackId);
      if(pcodec)
      {
        uint32 nbytestoskip = 0;
        for(uint32 j = 0; j < (uint32)(m_nCodecHdrToSend-1) ;j++)
        {
          nbytestoskip += m_nCodecHdrSizes[j];
        }
        memcpy(pucDataBuf, pcodec + nbytestoskip,
               m_nCodecHdrSizes[m_nCodecHdrToSend - 1]);
        *plBytesRead = m_nCodecHdrSizes[m_nCodecHdrToSend - 1];
        m_nCodecHdrToSend++;
        retError = MKAV_API_SUCCESS;
        return retError;
      }
    }
    else
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
        "m_bNeedToSendCodecHdr nMaxBufSize %lu < nCodecHdrSize %lu",
        ulBufSize, ulCodecDataSize);
      *plBytesRead = ulCodecDataSize;
      retError = MKAV_API_INSUFFICIENT_BUFFER;
      return retError;
    }
  }
  if(!m_pCurrCluster)
  {
    m_pCurrCluster = (cluster_info*)MM_Malloc(sizeof(cluster_info));
    if(m_pCurrCluster)
    {
      memset(m_pCurrCluster, 0, sizeof(cluster_info));
    }
  }
  else if(pucDataBuf && ulBufSize)
  {
    /* Each block/simple block may contain more than one frame. If Parser is
       configured to provide one frame at a time, then ReadFrames will read
       one frame if possible. Parser will not proceed further and return with
       this function return status directly. If all the frames are extracted
       then flag will be marked as false. In that case, Parser will proceed to
       next block/simple block. */
    retError = ReadFrames(trackId, pucDataBuf, &ulBufSize,
                          psampleInfo, bIsFrameAvailable);
    if(true == bIsFrameAvailable)
    {
      *plBytesRead = ulBufSize;
      return retError;
    }
  }
  if(!m_pCurrCluster)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
              "GetCurrentSample failed to allocate memory for m_pCurrCluster");
    retError = MKAV_API_OUT_OF_MEMORY;
    return retError;
  }
  //! Update the timestamp  value in cluster using the class structure.
  else if((m_pAllClustersInfo) && (m_pAllClustersInfo->pClusterInfo))
  {
    m_pCurrCluster->nTimeCode =
      m_pAllClustersInfo->pClusterInfo[m_nCurrCluster].nTimeCode;
  }
#ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample m_nCurrOffset %llu",
               m_nCurrOffset);
#endif
  if(m_pAllClustersInfo && m_pAllClustersInfo->nClusters &&
     m_pAllClustersInfo->pClusterInfo && plBytesRead && psampleInfo)
  {
    *plBytesRead         = 0;
    uint32 nSimpleBlocks = 0;
    while( (*plBytesRead == 0) && (bok) && (m_pAllClustersInfo))
    {
      uint64 ullCurClusterEndOffset =
                    m_pAllClustersInfo->pClusterInfo[m_nCurrCluster].nSize +
                    m_pAllClustersInfo->pClusterInfo[m_nCurrCluster].nOffset;
      if( m_nCurrOffset >= ullCurClusterEndOffset)
      {
        bok             = false;
        bIsClusterStart = true;
      }
      if(bok)
      {
        blockgroup_info *pBlockGroup = NULL;
        blockinfo       *pBlockInfo  = NULL;
        do
        {
          bool bBlockFound = false;
          //Reset Error as success
          retError = MKAV_API_SUCCESS;
          /* This flag indicates new Cluster start. In that case Parser will
             not use previously parsed Block details to calc Offset value. */
          if(bIsClusterStart)
          {
            bBlockFound     = false;
            bIsClusterStart = false;
          }
          else if((!m_pCurrCluster->pSimpleBlocks) &&
                  (m_pCurrCluster->pBlockGroup))
          {
            pBlockGroup = m_pCurrCluster->pBlockGroup;
            int nCount  = pBlockGroup->nBlocksParsed;
            for(; nCount < (int)pBlockGroup->nBlockinfo; nCount++)
            {
              pBlockInfo = &pBlockGroup->pBlockInfo[nCount];
              if((pBlockGroup->pBlockInfo[nCount].nTrackNumber == trackId))
              {
                pBlockGroup->nBlocksParsed = nCount + 1;
                bBlockFound = true;
                break;
              }
            }
            if (nCount == (int)pBlockGroup->nBlockinfo && !bBlockFound)
            {
              m_nCurrOffset = pBlockGroup->nBlockEndOffset;
              pBlockInfo    = NULL;
            }
          }
          else if((m_pCurrCluster->pSimpleBlocks) &&
                  (nSimpleBlocks < m_pCurrCluster->nSimpleBlocks))
          {
            bBlockFound = true;
            pBlockInfo  =
             &m_pCurrCluster->pSimpleBlocks[m_pCurrCluster->nSimpleBlocks - 1];
            nSimpleBlocks++;
            break;
          }
          else
          {
            nSimpleBlocks = 0;
          }
          if(bBlockFound == true)
          {
            break;
          }
          /* Go to next SimpleBlock/BlockGroup atom and extract metadata.*/
          uint64 ullCurrClusterSize =
                        m_pAllClustersInfo->pClusterInfo[m_nCurrCluster].nSize;

          /* If current offset is reached beyond current cluster end, break
             loop. Need to update other parameters before parsing next
             cluster. */
          if (m_nCurrOffset >= ullCurClusterEndOffset)
          {
            break;
          }
          retError = ParseClusterElement(m_nCurrOffset,
                                         ullCurrClusterSize,
                                         m_pCurrCluster);
        } while(MKAV_API_SUCCESS == retError);
        /* If Cluster element parsing is failed, then exit. */
        if (MKAV_API_SUCCESS != retError)
        {
          bok = false;
        }
        /* If CUE element is not present, then parser will create dynamically
           updated seek table. */
        if ((!m_pAllCuesInfo) && (m_SeekTableInfo.pSeekTableEntries) &&
            ((pBlockInfo) && (pBlockInfo->nFramesParsed == 0)))
        {
          UpdateSeekTable(pBlockGroup, pBlockInfo);
        }
        /* Update current offset to end of the block (if simple block) or to
           end of block group if all the blocks in block group are parsed. */
        if(pBlockInfo)
        {
          m_nCurrOffset = pBlockInfo->nDataOffset + pBlockInfo->nDataSize;
          if ((pBlockGroup) &&
              (pBlockGroup->nBlocksParsed == pBlockGroup->nBlockinfo))
          {
            m_nCurrOffset = pBlockGroup->nBlockEndOffset;
          }
        }
        /* Read frame data if buffer pointer is not NULL. */
        if((pBlockInfo) && (pBlockInfo->nTrackNumber == trackId) &&
           (pucDataBuf))
        {
          bIsClusterStart = false;
          //! 'ulTempBufSize' contains input buffer size value. Below function
          //! will updated buffer size value, overwrite with correct value.
          ulBufSize = ulTempBufSize;
          /* Read one or all the frames from block based on Parser config. */
          retError = ReadFrames(trackId, pucDataBuf, &ulBufSize,
                                psampleInfo, bIsFrameAvailable);
          if(true == bIsFrameAvailable)
          {
            //! If read is not successful, then do not update o/p bytes read
            if(MKAV_API_SUCCESS != retError)
              bok = false;
            else
              *plBytesRead = ulBufSize;
          }
        }//if(pBlockInfo->nTrackNumber == trackId && m_pCurrCluster)
        //! During seek Parser is interested only in sample properties.
        //! This is to avoid the extra overhead of reading media data.
        else if((pBlockInfo) && (pBlockInfo->nTrackNumber == trackId))
        {
          UpdateSampleProperties(trackId, 0/*FrameDur*/, pBlockInfo->nDataSize,
                                 psampleInfo, pBlockInfo, bIsFrameAvailable);
          break;
        }
      }
      else
      {
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
               "m_nCurrOffset %llu, m_nCurrCluster %llu, Total clusters %lu",
                m_nCurrOffset, m_nCurrCluster, m_pAllClustersInfo->nClusters);

        if((m_nCurrCluster+1) < m_pAllClustersInfo->nClusters)
        {
          m_nCurrOffset =
                  m_pAllClustersInfo->pClusterInfo[m_nCurrCluster + 1].nOffset;
          m_nCurrCluster++;
          MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
          "Updated m_nCurrOffset %llu, m_nCurrCluster %llu, Total clusters %lu",
              m_nCurrOffset, m_nCurrCluster, m_pAllClustersInfo->nClusters);
          bok = true;
        }
        else
        {
          /* When playing from istreamport, and underlying source is not file,
             Parser may not have parsed all the clusters.
             Need to locate cluster until EOF. */
          if( (m_bHttpPlay) && (m_nCurrOffset < m_nFileSize) )
          {
            uint64 ullClusterSize = 0;
            retError = ParseNextClusterHeader(m_nCurrOffset, ullClusterSize);
            if(MKAV_API_SUCCESS == retError)
            {
              bok = true;
              m_nCurrCluster++;
              m_nCurrOffset =
                 m_pAllClustersInfo->pClusterInfo[m_nCurrCluster].nOffset;
            }
          }
        }
        if(!bok)
        {
          bok = false;
          if(MKAV_API_DATA_UNDERRUN == retError)
          {
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                         "Data under-run for track %lu at cluster num %llu",
                         trackId, m_nCurrCluster);
          }
          else
          {
            retError = MKAV_API_EOF;
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                         "Consumed all samples, end of track %lu, retError %x",
                         trackId, retError);
          }
          *plBytesRead = 0;
        }
      }
    }//while( (*nBytesRead == 0) && (bok) )
  }
  return retError;
}

/* ======================================================================
FUNCTION:
  aviParser::randomAccessDenied

DESCRIPTION:
  gets if repositioning is allowed or not.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 If cues are present, it returns false.
 Else returns true.

SIDE EFFECTS:
  None.
======================================================================*/
uint8 MKAVParser::randomAccessDenied()
{
  uint8 bSeekDenied = true;
  if(m_pAllCuesInfo && m_pAllCuesInfo->nCuePoints)
  {
    bSeekDenied = false;
  }
  else if(m_SeekTableInfo.pSeekTableEntries)
  {
    bSeekDenied = false;
  }
  else
  {
    /* If Cue Element is not available in the given clip, Parser will create
       the dynamic seek table. */
    uint32 ulEntries = 0;
    /* Calculate total no. of entries that can be stored for SEEK_ENTRY_DELTA.
       If total number entries are more than MAX_SEEK_ENTRIES with 500ms delta,
       then limit total entries to "MAX_SEEK_ENTRIES" and edit the delta value
       accordingly. */
    m_SeekTableInfo.ulEntries = (uint32)(m_nClipDuration / SEEK_ENTRY_DELTA) + 1;
    m_SeekTableInfo.ulDelta   = SEEK_ENTRY_DELTA;
    if (m_SeekTableInfo.ulEntries > MAX_SEEK_ENTRIES)
    {
      m_SeekTableInfo.ulEntries = MAX_SEEK_ENTRIES;
      m_SeekTableInfo.ulDelta   = (uint32)(m_nClipDuration / MAX_SEEK_ENTRIES);
    }
    ulEntries = m_SeekTableInfo.ulEntries;
    m_SeekTableInfo.pSeekTableEntries = (seek_table_entry*)
                                        MM_Malloc(ulEntries *
                                                  sizeof(seek_table_entry));
    if (m_SeekTableInfo.pSeekTableEntries)
    {
      memset(m_SeekTableInfo.pSeekTableEntries, 0,
             ulEntries * sizeof(seek_table_entry));
      bSeekDenied = false;
    }
  }
  return bSeekDenied;
}

/*! ======================================================================
@brief  Repositions given track to specified time

@detail  Seeks given track in forward/backward direction to specified time

@param[in]
 trackid: Identifies the track to be repositioned.
 nReposTime:Target time to seek to
 nCurrPlayTime: Current playback time
 sample_info: Sample Info to be filled in if seek is successful
 canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
 nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.

@return    MKAVPARSER_SUCCESS if successful other wise returns MKAVPARSER_FAIL
@note      None.
========================================================================== */
MKAV_API_STATUS MKAVParser::Seek(uint32 ulTrackId, uint64 nReposTime,
                                 uint64 nCurrPlayTime,
                                 mkav_stream_sample_info* pSampleInfo,
                                 bool bForward, bool canSyncToNonKeyFrame,
                                 int nSyncFramesToSkip)
{
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "MKAVParser::Seek id %lu nReposTime %llu nCurrPlayTime %llu",
               ulTrackId, nReposTime, nCurrPlayTime);
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
   "MKAVParser::Seek bForward %d canSyncToNonKeyFrame %d nSyncFramesToSkip %d",
   bForward, canSyncToNonKeyFrame, nSyncFramesToSkip);

  MKAV_API_STATUS retError = MKAV_API_FAIL;
  mkav_media_codec_type eCodec = GetTrackCodecType(ulTrackId);
  bool bClosest = false;
#ifdef _ANDROID_
  bClosest = true;
#endif

  /* Set Parser state as Seeking. In Seek state, Parser will not calculate each
     frame size, if lacing is XING/EBML. This is to avoid extra overhead
     processing. */
  m_eParserState = MKAVPARSER_SEEKING;

  //make sure trackid is valid
  if((pSampleInfo) && (MKAV_UNKNOWN_CODEC != eCodec))
  {
    if (MKAV_OPUS == eCodec)
    {
      mkav_audio_info sAudio;
      memset(&sAudio, 0, sizeof(mkav_audio_info));
      GetAudioTrackProperties(ulTrackId, &sAudio);
      //! Seek preroll is stored in Nano-Sec units. Convert to milli-sec units
      uint64 ullSeekPreroll = sAudio.ullSeekPrerollDelay /
                             (MILLISEC_TIMESCALE_UNIT*MILLISEC_TIMESCALE_UNIT);
      if (nReposTime > ullSeekPreroll)
      {
        nReposTime -= ullSeekPreroll;
      }
      else
        nReposTime = 0;
    }//! if (MKAV_OPUS == eCodec)

    memset(pSampleInfo,0,sizeof(mkav_stream_sample_info));
    if(0 == nReposTime)
    {
      retError = SeekToZERO(pSampleInfo, ulTrackId);
    }//if(nReposTime == 0)
    else if((m_pAllCuesInfo) && (m_pAllCuesInfo->nCuePoints) &&
            (m_pAllCuesInfo->pCuePointInfo) )
    {
      bool bSeekDone = false;
      uint32 ulCuePoint = 0;
      if(bForward || bClosest)
      {
        cue_point_info* pCuePointInfo = m_pAllCuesInfo->pCuePointInfo;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "MKAVParser Fwd Seek, Seek TIME %llu ",nReposTime);
        for (; (ulCuePoint < m_pAllCuesInfo->nCuePoints) && (!bSeekDone);
             ulCuePoint++)
        {
          /* Look for entry whose time is more than seek time.
             If closest seek is selected, then Parser provides last entry
             as o/p seek time. */
          if((pCuePointInfo[ulCuePoint].nCueTime >= nReposTime) ||
             ((ulCuePoint == m_pAllCuesInfo->nCuePoints - 1) && (bClosest)) )
          {
            uint8 ucPrevEntry = 0;
            /* If there is a request for Closest seek, then check the closest
               entry and seek to the entry whose timestamp is closer to the
               desired seek time. */
            if ((bClosest) && (ulCuePoint) &&
                (pCuePointInfo[ulCuePoint].nCueTime > nReposTime))
            {
              if ((pCuePointInfo[ulCuePoint].nCueTime - nReposTime) >
                  (nReposTime - pCuePointInfo[ulCuePoint - 1].nCueTime) ||
                  pCuePointInfo[ulCuePoint - 1].nCueTime >= nReposTime)
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "MKAVParser Closest Seek, Seek to earlier entry %llu",
                          pCuePointInfo[ulCuePoint - 1].nCueTime);
                ucPrevEntry = 1;
              }
            }
            //! Subtract in case closest seek requested
            ulCuePoint -= ucPrevEntry;
            /* If Parser uses block number and cluster position is not updated
               to actual block number, then Parser updates cluster offset value
               linearly by parsing block header. */
            if (pCuePointInfo[ulCuePoint].pCueTrackPosnInfo &&
                pCuePointInfo[ulCuePoint].pCueTrackPosnInfo->nCueBlockNumber &&
                (0 == ulCuePoint ||
                pCuePointInfo[ulCuePoint].pCueTrackPosnInfo->nCueClusterPosn ==
                pCuePointInfo[ulCuePoint-1].pCueTrackPosnInfo->nCueClusterPosn))
            {
              UpdateSampleOffset(pCuePointInfo + ulCuePoint, ulTrackId);
            }
            retError = UpdateSeekSampleProperties(ulTrackId, pSampleInfo,
                                                  pCuePointInfo + ulCuePoint);
            if (MKAV_API_INVALID_PARAM == retError)
            {
              ulCuePoint ++;
            }
            /* Mark the flag as TRUE if sync frame is found and sample offset
               is updated successfully. */
            else if(MKAV_API_SUCCESS == retError)
            {
              bSeekDone = true;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                      "MKAV Parser Fwd Seek successful TIME %llu ",
               m_pAllCuesInfo->pCuePointInfo[ulCuePoint].nCueTime);
            }
          }//if(m_pAllCuesInfo->pCuePointInfo[j].nCueTime >= nReposTime)
        }//for (uint32 j = 0; j < m_pAllCuesInfo->nCuePoints;j++)
      }
      else
      {
        //For backward seek look from end
        ulCuePoint = (m_pAllCuesInfo->nCuePoints - 1);
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "Seeking backward, Seek TIME %llu ",nReposTime);
        cue_point_info* pCuePointInfo = m_pAllCuesInfo->pCuePointInfo;
        for (; ((int32)ulCuePoint >= 0) && (!bSeekDone); ulCuePoint--)
        {
          if(pCuePointInfo[ulCuePoint].nCueTime <= nReposTime)
          {
            retError = UpdateSeekSampleProperties(ulTrackId, pSampleInfo,
                                                  pCuePointInfo + ulCuePoint);
            if (MKAV_API_SUCCESS == retError)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "MKAV Parser backward Seek successful TIME %llu ",
                   pCuePointInfo[ulCuePoint].nCueTime);
              bSeekDone = true;
            }
            else if (MKAV_API_INVALID_PARAM == retError)
            {
              ulCuePoint--;
            }
          }//if(m_pAllCuesInfo->pCuePointInfo[j].nCueTime <= nReposTime)
        }//for (uint32 j = 0; j < m_pAllCuesInfo->nCuePoints;j++)
      }//if (bForward == false)
    }//if(m_pAllCuesInfo && m_pAllCuesInfo->nCuePoints)
    else if (m_SeekTableInfo.pSeekTableEntries)
    {
      retError = SeekUsingDynamicSeekTable(nReposTime, ulTrackId, bForward,
                                           bClosest, pSampleInfo);
    }//if (m_SeekTableInfo.pSeekTableEntries)
  }//if(codectype != MKAV_UNKNOWN_CODEC)

  //! Move Parser State to READY before reporting Seek Status
  m_eParserState = MKAVPARSER_READY;
  return retError;
}
/* =============================================================================
FUNCTION:
 MKAVParser::GetVideoWidth

DESCRIPTION:
Returns video width for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Video width
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 MKAVParser::GetVideoWidth(uint32 id)
{
  uint32 ulWidth = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"GetVideoWidth");
  for(int i = 0; (i< m_nstreams) && m_pTrackEntry; i++)
  {
    if( (m_pTrackEntry[i].TrackNo == id)&& (m_pTrackEntry[i].pVideoInfo) )
    {
      ulWidth = (uint32)m_pTrackEntry[i].pVideoInfo->PixelWidth;
      break;
    }
  }
  return ulWidth;
}
/* =============================================================================
FUNCTION:
 MKAVParser::GetVideoHeight

DESCRIPTION:
Returns video height for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Video height
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 MKAVParser::GetVideoHeight(uint32 id)
{
  uint32 ulHeight = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"GetVideoHeight");
  for(int i = 0; (i< m_nstreams) && m_pTrackEntry; i++)
  {
    if((m_pTrackEntry[i].TrackNo == (uint64)id)&& (m_pTrackEntry[i].pVideoInfo))
    {
      ulHeight = (uint32)m_pTrackEntry[i].pVideoInfo->PixelHeight;
      break;
    }
  }
  return ulHeight;
}

/* ============================================================================
  @brief  GetAudioTrackProperties.

  @details    This function is used to return Audio samples bit-depth.

  @param[in]      ulTrackId           Track ID.

  @return  Returns the BitDepthi available.
           By default this field will be reset to ZERO.
  @note       None.
============================================================================ */
bool MKAVParser::GetAudioTrackProperties(uint32 ulTrackId,
                                         mkav_audio_info* pAudioInfo)
{
  bool bAvailable = false;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"GetAudioTrackProperties");
  for(uint32 ulIndex = 0; (ulIndex< m_nstreams) && m_pTrackEntry; ulIndex++)
  {
    if( (m_pTrackEntry[ulIndex].TrackNo == ulTrackId) && (pAudioInfo) &&
        (m_pTrackEntry[ulIndex].pAudioInfo) )
    {
      memcpy(pAudioInfo, m_pTrackEntry[ulIndex].pAudioInfo,
             sizeof(mkav_audio_info));
      pAudioInfo->ullEncoderDelay     = m_pTrackEntry[ulIndex].nCodecDelay;
      pAudioInfo->ullSeekPrerollDelay = m_pTrackEntry[ulIndex].nSeekPreRoll;
      bAvailable = true;
      break;
    }
  }
  return bAvailable;
}

/* =============================================================================
FUNCTION:
 MKAVParser::GetCodecHeader

DESCRIPTION:
Returns codec specific header for given trackid
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns codec specific header for given trackid
SIDE EFFECTS:
  None.
=============================================================================*/
uint8* MKAVParser::GetCodecHeader(uint32 id, bool bRawCodec)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCodecHeader");

  uint8* pHeader = NULL;
  for(int i = 0; (i< m_nstreams) && m_pTrackEntry; i++)
  {
    if(m_pTrackEntry[i].TrackNo == id)
    {
      if(m_pTrackEntry[i].TrackType == MKAV_TRACK_TYPE_VIDEO)
      {
        if(m_pTrackEntry[i].pVideoInfo && m_pTrackEntry[i].pCodecPvt)
        {
          //! If codec data is required as it is, then Parser reads whole
          //! Private codec data and provides
          if((MKAV_AVC1_VIDEO_CODEC == m_pTrackEntry[i].CodecType) &&
             (false == bRawCodec) )
          {
            //we have codec pvt header, parse it to prepare the way we want it to send downstream
            if(!m_pTrackEntry[i].pVideoInfo->pCodecHdr)
            {
              uint8 noffset = 0;
              mkav_avc1_info* pcodec = (mkav_avc1_info*)
                                       MM_Malloc(sizeof(mkav_avc1_info));
              if(pcodec)
              {
                memset(pcodec,0,sizeof(mkav_avc1_info));
                noffset++;//skip reserved field
                pcodec->nProfile = m_pTrackEntry[i].pCodecPvt[noffset];
                noffset++;//skip profile field
                noffset++;//skip reserved field
                pcodec->nLevel = m_pTrackEntry[i].pCodecPvt[noffset];
                noffset++;//skip level field
                pcodec->NALU_Len_Minus1 = (m_pTrackEntry[i].pCodecPvt[noffset] & 0x03);
                noffset++;//skip the nalu length field
                pcodec->nSP = (m_pTrackEntry[i].pCodecPvt[noffset] & 0x1F);
                noffset++;//skip the byte
                pcodec->pSPLengths = (uint16*)MM_Malloc(pcodec->nSP * sizeof(uint16));

                if(pcodec->pSPLengths)
                {
                  memset(pcodec->pSPLengths,0,(pcodec->nSP * 2));
                  int nTotalLength = 0;
                  pcodec->nOutData += (pcodec->nSP * AVC1_START_CODE_SIZE);
                  for(int j = 0; j < pcodec->nSP;j++)
                  {
                    memcpy(pcodec->pSPLengths+j,(m_pTrackEntry[i].pCodecPvt+noffset),2);
                    MKV_REVERSE_ENDIAN((uint8*)pcodec->pSPLengths+j,2);
                    noffset = uint8(noffset + 2);
                    if(!pcodec->pSPData)
                    {
                      pcodec->pSPData = (uint8*)MM_Malloc(pcodec->pSPLengths[j]);
                      if(pcodec->pSPData)
                      {
                        memcpy(pcodec->pSPData,
                               m_pTrackEntry[i].pCodecPvt+noffset,
                               pcodec->pSPLengths[j]);
                        nTotalLength += pcodec->pSPLengths[j];
                      }
                    }
                    else
                    {
                      uint8* ptemp = (uint8*)MM_Realloc((uint8*)pcodec->pSPData,
                                                        nTotalLength +
                                                        pcodec->pSPLengths[j]);
                      if(ptemp)
                      {
                        pcodec->pSPData = ptemp;
                        memcpy(ptemp + nTotalLength,
                               m_pTrackEntry[i].pCodecPvt + noffset,
                               pcodec->pSPLengths[j]);
                        nTotalLength += pcodec->pSPLengths[j];
                      }
                    }
                    noffset = (uint8)(noffset + pcodec->pSPLengths[j]);
                  }
                  pcodec->nSPData = nTotalLength;
                  pcodec->nOutData += nTotalLength;
                }//if(pcodec->pSPLengths)
                pcodec->nPP = m_pTrackEntry[i].pCodecPvt[noffset];
                pcodec->pPPLengths = (uint16*)MM_Malloc(pcodec->nPP * 2);
                noffset++;

                if(pcodec->pPPLengths)
                {
                  int nTotalLength = 0;
                  memset(pcodec->pPPLengths,0,(pcodec->nPP * 2));
                  pcodec->nOutData += (pcodec->nPP * AVC1_START_CODE_SIZE);
                  for(int j = 0; j < pcodec->nPP;j++)
                  {
                    memcpy(pcodec->pPPLengths+j,(m_pTrackEntry[i].pCodecPvt+noffset),2);
                    MKV_REVERSE_ENDIAN((uint8*)pcodec->pPPLengths+j,2);
                    noffset = (uint8)(noffset + 2);
                    if(!pcodec->pPPData)
                    {
                      pcodec->pPPData = (uint8*)MM_Malloc(pcodec->pPPLengths[j]);
                      if(pcodec->pPPData)
                      {
                        memcpy(pcodec->pPPData,
                               m_pTrackEntry[i].pCodecPvt+noffset,
                               pcodec->pPPLengths[j]);
                        nTotalLength += pcodec->pPPLengths[j];
                      }
                    }
                    else
                    {
                      uint8* ptemp =(uint8*)MM_Realloc((uint8*)pcodec->pPPData,
                                                       nTotalLength +
                                                       pcodec->pPPLengths[j]);
                      if(ptemp)
                      {
                        pcodec->pPPData = ptemp;
                        memcpy(ptemp + nTotalLength,
                               m_pTrackEntry[i].pCodecPvt+noffset,
                               pcodec->pPPLengths[j]);
                        nTotalLength += pcodec->pPPLengths[j];
                      }
                    }
                    noffset = (uint8)(noffset + pcodec->pPPLengths[j]);
                  }
                  pcodec->nPPData = nTotalLength;
                  pcodec->nOutData += nTotalLength;
                }//if(pcodec->pPPLengths)
                PrepareAVCCodecInfo(pcodec);
                m_pTrackEntry[i].pVideoInfo->pCodecHdr = pcodec;
                pHeader = pcodec->pOutData;
              }//if(pcodec)
            }//if(!m_pTrackEntry[i].pVideoInfo->pCodecHdr)
            else
            {
              pHeader = (uint8*)((mkav_avc1_info*)
                        m_pTrackEntry[i].pVideoInfo->pCodecHdr)->pOutData;
            }
          }//if(MKAV_AVC1_VIDEO_CODEC == m_pTrackEntry[i].CodecType)
          else if ((MKAV_HEVC_VIDEO_CODEC == m_pTrackEntry[i].CodecType) &&
                   (false == bRawCodec) )
          {
            pHeader = (uint8*)m_pTrackEntry[i].pVideoInfo->pCodecHdr;
            if (!m_pTrackEntry[i].pVideoInfo->pCodecHdr)
            {
              if (m_pTrackEntry[i].nCodecConfigDataSize)
              {
                m_pTrackEntry[i].pVideoInfo->pCodecHdr =
                  MM_Malloc(m_pTrackEntry[i].nCodecConfigDataSize);
                pHeader = (uint8*)m_pTrackEntry[i].pVideoInfo->pCodecHdr;
                (void)PrepareHEVCCodecInfo(m_pTrackEntry[i].pCodecPvt,
                                           m_pTrackEntry[i].nCodecPvtSize,
                                           &m_pTrackEntry[i].nNALULenMinusOne,
                                           pHeader);
              }
            }
          } //if (MKAV_HEVC_VIDEO_CODEC == m_pTrackEntry[i].CodecType)
          else
          {
            pHeader = (uint8*)m_pTrackEntry[i].pCodecPvt;
          }
        }//if(m_pTrackEntry[i].pVideoInfo && m_pTrackEntry[i].pCodecPvt)
      }//if(m_pTrackEntry[i].TrackType == MKAV_TRACK_TYPE_VIDEO)
      if(m_pTrackEntry[i].TrackType == MKAV_TRACK_TYPE_AUDIO)
      {
        /*
        * In case of vorbis, pvt codec data stores sizes of first 2 vorbis packet
        * using XIPH lacing followed by 3 vorbis packets.
        * Skip all of such leading bytes to make sure data starts with
        * vorbis identification header.
        */
        if((m_pTrackEntry[i].CodecType == MKAV_VORBIS_AUDIO_CODEC)&&
           (m_pTrackEntry[i].pCodecPvt)&&
           (m_pTrackEntry[i].nCodecPvtSize >= MKAV_VORBIS_CODEC_START_SIZE) )
        {
          for(int itr = 0; (itr+MKAV_VORBIS_CODEC_START_SIZE) <= (int)m_pTrackEntry[i].nCodecPvtSize;itr++)
          {
            if(memcmp(m_pTrackEntry[i].pCodecPvt+itr,MKAV_VORBIS_CODEC_START,MKAV_VORBIS_CODEC_START_SIZE)== 0)
            {
              pHeader = m_pTrackEntry[i].pCodecPvt+itr;
            }
          }
        }
        else
        {
          pHeader = (uint8*)m_pTrackEntry[i].pCodecPvt;
        }
      }
      break;
    }//if(m_pTrackEntry[i].TrackNo == id)
  }
  return pHeader;
}
/* =============================================================================
FUNCTION:
 MKAVParser::GetCodecHeaderSize

DESCRIPTION:
Returns codec specific header size for given trackid
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns codec specific header size for given trackid
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 MKAVParser::GetCodecHeaderSize(uint32 id, bool bRawCodec)
{
  uint32 nsize = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"GetCodecHeaderSize");
  for(int i = 0; (i< m_nstreams) && m_pTrackEntry; i++)
  {
    if(m_pTrackEntry[i].TrackNo == id)
    {
      if(m_pTrackEntry[i].TrackType == MKAV_TRACK_TYPE_VIDEO)
      {
        if((m_pTrackEntry[i].CodecType == MKAV_AVC1_VIDEO_CODEC) &&
           (m_pTrackEntry[i].pVideoInfo->pCodecHdr))
        {
          //! Even if client requests for whole private data, Parser has to
          //! prepare H264 codec data to identify NAL Unit size length.
          mkav_avc1_info* avcinfo = (mkav_avc1_info*)
                                    m_pTrackEntry[i].pVideoInfo->pCodecHdr;
          nsize = FILESOURCE_MAX(avcinfo->nOutData,
                                 m_pTrackEntry[i].nCodecPvtSize);
        }
        else if (MKAV_HEVC_VIDEO_CODEC == m_pTrackEntry[i].CodecType)
        {
          //! Even if client requests for whole private data, Parser has to
          //! prepare HEVC codec data to identify NAL Unit size length.
          if (!m_pTrackEntry[i].nCodecConfigDataSize)
          {
            m_pTrackEntry[i].nCodecConfigDataSize =
              PrepareHEVCCodecInfo(m_pTrackEntry[i].pCodecPvt,
                                   m_pTrackEntry[i].nCodecPvtSize,
                                   &m_pTrackEntry[i].nNALULenMinusOne);
          }
          nsize = m_pTrackEntry[i].nCodecConfigDataSize;
          if (bRawCodec)
          {
            nsize = FILESOURCE_MAX(m_pTrackEntry[i].nCodecConfigDataSize,
                                   m_pTrackEntry[i].nCodecPvtSize);
          }
        }
      }//if(m_pTrackEntry[i].TrackType == MKAV_TRACK_TYPE_VIDEO)
      else if(m_pTrackEntry[i].TrackType == MKAV_TRACK_TYPE_AUDIO)
      {
        /*
        * In case of vorbis, pvt codec data stores sizes of first 2 packets
        * using XIPH lacing followed by 3 vorbis packets of codec data.
        * Parse first few bytes to get sizes of 2 Vorbis Packets, third packet
        * size is difference of total size and sum of first two vorbis packets.
        * After size fields, we will have Vorbis data section.
        */
        if((m_pTrackEntry[i].CodecType == MKAV_VORBIS_AUDIO_CODEC)&&
           (m_pTrackEntry[i].pCodecPvt)&&
           (m_pTrackEntry[i].nCodecPvtSize >= MKAV_VORBIS_CODEC_START_SIZE) )
        {
          if (0 == m_nCodecHdrSizes[0])
          {
            //0th byte indicate number of frames/packets present
            uint32 ulBufIndex       = 1;
            uint8* pCodecData       = m_pTrackEntry[i].pCodecPvt;
            uint32 ulCodecDataSize  = m_pTrackEntry[i].nCodecPvtSize;
            uint32 ulCount          = pCodecData[0];
            uint32 ulWriteIndex     = 0;
            uint32 ulPrevFramesSize = 0;
            while(ulWriteIndex < ulCount && ulBufIndex < ulCodecDataSize)
            {
              m_nCodecHdrSizes[ulWriteIndex] = 0;
              while(pCodecData[ulBufIndex] == 0xFF)
              {
                m_nCodecHdrSizes[ulWriteIndex] += pCodecData[ulBufIndex];
                ulBufIndex++;
              }
              m_nCodecHdrSizes[ulWriteIndex] += pCodecData[ulBufIndex++];
              ulPrevFramesSize               += m_nCodecHdrSizes[ulWriteIndex];
              ulWriteIndex++;
            }
            m_nCodecHdrSizes[ulWriteIndex] = m_pTrackEntry[i].nCodecPvtSize -
                                             ulPrevFramesSize - ulBufIndex;
            nsize = ulCodecDataSize - ulBufIndex;
          }
          else
          {
            uint32 ulIndex = 0;
            while( ulIndex < MKAV_VORBIS_CODEC_HDRS)
            {
              nsize += m_nCodecHdrSizes[ulIndex++];
            }
          }
        }
      }
      //Update codec pvtSize if available and nsize not updated already
      if( ( nsize == 0 )&& ( m_pTrackEntry[i].nCodecPvtSize != 0 ))
      {
        nsize = m_pTrackEntry[i].nCodecPvtSize;
      }
      break;
    }//if(m_pTrackEntry[i].TrackNo == id)
  }//for(int i = 0; (i< m_nstreams) && m_pTrackEntry; i++)
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"GetCodecHeaderSize nsize %lu",nsize);
  return nsize;
}
/* =============================================================================
FUNCTION:
 MKAVParser::ParseVideoInfo

DESCRIPTION:
Parses audio info from given offset and stores it into given Video info struct

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 MKAVPARSER_SUCCESS if no error occurs otherwise returns resp. error code
SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS MKAVParser::ParseVideoInfo(uint8 *pDataBuffer,
                                           uint32 ullElementSize,
                                           mkav_video_info* pVideoinfo)
{
  MKAV_API_STATUS status = MKAV_API_INVALID_PARAM;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"ParseVideoInfo %lu", ullElementSize);
  uint32 ulIndex      = 0;
  uint32 ulMatchedId  = 0;
  bool noerror        = true;
  if(ullElementSize && pVideoinfo)
  {
    status = MKAV_API_FAIL;
    memset(pVideoinfo,0,sizeof(mkav_video_info));

    while(noerror && (ulIndex < ullElementSize))
    {
      bool  bok             = true;
      uint8 ucEBMLId        = pDataBuffer[ulIndex];
      uint8 ucEBMLIdBytes   = AtomIdBytes(pDataBuffer + ulIndex);
      uint8 ucEBMLSizeBytes = 0;
      uint8 *pMemory        = NULL;
      uint64 ullSize        = AtomSize(pDataBuffer + ulIndex + ucEBMLIdBytes,
                                       &ucEBMLSizeBytes);

      /* Check whether complete atom is read into local buffer or not*/
      if(ullSize > (ullElementSize - ulIndex))
      {
        noerror = false;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
          "ParseVideoInfo is failed, atom is corrupted");
        break;
      }

      /* Compare Index against standard indexes that can occur in Video atom */
      if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_COLOUR_SPACE,
                 EBML_ELEMENT_ID_SIZE_THREE) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_COLOUR_SPACE");
        #endif
        ulMatchedId = EBML_VIDEO_COLOUR_SPACE;
        pMemory = (uint8*)&(pVideoinfo->ColourSpace);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_GAMMA_VALUE,
                      EBML_ELEMENT_ID_SIZE_THREE) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_GAMMA_VALUE");
        #endif
        ulMatchedId = EBML_VIDEO_GAMMA_VALUE;
        pMemory = (uint8*)&(pVideoinfo->GammaValue);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_FRAME_RATE,
                      EBML_ELEMENT_ID_SIZE_THREE) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_FRAME_RATE");
        #endif
        ulMatchedId = EBML_VIDEO_FRAME_RATE;
        pMemory = (uint8*)&(pVideoinfo->FrameRate);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_STEREO_MODE,
                      sizeof(EBML_VIDEO_STEREO_MODE)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_STEREO_MODE");
        #endif
        ulMatchedId = EBML_VIDEO_STEREO_MODE;
        pMemory = (uint8*)&(pVideoinfo->StereoMode);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_PIXEL_CROP_BOTTOM,
                      sizeof(EBML_VIDEO_PIXEL_CROP_BOTTOM)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_MEDIUM,"EBML_VIDEO_PIXEL_CROP_BOTTOM");
        #endif
        ulMatchedId = EBML_VIDEO_PIXEL_CROP_BOTTOM;
        pMemory = (uint8*)&(pVideoinfo->PixelCropBottom);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_PIXEL_CROP_TOP,
                      sizeof(EBML_VIDEO_PIXEL_CROP_TOP)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_PIXEL_CROP_TOP");
        #endif
        ulMatchedId = EBML_VIDEO_PIXEL_CROP_TOP;
        pMemory = (uint8*)&(pVideoinfo->PixelCropTop);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_PIXEL_CROP_LEFT,
                      sizeof(EBML_VIDEO_PIXEL_CROP_LEFT)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_PIXEL_CROP_LEFT");
        #endif
        ulMatchedId = EBML_VIDEO_PIXEL_CROP_LEFT;
        pMemory = (uint8*)&(pVideoinfo->PixelCropLeft);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_PIXEL_CROP_RIGHT,
                      sizeof(EBML_VIDEO_PIXEL_CROP_RIGHT)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_VIDEO_PIXEL_CROP_RIGHT");
        #endif
        ulMatchedId = EBML_VIDEO_PIXEL_CROP_RIGHT;
        pMemory = (uint8*)&(pVideoinfo->PixelCropRight);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_DISPLAY_WIDTH,
                      sizeof(EBML_VIDEO_DISPLAY_WIDTH)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_DISPLAY_WIDTH");
        #endif
        ulMatchedId = EBML_VIDEO_DISPLAY_WIDTH;
        pMemory = (uint8*)&(pVideoinfo->DisplayWidth);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_DISPLAY_HEIGHT,
                      sizeof(EBML_VIDEO_DISPLAY_HEIGHT)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_DISPLAY_HEIGHT");
        #endif
        ulMatchedId = EBML_VIDEO_DISPLAY_HEIGHT;
        pMemory = (uint8*)&(pVideoinfo->DisplayHeight);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_DISPLAY_UNIT,
                      sizeof(EBML_VIDEO_DISPLAY_UNIT)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_DISPLAY_UNIT");
        #endif
        ulMatchedId = EBML_VIDEO_DISPLAY_UNIT;
        pMemory = (uint8*)&(pVideoinfo->DisplayUnit);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_VIDEO_ASPECTRATIO_TYPE,
                      sizeof(EBML_VIDEO_ASPECTRATIO_TYPE)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_VIDEO_ASPECTRATIO_TYPE");
        #endif
        ulMatchedId = EBML_VIDEO_ASPECTRATIO_TYPE;
        pMemory = (uint8*)&(pVideoinfo->AspectRatioType);
      }
      else if(ucEBMLId == EBML_VIDEO_FLAG_INTERLACED)
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_FLAG_INTERLACED");
        #endif
        ulMatchedId = EBML_VIDEO_FLAG_INTERLACED;
        pMemory = (uint8*)&(pVideoinfo->FlagInterlaced);
      }
      else if(ucEBMLId == EBML_VIDEO_PIXEL_WIDTH)
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_PIXEL_WIDTH");
        #endif
        ulMatchedId = EBML_VIDEO_PIXEL_WIDTH;
        pMemory = (uint8*)&(pVideoinfo->PixelWidth);
      }
      else if(ucEBMLId == EBML_VIDEO_PIXEL_HEIGHT)
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "EBML_VIDEO_PIXEL_HEIGHT");
        #endif
        ulMatchedId = EBML_VIDEO_PIXEL_HEIGHT;
        pMemory = (uint8*)&(pVideoinfo->PixelHeight);
      }
      else
      {
#ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Unknown/Unsupported atom ");
#endif
        bok = false;
      }
      ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
      if( (ulMatchedId != 0) && pMemory )
      {
        MKV_REVERSE_ENDIAN(pDataBuffer + ulIndex,(uint8)ullSize);
        memcpy(pMemory, pDataBuffer + ulIndex,(size_t)ullSize);
      }//if( (ulMatchedId != 0))
      ulIndex += (uint32)ullSize;
    }//while(noerror)
  }//if(ullSize && pVideoinfo)
  if(ulIndex == ullElementSize && noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* =============================================================================
FUNCTION:
 MKAVParser::ParseAudioInfo

DESCRIPTION:
Parses audio info from given offset and stores it into given aud info structure
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 MKAVPARSER_SUCCESS if no error occurs otherwise returns resp. error code
SIDE EFFECTS:
  None.
=============================================================================*/
MKAV_API_STATUS MKAVParser::ParseAudioInfo(uint8 *pDataBuffer,
                                           uint64 ullElementSize,
                                           mkav_audio_info* pAudioInfo)
{
  MKAV_API_STATUS status = MKAV_API_INVALID_PARAM;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "ParseAudioInfo %llu",ullElementSize);
  uint32 ulIndex         = 0;
  uint32 ulMatchedId     = 0;
  size_t ulMaxTargetSize = 0;
  bool noerror           = true;
  if(ullElementSize && pAudioInfo)
  {
    status = MKAV_API_FAIL;
    while(noerror && (ulIndex < ullElementSize))
    {
      bool  bok             = true;
      uint8 ucEBMLId        = pDataBuffer[ulIndex];
      uint8 ucEBMLIdBytes   = AtomIdBytes(pDataBuffer + ulIndex);
      uint8 ucEBMLSizeBytes = 0;
      uint8 *pMemory        = NULL;
      uint64 ullSize        = AtomSize(pDataBuffer + ulIndex + ucEBMLIdBytes,
                                       &ucEBMLSizeBytes);

      /* Check whether complete atom is read into local buffer or not*/
      if(ullSize > (ullElementSize - ulIndex))
      {
        noerror = false;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
          "ParseAudioInfo is failed, atom is corrupted");
        break;
      }

      /* Compare Index with standard audio indexes */
      if(!memcmp(pDataBuffer + ulIndex, &EBML_AUDIO_OUT_SAMPL_FREQ,
                 sizeof(EBML_AUDIO_OUT_SAMPL_FREQ)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_AUDIO_OUT_SAMPL_FREQ");
        #endif
        ulMatchedId = EBML_AUDIO_OUT_SAMPL_FREQ;
        pMemory = (uint8*)&(pAudioInfo->OutputSamplingFrequency);
        ulMaxTargetSize = sizeof(pAudioInfo->OutputSamplingFrequency);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_AUDIO_CHANNLES_POSN,
                      sizeof(EBML_AUDIO_CHANNLES_POSN)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_AUDIO_CHANNLES_POSN");
        #endif
        ulMatchedId = EBML_AUDIO_CHANNLES_POSN;
        pMemory = (uint8*)&(pAudioInfo->ullChannelMask);
        ulMaxTargetSize = sizeof(pAudioInfo->ullChannelMask);
      }
      else if(!memcmp(pDataBuffer + ulIndex, &EBML_AUDIO_SAMPLE_BITDEPTH,
                      sizeof(EBML_AUDIO_SAMPLE_BITDEPTH)) )
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_AUDIO_SAMPLE_BITDEPTH");
        #endif
        ulMatchedId = EBML_AUDIO_SAMPLE_BITDEPTH;
        pMemory = (uint8*)&(pAudioInfo->ullBitsPerSample);
        ulMaxTargetSize = sizeof(pAudioInfo->ullBitsPerSample);
      }
      else if(ucEBMLId == EBML_AUDIO_SAMPLING_FREQ)
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_AUDIO_SAMPLING_FREQ");
        #endif
        ulMatchedId = EBML_AUDIO_SAMPLING_FREQ;
        pMemory = (uint8*)&(pAudioInfo->SamplingFrequency);
        ulMaxTargetSize = sizeof(pAudioInfo->SamplingFrequency);
      }
      else if(ucEBMLId == EBML_AUDIO_NO_CHANNELS)
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_AUDIO_NO_CHANNELS");
        #endif
        ulMatchedId = EBML_AUDIO_NO_CHANNELS;
        pMemory = (uint8*)&(pAudioInfo->ullNumChannels);
        ulMaxTargetSize = sizeof(pAudioInfo->ullNumChannels);
      }
      else
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"Unsupported/Unknown atom ");
        #endif
        bok = false;
      }
      ulIndex += (ucEBMLSizeBytes + ucEBMLIdBytes);
      if( (ulMatchedId != 0) && pMemory )
      {
        if(ullSize <= (uint64)ulMaxTargetSize)
        {
          bool bIsCopy = true;
          if((ulMatchedId == EBML_AUDIO_SAMPLING_FREQ)||
             (ulMatchedId == EBML_AUDIO_OUT_SAMPL_FREQ))
          {
            //Special case for handling 32/64 bit floating point values
            if(ullSize <= sizeof(float))
            {
              bIsCopy = false;
              float freqval = 0.0;
              memcpy(&freqval, pDataBuffer + ulIndex, (size_t)ullSize);
              MKV_REVERSE_ENDIAN((uint8*)&freqval,(uint8)ullSize);
              if(ulMatchedId == EBML_AUDIO_SAMPLING_FREQ)
              {
                pAudioInfo->SamplingFrequency = (uint64)freqval;
              }
              else
              {
                pAudioInfo->OutputSamplingFrequency = (uint64)freqval;
              }
            }//if(ullSize <= sizeof(float))
            else if (sizeof(double) == ullSize)
            {
              double dFreqVal = 0.0;
              bIsCopy = false;
              memcpy(&dFreqVal, pDataBuffer + ulIndex, (size_t)ullSize);
              MKV_REVERSE_ENDIAN((uint8*)&dFreqVal,(uint8)ullSize);
              if(ulMatchedId == EBML_AUDIO_SAMPLING_FREQ)
              {
                pAudioInfo->SamplingFrequency = (uint64)dFreqVal;
              }
              else
              {
                pAudioInfo->OutputSamplingFrequency = (uint64)dFreqVal;
              }
            }//else if (sizeof(double) == ullSize)
          }
          if(bIsCopy)
          {
            memcpy(pMemory, pDataBuffer + ulIndex, (size_t)ullSize);
            MKV_REVERSE_ENDIAN((uint8*)pMemory,(uint8)ullSize);
          }
        }
        else
        {
          noerror = false;
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
            "ParseAudioInfo failed datasize %llu > ulMaxTargetSize %lu",
            ullSize, ulMaxTargetSize);
        }
      }//if( (ulMatchedId != 0))
      ulIndex += (uint32)ullSize;
    }//while(noerror)

    if(!pAudioInfo->OutputSamplingFrequency)
    {
      //Default value for sampling frequency is 8KHz
      if(!pAudioInfo->SamplingFrequency)
      {
        pAudioInfo->SamplingFrequency = 8000;
      }
      pAudioInfo->OutputSamplingFrequency = pAudioInfo->SamplingFrequency;
    }
    if(noerror && (ulIndex == ullElementSize))
    {
      status = MKAV_API_SUCCESS;
    }
    if(!pAudioInfo->ullNumChannels)
    {
      pAudioInfo->ullNumChannels = 2;
    }
  }//if(ullElementSize && pAudioInfo)
  return status;
}

/* ============================================================================
  @brief  Parse and stores content encode element details.

  @details    This function is used to parse Content encode element.

  @param[in]      pucDataBuf        Buffer which has Content Element data.
  @param[in]      ullContEncodeSize Content Encode Element size.
  @param[in/out]  pEncodeInfo       Pointer to store encode properties.

  @return     MKAV_API_SUCCESS indicating parsing done successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
MKAV_API_STATUS MKAVParser::ParseContentEncodeInfo(uint8 *pucDataBuf,
                                                 uint64 ullContEncodeSize,
                                                 mkav_encode_info* pEncodeInfo)
{
  MKAV_API_STATUS status = MKAV_API_INVALID_PARAM;
  uint32 ulIndex         = 0;
  bool   noerror         = true;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "ParseContentEncodeInfo %llu",
               ullContEncodeSize);
  if(ullContEncodeSize && pEncodeInfo)
  {
    status = MKAV_API_FAIL;
    while(noerror && (ulIndex < ullContEncodeSize))
    {
      uint8 ucEBMLIdBytes   = AtomIdBytes(pucDataBuf + ulIndex);
      uint8 ucEBMLSizeBytes = 0;
      uint64 ullSize        = AtomSize(pucDataBuf + ulIndex + ucEBMLIdBytes,
                                       &ucEBMLSizeBytes);

      /* Check whether complete atom is read into local buffer or not*/
      if(ullSize > (ullContEncodeSize - ulIndex))
      {
        noerror = false;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
          "ParseContentEncodeInfo is failed, atom is corrupted");
        break;
      }

      /* Compare Index with standard audio indexes */
      if(!memcmp(pucDataBuf + ulIndex, &EBML_TRACK_CONTENTS_ENCODING_ORDER,
                 sizeof(EBML_TRACK_CONTENTS_ENCODING_ORDER)) )
      {
        ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "EBML_TRACK_CONTENTS_ENCODING_ORDER");
        #endif
        memcpy((void*)&pEncodeInfo->ulEncodeOrder, pucDataBuf + ulIndex,
               FILESOURCE_MIN((uint32)ullSize, sizeof(uint32)));
      }
      else if(!memcmp(pucDataBuf+ulIndex, &EBML_TRACK_CONTENTS_ENCODING_SCOPE,
                      sizeof(EBML_TRACK_CONTENTS_ENCODING_SCOPE)) )
      {
        ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "EBML_TRACK_CONTENTS_ENCODING_SCOPE");
        #endif
        pEncodeInfo->eEncodeScope = (EBML_CONTENT_ENCODING_SCOPE)
                                     pucDataBuf[ulIndex];
      }
      else if(!memcmp(pucDataBuf + ulIndex, &EBML_TRACK_CONTENTS_ENCODING_TYPE,
                      sizeof(EBML_TRACK_CONTENTS_ENCODING_TYPE)) )
      {
        ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "EBML_TRACK_CONTENTS_ENCODING_TYPE");
        #endif
        pEncodeInfo->eEncodeType = (EBML_CONTENT_ENCODE_TYPE)
                                   pucDataBuf[ulIndex];
      }
      else if(!memcmp(pucDataBuf+ulIndex,&EBML_TRACK_CONTENTS_COMPRESSION_TYPE,
                      sizeof(EBML_TRACK_CONTENTS_COMPRESSION_TYPE)) )
      {
        ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "EBML_TRACK_CONTENTS_COMPRESSION_TYPE");
        #endif
        pEncodeInfo->eEncodeType = CONTENT_COMPRESSED;
        /* It is a special case. This atom contains child atoms.
           At the end of condition checks, it will skip current atom.
           So to keep the parsing data, subtract with size of data.*/
        ulIndex -= (uint32)ullSize;
      }
      else if(!memcmp(pucDataBuf + ulIndex, &EBML_TRACK_CONTENTS_ENCODING,
                      sizeof(EBML_TRACK_CONTENTS_ENCODING)) )
      {
        ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "EBML_TRACK_CONTENTS_ENCODING");
        #endif
        /* It is a special case. This atom contains child atoms.
           At the end of condition checks, it will skip current atom.
           So to keep the parsing data, subtract with size of data.*/
        ulIndex -= (uint32)ullSize;
      }
      else if(!memcmp(pucDataBuf+ulIndex,&EBML_TRACK_CONTENTS_COMPRESSION_ALGO,
                      sizeof(EBML_TRACK_CONTENTS_COMPRESSION_ALGO)) )
      {
        ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "EBML_TRACK_CONTENTS_COMPRESSION_ALGO");
        #endif
        pEncodeInfo->eCompAlgo = (EBML_CONTENT_COMPRESSION_ALGO)
                                 ((uint32)pucDataBuf[ulIndex]);
      }
      else if(!memcmp(pucDataBuf+ulIndex,&EBML_TRACK_CONTENTS_COMPRESSION_SET,
                      sizeof(EBML_TRACK_CONTENTS_COMPRESSION_SET)) )
      {
        ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "EBML_TRACK_CONTENTS_COMPRESSION_ALGO");
        #endif
        if(!pEncodeInfo->pCompSettings)
        {
          pEncodeInfo->pCompSettings    = MM_Malloc((uint32)ullSize);
          pEncodeInfo->ulCompSettingLen = (uint32)ullSize;
        }
        if(pEncodeInfo->pCompSettings)
        {
          memcpy(pEncodeInfo->pCompSettings, pucDataBuf + ulIndex,
                 (uint32)ullSize);
        }
      }
      else
      {
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"Unsupported/Unknown atom ");
        #endif
        ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
      }
      /* Skip current atom data and go to start of next atom */
      ulIndex += (uint32)ullSize;
    }//while(noerror)

    if(noerror && (ulIndex == ullContEncodeSize))
    {
      status = MKAV_API_SUCCESS;
    }
  }//if(ullContEncodeSize && pEncodeInfo)
  return status;
}

/* ============================================================================
  @brief  Parse and stores Cues Info element details.

  @details    This function is used to parse Cues Info element.

  @param[in]      ullOffset     Start offset of data.
  @param[in]      ullElemSize   Cues Element size.

  @return     MKAV_API_SUCCESS indicating parsing done successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
MKAV_API_STATUS MKAVParser::ParseCuesInfo(uint64 ullOffset,uint64 ullElemSize)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseCuesInfo %llu",ullOffset);
  #endif
  cue_point_info*  hTempCuePoint = NULL;
  uint32 ulBytesToRead = DEF_EBML_MAX_SIGNATURE_LENGTH_VAL;
  uint32 ulDataRead    = 0;
  uint32 ulIndex       = 0;
  uint64 ullEndOffset  = ullOffset + ullElemSize;
  bool noerror         = false;

  if(m_bHttpPlay && !EnsureDataCanBeRead(ullOffset, ullElemSize))
  {
    return MKAV_API_DATA_UNDERRUN;
  }

  status = MKAV_API_OUT_OF_MEMORY;
  m_pAllCuesInfo = (all_cues_info*)MM_Malloc(sizeof(all_cues_info));
  if(m_pAllCuesInfo)
  {
    memset(m_pAllCuesInfo,0,sizeof(all_cues_info));
    noerror = true;
  }
  while( (ullOffset < ullEndOffset)&& (noerror) )
  {
    bool bok   = false;
    ulIndex    = 0;
    ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                   m_pDataBuffer, DEF_DATA_BUF_SIZE);
    if(ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL)
    {
      status  =  MKAV_API_FAIL;
      noerror = false;
    }
    else
    {
      uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
      uint8 ucEBMLSizeBytes = 0;
      uint64 ullSize        = AtomSize(m_pDataBuffer + ucEBMLIdBytes,
                                       &ucEBMLSizeBytes);

      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM, "ebmlSizeBytes %d size %llu",
                   ucEBMLSizeBytes, ullSize);
      #endif

      ulIndex += (ucEBMLSizeBytes + ucEBMLIdBytes);
      if(EBML_CUE_POINT == m_pDataBuffer[0])
      {
        if(!m_pAllCuesInfo->pCuePointInfo)
        {
          m_pAllCuesInfo->pCuePointInfo = (cue_point_info*)
                                           MM_Malloc(sizeof(cue_point_info));
          if(m_pAllCuesInfo->pCuePointInfo)
          {
            memset(m_pAllCuesInfo->pCuePointInfo, 0, sizeof(cue_point_info));
            hTempCuePoint = m_pAllCuesInfo->pCuePointInfo;
            m_pAllCuesInfo->nCuePoints++;
            bok = true;
          }
        }
        else
        {
          uint32 ulAllocSize = (uint32)sizeof(cue_point_info) *
                               (m_pAllCuesInfo->nCuePoints + 1);
          cue_point_info* pTemp = (cue_point_info*)
                                  MM_Realloc( m_pAllCuesInfo->pCuePointInfo,
                                              ulAllocSize);
          if(pTemp)
          {
            m_pAllCuesInfo->pCuePointInfo = pTemp;
            memset(m_pAllCuesInfo->pCuePointInfo + m_pAllCuesInfo->nCuePoints,
                   0, sizeof(cue_point_info));
            hTempCuePoint = m_pAllCuesInfo->pCuePointInfo +
                            m_pAllCuesInfo->nCuePoints;
            m_pAllCuesInfo->nCuePoints++;
            bok = true;
          }
        }
        if(true == bok)
        {
          status = ParseCuePoint(ullOffset + ulIndex, ullSize, hTempCuePoint);
          if(MKAV_API_SUCCESS != status)
          {
            noerror = false;
          }
        }
        else
        {
          status  = MKAV_API_OUT_OF_MEMORY;
          noerror = false;
        }
      }//if(EBML_CUE_POINT == m_pDataBuffer[0])

      ullOffset += ullSize + ulIndex;
    }
  }//while( (ullElemSize < nEndOffset)&&(noerror) )
  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* ============================================================================
  @brief  Parse and stores Cues Point element details.

  @details    This function is used to parse Cues Point element.

  @param[in]      ullOffset     Start offset of data.
  @param[in]      ullElemSize   Cues Element size.
  @param[in/out]  pCuePtInfo    Pointer to store Cue point details.

  @return     MKAV_API_SUCCESS indicating parsing done successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
MKAV_API_STATUS  MKAVParser::ParseCuePoint(uint64 ullOffset, uint64 ullCueSize,
                                           cue_point_info* pCuePtInfo)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseCuePoint %llu", ullOffset);
  #endif
  uint32 ulBytesToRead = 0;
  uint32 ulDataRead    = 0;
  uint32 ulIndex       = 0;
  uint64 ullEndOffset  = ullOffset + ullCueSize;
  uint8* pucMemory     = NULL;
  size_t ulMaxCopySize = 0;
  bool   noerror       = true;

  cue_track_posn_info* hTempCueTrackPosnInfo = NULL;

  while( (ullOffset < ullEndOffset) && (noerror) )
  {
    /* Read data from file only if all the data is consumed or for the first
       time. */
    if(!ulDataRead ||
      (ulDataRead < uint64(ullEndOffset - ullOffset) &&
      (ulDataRead - ulIndex) < DEF_EBML_MX_SZ_LENGTH_VAL))
    {
      ullOffset    += ulIndex;
      ulIndex       = 0;
      ulBytesToRead = uint32(ullEndOffset - ullOffset);
      ulDataRead    = GetDataFromSource(ullOffset, ulBytesToRead,
                                        m_pDataBuffer, DEF_DATA_BUF_SIZE);
      if((ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL) &&
         (ulDataRead < ulBytesToRead) )
      {
        status  =  MKAV_API_READ_FAIL;
        noerror = false;
        break;
      }
    }
    else if(ulDataRead == ulIndex)
    {
      status = MKAV_API_SUCCESS;
      break;
    }

    uint8 ucEBMLId        = m_pDataBuffer[ulIndex];
    uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
    uint8 ucEBMLSizeBytes = 0;
    uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);

    /* Check whether complete atom is read into local buffer or not*/
    if(ullSize > (ulDataRead - ulIndex))
    {
      ulDataRead = 0;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseCuepointElement: complete atom is not read");
      #endif
      continue;
    }

    ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
    if(ucEBMLId == EBML_CUE_TIME)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "EBML_CUE_TIME %llu", ullOffset);
      #endif
      pucMemory = (uint8*)&pCuePtInfo->nCueTime;
      ulMaxCopySize = sizeof(pCuePtInfo->nCueTime);
      memcpy(pucMemory, m_pDataBuffer + ulIndex,
             FILESOURCE_MIN((uint32)ullSize,ulMaxCopySize));
      MKV_REVERSE_ENDIAN(pucMemory,
                        (uint8)FILESOURCE_MIN((uint32)ullSize, ulMaxCopySize));
    }
    else if(ucEBMLId == EBML_CUE_TRACK_POSITIONS)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "EBML_CUE_TRACK_POSITIONS %llu",
                   ullOffset);
      #endif
      if(!pCuePtInfo->pCueTrackPosnInfo)
      {
        pCuePtInfo->pCueTrackPosnInfo =(cue_track_posn_info*)
                                        MM_Malloc(sizeof(cue_track_posn_info));
        if(pCuePtInfo->pCueTrackPosnInfo)
        {
          memset(pCuePtInfo->pCueTrackPosnInfo,0,sizeof(cue_track_posn_info));
          hTempCueTrackPosnInfo = pCuePtInfo->pCueTrackPosnInfo;
          pCuePtInfo->nCueTrackPosnInfo++;
        }
        else
        {
          status  = MKAV_API_OUT_OF_MEMORY;
          noerror = false;
        }
      }
      else
      {
        uint64 AllocSize = sizeof(cue_track_posn_info)*
                           (pCuePtInfo->nCueTrackPosnInfo + 1);
        cue_track_posn_info* pTemp = (cue_track_posn_info*)
                    MM_Realloc( pCuePtInfo->pCueTrackPosnInfo,
                                (uint32)AllocSize);
        if(pTemp)
        {
          pCuePtInfo->pCueTrackPosnInfo = pTemp;
          memset(pCuePtInfo->pCueTrackPosnInfo + pCuePtInfo->nCueTrackPosnInfo,
                 0, sizeof(cue_track_posn_info));
          hTempCueTrackPosnInfo = pCuePtInfo->pCueTrackPosnInfo +
                                  pCuePtInfo->nCueTrackPosnInfo;
          pCuePtInfo->nCueTrackPosnInfo++;
        }
        else
        {
          status  = MKAV_API_OUT_OF_MEMORY;
          noerror = false;
        }
      }
      if(true == noerror)
      {
        status = ParseCueTrackPosnInfo(m_pDataBuffer + ulIndex, ullSize,
                                       hTempCueTrackPosnInfo);
        if(MKAV_API_SUCCESS != status)
        {
          noerror = false;
        }
      } //end of if(true == noerror)
    } //end of if(ucEBMLID == EBML_CUE_TRACK_POSITIONS)

    ulIndex += (uint32)ullSize;
  }//while( (ullOffset < nEndOffset)&&(noerror) )
  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* ============================================================================
  @brief  Parse and stores Cue Track Position Info element details.

  @details    This function is used to parse Cue Track Position Info element.

  @param[in]      pucDataBuf     Buffer which contains metadata.
  @param[in]      ullElementSize Element size.
  @param[in/out]  pCueTrkPosInfo Pointer to store Track posn details.

  @return     MKAV_API_SUCCESS indicating parsing done successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
MKAV_API_STATUS  MKAVParser::ParseCueTrackPosnInfo(
                                           uint8 *pucDataBuf,
                                           uint64 ullElementSize,
                                           cue_track_posn_info* pCueTrkPosInfo)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "ParseCueTrackPosnInfo");
  #endif
  cue_ref_info* hTempCueRefInfo = NULL;
  uint32 ulIndex                = 0;
  uint8* pucMemory              = NULL;
  size_t ulMaxCopySize          = 0;
  uint32 nMatchedId             = 0;
  bool   noerror                = true;

  while( (ulIndex < ullElementSize)&& (noerror) )
  {
    pucMemory     = NULL;
    ulMaxCopySize = 0;

    bool  bMaster         = false;
    bool  bok             = true;
    uint8 ucEBMLId        = pucDataBuf[ulIndex];
    uint8 ucEBMLIdBytes   = AtomIdBytes(pucDataBuf + ulIndex);
    uint8 ucEBMLSizeBytes = 0;
    uint64 ullSize        = AtomSize(pucDataBuf + ulIndex + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);

    /* Check whether complete atom is read into local buffer or not*/
    if(ullSize > (ullElementSize - ulIndex))
    {
      noerror = false;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseCueTrackPosnInfo is failed, atom is corrupted");
      break;
    }

    if(!memcmp(pucDataBuf + ulIndex, &EBML_CUE_BLOCK_NO, sizeof(uint16) ))
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CUE_BLOCK_NO ");
      #endif
      nMatchedId = EBML_CUE_BLOCK_NO;
      pucMemory = (uint8*)&pCueTrkPosInfo->nCueBlockNumber;
      ulMaxCopySize = sizeof(pCueTrkPosInfo->nCueBlockNumber);
    }
    else if(ucEBMLId == EBML_CUE_TRACK)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CUE_TRACK");
      #endif
      nMatchedId = EBML_CUE_TRACK;
      pucMemory = (uint8*)&pCueTrkPosInfo->nCueTrack;
      ulMaxCopySize = sizeof(pCueTrkPosInfo->nCueTrack);
    }
    else if(ucEBMLId == EBML_CUE_CLUSTER_POSN)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CUE_CLUSTER_POSN");
      #endif
      nMatchedId = EBML_CUE_CLUSTER_POSN;
      pucMemory = (uint8*)&pCueTrkPosInfo->nCueClusterPosn;
      ulMaxCopySize = sizeof(pCueTrkPosInfo->nCueClusterPosn);
    }
    else if(ucEBMLId == EBML_CUE_CODEC_STATE)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CUE_CODEC_STATE");
      #endif
      pucMemory = (uint8*)&pCueTrkPosInfo->nCueCodecState;
      ulMaxCopySize = sizeof(pCueTrkPosInfo->nCueCodecState);
      nMatchedId = EBML_CUE_CODEC_STATE;
    }
    else if(ucEBMLId == EBML_CUE_REFERENCE)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CUE_REFERENCE");
      #endif
      bok = false;
      nMatchedId = EBML_CUE_REFERENCE;
      if(!pCueTrkPosInfo->pCueRefInfo)
      {
        pCueTrkPosInfo->pCueRefInfo = (cue_ref_info*)
                                      MM_Malloc(sizeof(cue_ref_info));
        if(pCueTrkPosInfo->pCueRefInfo)
        {
          memset(pCueTrkPosInfo->pCueRefInfo,0,sizeof(cue_ref_info));
          hTempCueRefInfo = pCueTrkPosInfo->pCueRefInfo;
          pCueTrkPosInfo->nCueRef++;
          bMaster = true;
          bok     = true;
        }
      }
      else
      {
        cue_ref_info* pTemp = (cue_ref_info*)MM_Realloc
          ( pCueTrkPosInfo->pCueRefInfo,
            (size_t)(sizeof(cue_ref_info)*(pCueTrkPosInfo->nCueRef+1) ) );
        if(pTemp)
        {
          pCueTrkPosInfo->pCueRefInfo = pTemp;
          memset(pCueTrkPosInfo->pCueRefInfo + pCueTrkPosInfo->nCueRef, 0,
                 sizeof(cue_ref_info));
          hTempCueRefInfo = pCueTrkPosInfo->pCueRefInfo +
                            pCueTrkPosInfo->nCueRef;
          pCueTrkPosInfo->nCueRef++;
          bMaster = true;
          bok     = true;
        }
      }
      if(false == bok)
      {
        noerror = false;
        status = MKAV_API_OUT_OF_MEMORY;
      }
    }//else if(origid == EBML_CUE_REFERENCE)
    ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
    if(bok == true)
    {
      if(!bMaster)
      {
        if(pucMemory)
        {
          memcpy(pucMemory, pucDataBuf + ulIndex,
                 FILESOURCE_MIN((uint32)ullSize, ulMaxCopySize));
          MKV_REVERSE_ENDIAN( pucMemory,
                         (uint8)FILESOURCE_MIN((uint32)ullSize,ulMaxCopySize));
          if(nMatchedId == EBML_CUE_CLUSTER_POSN)
          {
            pCueTrkPosInfo->nCueClusterPosn += m_nSegmentPosn;
          }
        }
      }
      else
      {
        if(nMatchedId == EBML_CUE_REFERENCE)
        {
          status = ParseCueRefInfo(pucDataBuf + ulIndex, ullSize,
                                   hTempCueRefInfo);
          if(MKAV_API_SUCCESS != status)
          {
            bok     = false;
            noerror = false;
          }
        }
      }
    }//if(bok == true)
    ulIndex += (uint32)ullSize;
  }//while( (ulIndex < ullElementSize)&& (noerror) )
  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* ============================================================================
  @brief  Parse and stores Cue Reference Info element details.

  @details    This function is used to parse Cue Reference Info element.

  @param[in]      pucDataBuf     Buffer which contains metadata.
  @param[in]      ullElementSize Element size.
  @param[in/out]  pCueRefInfo    Pointer to store Cue Ref Info details.

  @return     MKAV_API_SUCCESS indicating parsing done successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
MKAV_API_STATUS  MKAVParser::ParseCueRefInfo(uint8 *pucDataBuf,
                                             uint64 ullElementSize,
                                             cue_ref_info* pCueRefInfo)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseCueRefInfo");
  #endif
  uint32 ulIndex       = 0;
  uint8* pucMemory     = NULL;
  size_t ulMaxCopySize = 0;
  uint32 ulMatchedId   = 0;
  bool   noerror       = true;
  while( (ulIndex < ullElementSize)&& (noerror) )
  {
    pucMemory     = NULL;
    ulMaxCopySize = 0;

    uint8 ucEBMLId        = pucDataBuf[ulIndex];
    uint8 ucEBMLIdBytes   = AtomIdBytes(pucDataBuf + ulIndex);
    uint8 ucEBMLSizeBytes = 0;
    uint64 ullSize        = AtomSize(pucDataBuf + ulIndex + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);

    /* Check whether complete atom is read into local buffer or not*/
    if(ullSize > (ullElementSize - ulIndex))
    {
      noerror = false;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "ParseCueTrackPosnInfo is failed, atom is corrupted");
      break;
    }
    if(ucEBMLId == EBML_CUE_REF_NUMBER)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CUE_REF_NUMBER");
      #endif
      pucMemory = (uint8*)&pCueRefInfo->nCueRefNo;
      ulMaxCopySize = sizeof(pCueRefInfo->nCueRefNo);
      ulMatchedId = EBML_CUE_REF_NUMBER;
    }
    else if(ucEBMLId == EBML_CUE_REF_TIME)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CUE_REF_TIME");
      #endif
      pucMemory = (uint8*)&pCueRefInfo->nCueRefTime;
      ulMaxCopySize = sizeof(pCueRefInfo->nCueRefTime);
      ulMatchedId = EBML_CUE_REF_TIME;
    }
    else if(ucEBMLId == EBML_CUE_REF_CLUSTER)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CUE_REF_CLUSTER");
      #endif
      pucMemory = (uint8*)&pCueRefInfo->nCueRefCluster;
      ulMaxCopySize = sizeof(pCueRefInfo->nCueRefCluster);
      ulMatchedId = EBML_CUE_REF_CLUSTER;
    }
    else if(ucEBMLId == EBML_CUE_REF_CODEC_STATE)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"EBML_CUE_REF_CODEC_STATE");
      #endif
      pucMemory = (uint8*)&pCueRefInfo->nCueRefCodecState;
      ulMaxCopySize = sizeof(pCueRefInfo->nCueRefCodecState);
      ulMatchedId = EBML_CUE_REF_CODEC_STATE;
    }
    ulIndex += ucEBMLSizeBytes + ucEBMLIdBytes;
    if(pucMemory)
    {
      memcpy(pucMemory, pucDataBuf + ulIndex,
             FILESOURCE_MIN((uint32)ullSize,ulMaxCopySize));
      MKV_REVERSE_ENDIAN(pucMemory,
                        (uint8)FILESOURCE_MIN((uint32)ullSize,ulMaxCopySize));
    }//if(pucMemory)
  }//while( (ulIndex < ullElementSize)&& (noerror) )
  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* =============================================================================
FUNCTION:
 MKAVParser::EnsureDataCanBeRead

DESCRIPTION:
Checks if data(given number of bytes) can be read from the given offset.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns true if read is possible else returns false;

SIDE EFFECTS:
  None.
=============================================================================*/
bool  MKAVParser::EnsureDataCanBeRead(uint64 ullOffset,uint64 ullSize)
{
  bool bret = true;
  if(m_bHttpPlay && !m_bEndOfData)
  {
    uint64 ullAvlOffset = 0;
    (void)MKAVCheckAvailableData(&ullAvlOffset, &m_bEndOfData,
                                 m_pUserData);
    if(m_bEndOfData)
    {
      m_nFileSize = ullAvlOffset;
    }
    if((ullOffset + ullSize) > ullAvlOffset)
    {
      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
       "EnsureDataCanBeRead reports UNDER RUN @offset %llu size %llu, bend %d",
       ullOffset, ullSize, m_bEndOfData);
      m_eParserPrvState = m_eParserState;
      if(false == m_bEndOfData)
      {
        m_eParserState = MKAVPARSER_DATA_UNDERRUN;
      }
      else
      {
        m_eParserState = MKAVPARSER_EOF;
      }

      bret = false;
    }
  }
  return bret;
}
/* =============================================================================
FUNCTION:
 MKAVParser::IsMetaDataParsingDone

DESCRIPTION:
Checks if entire meta data is parsed and returns true/false accordingly.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns true if metadata is parsed else returns false;

SIDE EFFECTS:
  None.
=============================================================================*/
bool  MKAVParser::IsMetaDataParsingDone()
{
  bool bret              = false;
  seekhead *pSeekHead    = NULL;
  if(m_pSeekHeadInfo && m_pSeekHeadInfo->pSeekHead)
  {
    pSeekHead = m_pSeekHeadInfo->pSeekHead;
#if 0
    int  ncount = 0;
    for(uint32 j = 0; j < m_pSeekHeadInfo->pSeekHead->nSeekInfo;j++)
    {
      if(memcmp(&EBML_SEEK_HEAD_ID,
                &(m_pSeekHeadInfo->pSeekHead->pSeekInfo[j].ebml_id),
                sizeof(EBML_SEEK_HEAD_ID)) == 0)
      {
        //count number of seek heads we should be parsing before reporting
        //metadata parsing done
        ncount++;
      }
    }
    //ncount denotes additional SEEK HEADs other than the first one.

    if( ( (ncount) && ( (ncount+1) == m_pSeekHeadInfo->nSeekHead ) )&&
        (m_pAllClustersInfo && m_pAllClustersInfo->nClusters) )
#else
    /* Report ready as soon as all mandatory metadata fields and one cluster is
       parsed. However, in the following if condition, if there is CUE atom
       (seek table) in the clip, we will parse that one as well before
       reporting to ready state.
    */
    if((m_pAllClustersInfo) && (m_pAllClustersInfo->nClusters))
#endif
    {
      /* If Segment Info and Tracks are not found before first cluster,
         then by using SeekHead find the start offsets of corresponding atoms*/
      bool bIdFound = false;
      if (!m_pSegmentInfo)
      {
        bIdFound = GetOffsetFromSeekHead(EBML_SEGMENT_INFO_ID, m_nCurrOffset);
      }
      else if(!m_pTrackEntry)
      {
        bIdFound = GetOffsetFromSeekHead(EBML_TRACKS_ID, m_nCurrOffset);
      }
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "IsMetaDataParsingDone reporting true @offset %llu",
                     m_nCurrOffset);
        bret = true;
      }
    }
    /* Check if Cue elements are present in the clip or not. */
    if(m_pAllCuesInfo == NULL && true == bret)
    {
      bool bCueFound = false;
      bCueFound = GetOffsetFromSeekHead(EBML_CUES_ID, m_nCurrOffset);
      //If Cues are found and calculated offset is within the range
      //Do not parse CUES in streaming case unless complete file is downloaded.
      //Before confirming, validate the offset given by SEEK-HEAD. The data may
      //be corrupted. If it is not valid CUES offset, then reset flag
      if (bCueFound)
      {
        bCueFound = false;
        if ((m_nCurrOffset < m_nFileSize) && (true == m_bEndOfData))
        {
          if(GetDataFromSource(m_nCurrOffset, DEF_EBML_MX_ID_LENGTH_VAL,
                               m_pDataBuffer, DEF_DATA_BUF_SIZE) )
          {
            //! do Byte reversal
            (void)AtomIdBytes(m_pDataBuffer);
            if(!memcmp(m_pDataBuffer, &EBML_CUES_ID, sizeof(EBML_CUES_ID)))
            {
              bCueFound = true;
            }
            else
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                "CUE not available at @offset %llu (Seek Head is corrupted)",
                m_nCurrOffset);
          }
        }
      }//! if (bCueFound)
      if (bCueFound)
      {
        bret = false;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
          "Parse CUE info available at @offset %llu before open_complete",
          m_nCurrOffset);
        m_eParserPrvState = m_eParserState;
        m_eParserState    = MKAVPARSER_PARSE_CUES_HDR;
      }
    }
  }
  /* If seek head is not present, then report true as soon as mandatory
     metadata fields and one cluster is identified. If cues are not found
     before first cluster, then seek will not be supported for such clips. */
  else if((m_bHttpPlay) && (m_pSegmentInfo) && (m_pTrackEntry) &&
          (m_pAllClustersInfo && m_pAllClustersInfo->nClusters))
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "IsMetaDataParsingDone reporting true @offset %llu",
                 m_nCurrOffset);
    bret = true;
  }
  if(m_nCurrOffset == m_nFileSize)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
     "IsMetaDataParsingDone reporting true (m_nCurrOffset == m_nFileSize)%llu",
                 m_nCurrOffset);
    bret = true;
  }
  return bret;
}
/* =============================================================================
FUNCTION:
 MKAVParser::DoWeNeedToParseMoreSeekHead

DESCRIPTION:
Checks if there are any seek heads to be parsed

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns absolute file offset for the next seek head to be parsed else returns 0

SIDE EFFECTS:
  None.
=============================================================================*/
uint64 MKAVParser::DoWeNeedToParseMoreSeekHead()
{
  uint64 nretoffset  = 0;

  if(m_pSeekHeadInfo && m_bHttpPlay)
  {
    for(uint32 j = 0; j < m_pSeekHeadInfo->nSeekHead;j++)
    {
      for(uint32 i = 0; i < m_pSeekHeadInfo->pSeekHead[j].nSeekInfo; i++)
      {
        if(memcmp(&EBML_SEEK_HEAD_ID,
                  &(m_pSeekHeadInfo->pSeekHead[j].pSeekInfo[i].ebml_id),
                  sizeof(EBML_SEEK_HEAD_ID)) == 0)
        {
          //check if we have parsed this SEEK HEAD
          if(m_pSeekHeadInfo->pSeekHead[j].pSeekInfo[i].ebml_id_file_posn > m_nCurrOffset)
          {
            nretoffset = m_pSeekHeadInfo->pSeekHead[j].pSeekInfo[i].ebml_id_file_posn;
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
              "DoWeNeedToParseMoreSeekHead m_nCurrOffset %llu nretoffset %llu",
              m_nCurrOffset, nretoffset);
            break;
          }
        }
      }
    }
  }
  return nretoffset;
}
/* =============================================================================
FUNCTION:
 MKAVParser::UpdateClustersInfoFromSEEKHeads

DESCRIPTION:
Stores the cluster information from parsed seek heads into clusterinfo.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 None

SIDE EFFECTS:
  None.
=============================================================================*/
void  MKAVParser::UpdateClustersInfoFromSEEKHeads()
{
  if(m_pSeekHeadInfo && m_bHttpPlay && m_pSeekHeadInfo->nSeekHead)
  {
    int32 nclusters = 0;
    //first get the count of all the clusters as doing in one pass will cause realloc multiple times.
    for(uint32 j = 0; j < m_pSeekHeadInfo->nSeekHead;j++)
    {
      for(uint32 i = 0; i < m_pSeekHeadInfo->pSeekHead[j].nSeekInfo; i++)
      {
        if(memcmp(&EBML_CLUSTER_ID,
                  &(m_pSeekHeadInfo->pSeekHead[j].pSeekInfo[i].ebml_id),
                  sizeof(EBML_CLUSTER_ID)) == 0)
        {
          nclusters++;
        }
      }//for(uint32 i = 0; i < m_pSeekHeadInfo->pSeekHead[j].nSeekInfo; i++)
    }//for(uint32 j = 0; j < m_pSeekHeadInfo->nSeekHead;j++)

    if(nclusters)
    {
      //if there were few clusters before the first occurence of SEEK HEAD
      //we would have recorded that information.
      //Free up cluster info, if any as it will be populated using SEEK HEADs.
      FreeClustersInfo();
      m_pAllClustersInfo = (all_clusters_info*)MM_Malloc(sizeof(all_clusters_info));
      if(m_pAllClustersInfo)
      {
        memset(m_pAllClustersInfo,0,sizeof(all_clusters_info));
        m_pAllClustersInfo->pClusterInfo = (cluster_info*)MM_Malloc(sizeof(cluster_info)*nclusters);
        if(m_pAllClustersInfo->pClusterInfo)
        {
          memset(m_pAllClustersInfo->pClusterInfo,0,sizeof(cluster_info)*nclusters);
          m_pAllClustersInfo->nClusters = nclusters;
          int nclusterindex = 0;
          for(uint32 j = 0; j < m_pSeekHeadInfo->nSeekHead;j++)
          {
            for(uint32 i = 0; i < m_pSeekHeadInfo->pSeekHead[j].nSeekInfo; i++)
            {
              if(memcmp(&EBML_CLUSTER_ID,
                        &(m_pSeekHeadInfo->pSeekHead[j].pSeekInfo[i].ebml_id),
                        sizeof(EBML_CLUSTER_ID)) == 0)
              {
                m_pAllClustersInfo->pClusterInfo[nclusterindex].nOffset =
                  m_pSeekHeadInfo->pSeekHead[j].pSeekInfo[i].ebml_id_file_posn;
              }
            }//for(uint32 i = 0; i < m_pSeekHeadInfo->pSeekHead[j].nSeekInfo; i++)
          }//for(uint32 j = 0; j < m_pSeekHeadInfo->nSeekHead;j++)
        }//if(m_pAllClustersInfo->pClusterInfo)
      }//if(m_pAllClustersInfo)
    }//if(nclusters)
  }//if(m_pSeekHeadInfo && m_bHttpPlay && m_pSeekHeadInfo->nSeekHead)
}
/* =============================================================================
FUNCTION:
 MKAVParser::FreeClustersInfo

DESCRIPTION:
Frees up cluster info, if it exists

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 None

SIDE EFFECTS:
  None.
=============================================================================*/
void MKAVParser::FreeClustersInfo()
{
  if(m_pAllClustersInfo)
  {
    if(m_pAllClustersInfo->pClusterInfo)
    {
      for(uint32 i = 0; i < m_pAllClustersInfo->nClusters; i++)
      {
        if(m_pAllClustersInfo->pClusterInfo[i].nBlockGroup)
        {
          if(m_pAllClustersInfo->pClusterInfo[i].pBlockGroup)
          {
            if(m_pAllClustersInfo->pClusterInfo[i].pBlockGroup->nBlockinfo)
            {
              MM_Free(m_pAllClustersInfo->pClusterInfo[i].pBlockGroup->pBlockInfo);
              m_pAllClustersInfo->pClusterInfo[i].pBlockGroup->pBlockInfo = NULL;
            }
            MM_Free(m_pAllClustersInfo->pClusterInfo[i].pBlockGroup);
            m_pAllClustersInfo->pClusterInfo[i].pBlockGroup = NULL;
          }
        }
        if(m_pAllClustersInfo->pClusterInfo[i].nSimpleBlocks)
        {
          if(m_pAllClustersInfo->pClusterInfo[i].pSimpleBlocks)
          {
            MM_Free(m_pAllClustersInfo->pClusterInfo[i].pSimpleBlocks);
            m_pAllClustersInfo->pClusterInfo[i].pSimpleBlocks = NULL;
          }
        }
      }//for(uint32 i = 0; i < m_pAllClustersInfo->nClusters; i++)
      MM_Free(m_pAllClustersInfo->pClusterInfo);
      m_pAllClustersInfo->pClusterInfo = NULL;
    }
    MM_Free(m_pAllClustersInfo);
    m_pAllClustersInfo = NULL;
  }
}

/* ============================================================================
  @brief  Reads one Sample/Frame at a time.

  @details    This function is used to read one media sample/frame at a time
              from recently parsed Block/BlockGroup element.
              Parser will support both bitstream and frame modes.
              In frame mode, it will give only one frame, where as in
              bit Stream mode, it give one Access Unit (set of frames).

  @param[in]      trackId              Track Id number.
  @param[in/out]  pucDataBuf           Buffer to read sample.
  @param[in/out]  pulBufferSize        Number of bytes read into buffer.
  @param[in/out]  psampleInfo          Structure to store sample properties.
  @param[in/out]  rbIsFrameAvailable   Flag to indicate whether frame is
                                       available or not.

  @return     MKAV_API_SUCCESS indicating sample read successfully.
              Else, it will report corresponding error.
  @note       BufferSize should be more than maximum frame size value.
============================================================================ */
MKAV_API_STATUS  MKAVParser::ReadFrames(uint32  ulTrackId,
                                        uint8*  pucDataBuf,
                                        uint32* pulBufferSize,
                                        mkav_stream_sample_info *pSampleInfo,
                                        bool&   rbIsFrameAvailable)
{
  MKAV_API_STATUS retError = MKAV_API_FAIL;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "ReadFrame(s) ");
  #endif
  uint64 ullOffset      = 0;
  uint32 ulMaxBufSize   = 0;
  uint32 *pFrameSizeBuf = NULL;
  blockgroup_info *pBlockGroup = m_pCurrCluster->pBlockGroup;
  blockinfo       *pBlockInfo  = m_pCurrCluster->pSimpleBlocks;

  if((!pBlockInfo) && (pBlockGroup) && (pBlockGroup->nBlocksParsed))
  {
    pBlockInfo = &pBlockGroup->pBlockInfo[pBlockGroup->nBlocksParsed - 1];
  }

  /* Check whether all frames are consumed or not.
     Check whether pBlockInfo ptr and other i/p params are valid or not.
     Check whether track number matches or not. */
  if(!pBlockInfo || !pucDataBuf || !pulBufferSize ||
     pBlockInfo->nFramesParsed >= pBlockInfo->nFrames ||
     pBlockInfo->nTrackNumber != ulTrackId)
  {
    rbIsFrameAvailable = false;
    return retError;
  }
  ulMaxBufSize       = *pulBufferSize;
  ullOffset          = pBlockInfo->nDataOffset;
  pFrameSizeBuf      = pBlockInfo->pFramesSize;
  rbIsFrameAvailable = true;

  if((pBlockInfo->nFrames == 1) ||
     (FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM == m_eFrameOutputModeEnum))
  {
    *pulBufferSize = pBlockInfo->nDataSize;
  }
  else
  {
    /* For Fixed lacing, each individual frame size will not be stored
       separately. Total data size need to be divided with total frames
       to calculate individual frame size value. */
    if(NULL == pFrameSizeBuf)
    {
      *pulBufferSize = pBlockInfo->nDataSize / pBlockInfo->nFrames;
      //Calculate individual frame start offset value
      ullOffset     = ullOffset + *pulBufferSize * pBlockInfo->nFramesParsed;
    }
    else
    {
      uint32 nCount = 0;
      //Calculate individual frame start offset value
      while(nCount < pBlockInfo->nFramesParsed)
      {
        ullOffset += pFrameSizeBuf[nCount++];
      }
      *pulBufferSize = pFrameSizeBuf[pBlockInfo->nFramesParsed];
    }
  }

  uint32 ulNALULengthBytes            = 0;
  mkav_media_codec_type  eCodecType   = MKAV_UNKNOWN_CODEC;
  mkav_video_info*       pVideoInfo   = NULL;
  mkav_audio_info*       pAudioInfo   = NULL;
  mkav_encode_info*      pEncodeInfo  = NULL;
  mkav_track_entry_info* pTrackEntry  = NULL;

  //Reset return error with success
  retError = MKAV_API_SUCCESS;

  for(uint32 ulndex = 0; (ulndex < m_nTrackEntry) && (m_pTrackEntry); ulndex++)
  {
   if(m_pTrackEntry[ulndex].TrackNo == ulTrackId)
   {
     pTrackEntry = m_pTrackEntry + ulndex;
     pVideoInfo  = m_pTrackEntry[ulndex].pVideoInfo;
     pAudioInfo  = m_pTrackEntry[ulndex].pAudioInfo;
     pEncodeInfo = m_pTrackEntry[ulndex].pEncodeInfo;
     eCodecType  = m_pTrackEntry[ulndex].CodecType;
     break;
   }
  }//for loop
  if(MKAV_AVC1_VIDEO_CODEC == eCodecType)
  {
    ulNALULengthBytes = ((mkav_avc1_info*)
                    pVideoInfo->pCodecHdr)->NALU_Len_Minus1;
  }//if(codec == MKAV_AVC1_VIDEO_CODEC)
  else if(MKAV_HEVC_VIDEO_CODEC == eCodecType)
  {
    ulNALULengthBytes = pTrackEntry->nNALULenMinusOne;
  }

  //! Some clips does not have Codec config data
  //! In such clips, we need not to replace NAL size with NAL Start codes as
  //! clip itself contains media data with NAL Start codes.
  if(((MKAV_AVC1_VIDEO_CODEC == eCodecType) && (ulNALULengthBytes)) ||
      ((MKAV_HEVC_VIDEO_CODEC == eCodecType) && (ulNALULengthBytes)) ||
     ((pEncodeInfo) &&
      (EBML_CONTENT_COMPRESSION_HEADER_STRIP == pEncodeInfo->eCompAlgo ) &&
      (FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER == m_eHeaderOutputModeEnum)))
  {
    if(!m_pTempBuffer)
    {
      m_pTempBuffer = (uint8*)MM_Malloc(ulMaxBufSize);
      if(m_pTempBuffer)
      {
        m_nTempBuffSize = ulMaxBufSize;
      }
    }
    else if(m_nTempBuffSize < ulMaxBufSize)
    {
      uint8* ptemp = (uint8*)MM_Realloc(m_pTempBuffer,(m_nTempBuffSize * 2));
      if(ptemp)
      {
        m_pTempBuffer = ptemp;
        m_nTempBuffSize *= 2;
      }
    }
  }//if((codec == MKAV_AVC1_VIDEO_CODEC)&&(nalulength))

  ulNALULengthBytes++;

  uint32 ulDataRead = 0;
  //! Ensure input buffer is sufficient to read
  if ((m_nTempBuffSize >= *pulBufferSize) ||
      (ulMaxBufSize >= *pulBufferSize))
  {
    ulDataRead = GetDataFromSource(ullOffset, *pulBufferSize ,
                       ((m_pTempBuffer!=NULL)?m_pTempBuffer:pucDataBuf),
                       ((m_pTempBuffer!=NULL)?m_nTempBuffSize:*pulBufferSize));
  }

  if( *pulBufferSize != ulDataRead )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "GetCurrentSample failed...noffset %llu",
                 pBlockInfo->nDataOffset);
    MapParserStatetoStatus(m_eParserState, retError);
  }
  else if(pTrackEntry)
  {
    /* If Compression type used is header strip, then metadata will contain the
       header bytes. Parser will keep that data as it is before frame start. */
    if((FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER == m_eHeaderOutputModeEnum) &&
       (MKAV_TRACK_TYPE_AUDIO == pTrackEntry->TrackType ) &&
       (pEncodeInfo) && (m_pTempBuffer) )
    {
      uint32 ulDataBufFilledLen   = 0;
      if(pEncodeInfo->eCompAlgo == EBML_CONTENT_COMPRESSION_HEADER_STRIP)
      {
        memcpy(pucDataBuf, pEncodeInfo->pCompSettings,
               pEncodeInfo->ulCompSettingLen);
        ulDataBufFilledLen += pEncodeInfo->ulCompSettingLen;
      }
      (void) memmove (pucDataBuf + ulDataBufFilledLen, m_pTempBuffer,
                      *pulBufferSize);
      *pulBufferSize += pEncodeInfo->ulCompSettingLen;
    }
    uint32 ulFrameDurinMS = (uint32)(pTrackEntry->DefaultDuration / 1000000);
    bool   bIsHdrPrsnt = (FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER ==
                          m_eHeaderOutputModeEnum) ||
                         (!m_pTempBuffer);
    /* If frame duration is not calculated earlier, Calculate the value now.
       This will be calculated with the help of frame data.*/
    if(0 == pTrackEntry->DefaultDuration)
    {
      ulFrameDurinMS = uint32(CalcAudioFrameDuration(pTrackEntry, eCodecType,
                                                     pucDataBuf, bIsHdrPrsnt)
                              / 1000000);
    }
    /* Update sample properties structure. */
    UpdateSampleProperties(ulTrackId, ulFrameDurinMS, *pulBufferSize,
                           pSampleInfo, pBlockInfo, rbIsFrameAvailable);

    /* Update media data with start codes if codec is H264. */
    if(((MKAV_AVC1_VIDEO_CODEC == eCodecType) ||
        (MKAV_HEVC_VIDEO_CODEC == eCodecType)) &&
       (m_pTempBuffer) )
    {
      *pulBufferSize = UpdateAVC1SampleWithStartCode(ulNALULengthBytes,
                                                     ulDataRead,
                                                     pucDataBuf,
                                                     m_pTempBuffer );
      if(0 == *pulBufferSize)
      {
        /* Reset with ulMaxBufSize value and mark as frame not available.
           Through this Parser will parse next frame and provide it to decoder.
        */
        *pulBufferSize     = ulMaxBufSize;
        rbIsFrameAvailable = true;
        retError           = MKAV_API_READ_FAIL;
      }
    }//if((MKAV_AVC1_VIDEO_CODEC == eCodecType) && (m_pTempBuffer) )
  }//else if(m_pTrackEntry)

  if(MKAV_API_SUCCESS == retError)
  {
    /* If mode is set to extract all frames as byte stream, then Parser will
       read all the samples in one attempt and return it. */
    if(FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM == m_eFrameOutputModeEnum)
    {
      pBlockInfo->nFramesParsed = pBlockInfo->nFrames;
    }
    else if(true == rbIsFrameAvailable)
    {
      pBlockInfo->nFramesParsed++;
    }
  }
  return retError;
}

/* ============================================================================
  @brief  Update sample properties.

  @details    This function is used to update sample properties in class var.

  @param[in]      ulTrackId            Track Id number.
  @param[in/out]  ulFrameDurinMS       Each frame duration in milli-sec units.
  @param[in/out]  ulFrameSize          Frame(s) size.
  @param[in/out]  psampleInfo          Structure to store sample properties.
  @param[in/out]  pBlockInfo           Current parsed block Structure .
  @param[in/out]  rbIsFrameAvailable   Flag to indicate whether frame
                                       can be consumed or not.

  @return     None.
  @note       None.
============================================================================ */
void  MKAVParser::UpdateSampleProperties(uint32 ulTrackId,
                                         uint32 ulFrameDurinMS,
                                         uint32 ulFrameSize,
                                         mkav_stream_sample_info *pSampleInfo,
                                         blockinfo *pBlockInfo,
                                         bool      &rbIsFrameAvailable)
{
  uint32 ulIndex = 0;
  //!Update Index to break the loop, if any of pointers are NULL
  if (!pSampleInfo || !pBlockInfo|| !m_pSampleInfo || !ulFrameSize)
  {
    ulIndex = m_nTrackEntry;
  }
  for(; ulIndex < m_nTrackEntry; ulIndex++)
  {
    if(m_pSampleInfo[ulIndex].nTrackNo == ulTrackId)
    {
#ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "trackId %lu cluster.nTimeCode %llu",
        ulTrackId, m_pCurrCluster->nTimeCode);
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "Block TimeCode %u TimeCodeScale from TrackEntry %lu",
        pBlockInfo->nTimeCode,
        (uint32)m_pSampleInfo[ulIndex].nTimeCodeScale);
#endif
      m_pSampleInfo[ulIndex].nsample++;
      m_pSampleInfo[ulIndex].nsize = ulFrameSize;
      if(m_pSampleInfo[ulIndex].nTimeCodeScale)
      {
        m_pSampleInfo[ulIndex].ntime =
          (uint64)( (m_pCurrCluster->nTimeCode + pBlockInfo->nTimeCode)*
          m_pSampleInfo[ulIndex].nTimeCodeScale);
        m_pSampleInfo[ulIndex].ntime /= MKAV_MILLISECONDS_TIMECODE;

        /* If one AU contains multiple frames, then Parser will try to
           calculate each frame duration. Parser will do this only for
           audio tracks of certain codec types. */
        m_pSampleInfo[ulIndex].ntime += ulFrameDurinMS *
                                        pBlockInfo->nFramesParsed;
      }
      else
      {
        if( ((m_pCurrCluster->nTimeCode + pBlockInfo->nTimeCode) <
             m_pSampleInfo[ulIndex].ntime)&&
            (m_pCurrCluster->nTimeCode == 0)
          )
        {
          m_pSampleInfo[ulIndex].ntime +=
            (m_pCurrCluster->nTimeCode + pBlockInfo->nTimeCode);
        }
        else
        {
          m_pSampleInfo[ulIndex].ntime =
            ( m_pCurrCluster->nTimeCode + pBlockInfo->nTimeCode);
        }
#ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                     "id %lu cluster.nTimeCode %llu block TimeCode %d",
                     ulTrackId, m_pCurrCluster->nTimeCode,
                     pBlockInfo->nTimeCode);
#endif
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH," TS= %llu",
          m_pSampleInfo[ulIndex].ntime);
      }
      m_pSampleInfo[ulIndex].noffset = pBlockInfo->nDataOffset;
      memcpy(pSampleInfo, &m_pSampleInfo[ulIndex],
             sizeof(mkav_stream_sample_info));
      // If we are starting after seek, make sure current sample
      // TS >= MinTS updated while seeking..
      if(m_pSampleInfo[ulIndex].bStartAfterSeek)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     " bStartAfterSeek MinTS= %llu",
                     m_pSampleInfo[ulIndex].nMinTSAfterSeek);
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     " bStartAfterSeek Current Sample TS= %llu",
                     m_pSampleInfo[ulIndex].ntime);
        if(m_pSampleInfo[ulIndex].ntime >=
           m_pSampleInfo[ulIndex].nMinTSAfterSeek)
        {
          m_pSampleInfo[ulIndex].bStartAfterSeek = false;
        }
        else
        {
          //drop the current sample and move to next one..
          ulFrameSize        = 0;
          rbIsFrameAvailable = false;
        }
      }
      break;
    }
  }//for(uint32 ulIndex =0; ulIndex < m_nTrackEntry; ulIndex++)
  return;
}

/* ============================================================================
  @brief  Calculate audio frames size in one block.

  @details    This function is used to calculate each individual frames size
              if multiple audio framem are present in one block.

  @param[in]      ucNumFrames          Number of frames.
  @param[in]      ucFlags              Block flag value.
  @param[in]      ullOffset            Sample start offset value.
  @param[in]      ulDataRead           Data read into buffer.
  @param[in]      ullElemSize          Block/simple block element size.
  @param[in]      rulIndex             Amount of data consumed in data buf.
  @param[in]      pulFramesSize        Data buf pointer.

  @return     Status as success or failure.
  @note       None.
============================================================================ */
MKAV_API_STATUS MKAVParser::CalcFrameSizes(blockinfo* pBlock,
                                           uint64     ullOffset,
                                           uint32     ulDataRead,
                                           uint64     ullElemSize,
                                           uint32&    rulIndex)
{
  if (NULL == pBlock)
  {
    return MKAV_API_INVALID_PARAM;
  }

  MKAV_API_STATUS retError = MKAV_API_SUCCESS;
  uint32* pulFramesSize    = pBlock->pFramesSize;
  uint64 ullRelativeFrmSz  = 0;
  uint32 ulBytesToRead     = MAX_ELEMENT_SIZE;
  uint32 ulPrevFramesSize  = 0;
  uint32 ulSkippedDataLen  = 0;
  uint8  nCount            = 0;
  uint8  ucEBMLSizeBytes   = 0;
  uint8   ucNumFrames      = pBlock->nFrames;
  uint8   ucFlags          = pBlock->nFlags;

  //! Free the memory allocated to store previous block size info
  if (pulFramesSize)
  {
    MM_Free(pulFramesSize);
  }
  pulFramesSize = (uint32*)MM_Malloc(sizeof(uint32) * ucNumFrames);
  pBlock->pFramesSize = pulFramesSize;

  /* Run the loop for Number of frames - 1 times.
     Last frame size will not be available, it has to be calculated by taking
     difference between total block size and sum of (N-1) frames size. */
  while(nCount < ucNumFrames - 1 && pulFramesSize)
  {
    /* Read data from file only if all the data is consumed or for the first
       time. */
    if(!ulDataRead ||
      ((ulDataRead - rulIndex) < DEF_EBML_MX_SZ_LENGTH_VAL))
    {
      ullOffset        += rulIndex;
      ulSkippedDataLen += rulIndex;
      rulIndex          = 0;
      ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                     m_pDataBuffer, DEF_DATA_BUF_SIZE);
      if(ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL)
      {
        retError = MKAV_API_READ_FAIL;
        break;
      }
    }
    if( (ucFlags & EBML_LACING_ID)== EBML_LACING_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"EBML_LACING");
      #endif

      pulFramesSize[nCount] = 0;//Reset with ZERO

      ullRelativeFrmSz = AtomSize(m_pDataBuffer + rulIndex, &ucEBMLSizeBytes);

      /* Extract first Frame Size value. This is absolute value. */
      if(0 == nCount)
      {
        pulFramesSize[nCount] = (uint32)ullRelativeFrmSz;
      }
      else
      {
        bool bIsNegativeValue = FALSE;

        if((int64)ullRelativeFrmSz >= VINT_MAX[ucEBMLSizeBytes - 1])
        {
          ullRelativeFrmSz = ullRelativeFrmSz - VINT_MAX[ucEBMLSizeBytes - 1];
          bIsNegativeValue = FALSE;
        }
        else
        {
          ullRelativeFrmSz = VINT_MAX[ucEBMLSizeBytes - 1] - ullRelativeFrmSz;
          bIsNegativeValue = TRUE;
        }

        if(bIsNegativeValue)
        {
          pulFramesSize[nCount] = pulFramesSize[nCount - 1] -
                                  (uint32)ullRelativeFrmSz;
        }
        else
        {
          pulFramesSize[nCount] = pulFramesSize[nCount - 1] +
                                  (uint32)ullRelativeFrmSz;
        }
      }
      rulIndex += ucEBMLSizeBytes;
       MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                    "simpleblock/block info: EBML_LACING #Framesize[%d] = %lu",
                    nCount, pulFramesSize[nCount]);
      ulPrevFramesSize += pulFramesSize[nCount];
      nCount++;
    }//! if( (ucFlags & EBML_LACING_ID)== EBML_LACING_ID)
    else if( (ucFlags & XIPH_LACING_ID) == XIPH_LACING_ID)
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "XIPH_LACING");
      #endif

      pulFramesSize[nCount] = 0;
      while(m_pDataBuffer[rulIndex] == 0xFF)
      {
        pulFramesSize[nCount] += m_pDataBuffer[rulIndex];
        rulIndex++;
      }
      pulFramesSize[nCount] += m_pDataBuffer[rulIndex++];
      ulPrevFramesSize      += pulFramesSize[nCount];
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                   "simpleblock/block info: XIPH_LACING #Framesize[%d] = %lu",
                   nCount, pulFramesSize[nCount]);
      nCount++;
    }//! else if( (ucFlags & XIPH_LACING_ID) == XIPH_LACING_ID)
  }//! while(nCount < ucNumFrames - 1 && pulFramesSize)

  /* Update Total amount of data skipped. */
  rulIndex = rulIndex + ulSkippedDataLen;

  if(pulFramesSize)
  {
    pulFramesSize[ucNumFrames - 1] = (uint32)ullElemSize -
                                      ulPrevFramesSize - rulIndex;
  }
  return retError;
}

/* ============================================================================
  @brief  Parse and stores Tags element details.

  @details    This function is used to parse Tags element.

  @param[in]      ullOffset      Start Offset from which we need to read data.
  @param[in]      ullElementSize Tags element size.

  @return     MKAV_API_SUCCESS indicating parsing done successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
MKAV_API_STATUS  MKAVParser::ParseTagsElement(uint64 ullOffset,
                                                uint64 ullElementSize)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseTagsElement %llu", ullOffset);
  #endif
  uint32 ulIndex       = 0;
  uint32 ulDataRead    = 0;
  uint32 ulBytesToRead = DEF_EBML_MAX_SIGNATURE_LENGTH_VAL;
  uint64 ullEndOffset  = ullOffset + ullElementSize;
  bool noerror         = true;
  if(m_bHttpPlay && !EnsureDataCanBeRead(ullOffset, ullElementSize))
  {
    noerror = false;
    status  = MKAV_API_DATA_UNDERRUN;
  }

  int32 retstat = TagInfoArray.MakeRoomFor(100);
  if(retstat == -1)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
      "Can't allocate memory for initial TagInfoArray" );
    status = MKAV_API_OUT_OF_MEMORY;
    noerror = false;
  }
  while( (ullOffset < ullEndOffset)&& (noerror) )
  {
    ulIndex    = 0;
    ulDataRead = GetDataFromSource(ullOffset, ulBytesToRead,
                                   m_pDataBuffer, DEF_DATA_BUF_SIZE);
    if( ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL)
    {
      status  = MKAV_API_READ_FAIL;
      noerror = false;
    }
    else
    {
      uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
      uint8 ucEBMLSizeBytes = 0;
      uint64 ullSize        = AtomSize(m_pDataBuffer + ucEBMLIdBytes,
                                       &ucEBMLSizeBytes);

      if(!memcmp(&EBML_TAGS_TAG_ID, m_pDataBuffer + ulIndex,
                 sizeof(EBML_TAGS_TAG_ID) ))
      {
        ulIndex = ucEBMLIdBytes + ucEBMLSizeBytes;
        #ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "ParseTagsElement called at offset %llu",
                     ullOffset);
        #endif
        status = ParseTagElement(ullOffset + ulIndex, ullSize);
        if(MKAV_API_SUCCESS != status)
        {
          noerror = false;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                      "ParseTagsElement ParseTagElement failed at offset %llu",
                      ullOffset);
        }
      }
      else
      {
        //update in case of unsupported/unknown atoms
        ulIndex = ucEBMLIdBytes + ucEBMLSizeBytes;
      }
      //Update offset to start of next atom
      ullOffset = ullOffset + ulIndex + ullSize;
    }
  }//while( (ullOffset < ullEndOffset)&& (noerror) )

  /* If Tag parsing is failed due to malloc failure, then mark status as
     success only. As TAGs are not mandatory fields, we can skip TAGS.*/
  if(MKAV_API_OUT_OF_MEMORY == status)
  {
    noerror = true;
  }
  if(noerror)
  {
    status = MKAV_API_SUCCESS;
  }
  return status;
}

/* ============================================================================
  @brief  Parse and stores Tag element details.

  @details    This function is used to parse Tag element.

  @param[in]      ullOffset      Start Offset from which we need to read data.
  @param[in]      ullElementSize Tag element size.

  @return     MKAV_API_SUCCESS indicating parsing done successfully.
              Else, it will report corresponding error.
  @note       None.
============================================================================ */
MKAV_API_STATUS  MKAVParser::ParseTagElement(uint64 ullOffset,
                                             uint64 ullElementSize)
{
  MKAV_API_STATUS status = MKAV_API_FAIL;
  #ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseTagElement %llu",
               ullOffset);
  #endif

  tag_info_type *pTagInfo = NULL;
  uint32 ulIndex          = 0;
  uint32 ulDataRead       = 0;
  uint64 ullEndOffset     = ullOffset + ullElementSize;
  uint32 ulBytesToRead    = uint32(ullEndOffset - ullOffset);
  bool noerror            = true;

  while( (ullOffset < ullEndOffset) && (noerror) )
  {
    bool bNeedAllocation   = true;
    bool bok               = true;
    uint32 ulMaxTargetSize = 0;
    uint8* pMemory         = NULL;

    /* Read data from file only if all the data is consumed or for the first
       time. */
    if(!ulDataRead ||
      (ulDataRead < uint64(ullEndOffset - ullOffset) &&
      (ulDataRead - ulIndex) < DEF_EBML_MX_SZ_LENGTH_VAL))
    {
      ullOffset    += ulIndex;
      ulIndex       = 0;
      ulBytesToRead = uint32(ullEndOffset - ullOffset);
      ulDataRead    = GetDataFromSource(ullOffset, ulBytesToRead,
                                        m_pDataBuffer, DEF_DATA_BUF_SIZE);
      if(ulDataRead < DEF_EBML_MX_SZ_LENGTH_VAL)
      {
        status  = MKAV_API_READ_FAIL;
        noerror = false;
        break;
      }
    }
    else if(ulDataRead == ulIndex)
    {
      status = MKAV_API_SUCCESS;
      break;
    }
    uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer + ulIndex);
    uint8 ucEBMLSizeBytes = 0;
    uint32 ulMatchedId    = 0;
    uint64 ullSize        = AtomSize(m_pDataBuffer + ulIndex + ucEBMLIdBytes,
                                     &ucEBMLSizeBytes);

    /* Check whether complete atom is read into local buffer or not*/
    if(ullSize > (ulDataRead - ulIndex))
    {
      ulDataRead = 0;
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "ParseTagElement: complete atom is not read");
      #endif
      continue;
    }

    //Compare ID against standard indexes that can occur as part of tracks atom
    if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TAGS_SMPLE_TAG_ID,
                    sizeof(EBML_TAGS_SMPLE_TAG_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTagElement EBML_TAGS_SMPLE_TAG_ID");
      #endif
      ulMatchedId = EBML_TAGS_SMPLE_TAG_ID;
      pTagInfo = (tag_info_type *)MM_Malloc(sizeof(tag_info_type));
      if(NULL == pTagInfo)
      {
        status = MKAV_API_OUT_OF_MEMORY;
        break;
      }
      memset(pTagInfo, 0, sizeof(tag_info_type));
      TagInfoArray += pTagInfo;
      m_nTagInfoCount++;
      /* It is a special case. This atom contains child atoms.
         At the end of condition checks, it will skip current atom.
         So to keep the parsing data, subtract with size of data.*/
      ulIndex -= (uint32)ullSize;
      bNeedAllocation = false;
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TAGS_TAG_NAME_ID,
                    sizeof(EBML_TAGS_TAG_NAME_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTagElement EBML_TAGS_TAG_NAME_ID");
      #endif
      ulMatchedId = EBML_TAGS_TAG_NAME_ID;
      if (pTagInfo)
      {
        pTagInfo->ulTagNameLen = (uint32)ullSize + 1;
      }

    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TAGS_TAG_LANG_ID,
                    sizeof(EBML_TAGS_TAG_LANG_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTagElement EBML_TAGS_TAG_LANG_ID");
      #endif
      ulMatchedId = EBML_TAGS_TAG_LANG_ID;
      if (pTagInfo)
      {
        pTagInfo->ulTagLangLen = (uint32)ullSize + 1;
      }
    }
    else if(!memcmp(m_pDataBuffer+2, &EBML_TAGS_TAG_DEFAULT_ID,
                    sizeof(EBML_TAGS_TAG_DEFAULT_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTagElement EBML_TAGS_TAG_DEFAULT_ID");
      #endif
      ulMatchedId     = EBML_TAGS_TAG_DEFAULT_ID;
      ulMaxTargetSize = 1;
      bNeedAllocation = false;
      if(pTagInfo)
      {
        pMemory = (uint8*)&pTagInfo->bTagDefault;
      }
      else
      {
        bok = false;
      }
    }
    else if(!memcmp(m_pDataBuffer + ulIndex, &EBML_TAGS_TAG_STRING_ID,
                    sizeof(EBML_TAGS_TAG_STRING_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTagElement EBML_TAGS_TAG_STRING_ID");
      #endif
      ulMatchedId = EBML_TAGS_TAG_STRING_ID;
      if (pTagInfo)
      {
        pTagInfo->ulTagStringLen = (uint32)ullSize + 1;
      }
    }
    else if(!memcmp(m_pDataBuffer + ulIndex,&EBML_TAGS_TAG_BINARY_ID,
                    sizeof(EBML_TAGS_TAG_BINARY_ID)) )
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "ParseTagElement EBML_TAGS_TAG_BINARY_ID");
      #endif
      ulMatchedId = EBML_TAGS_TAG_BINARY_ID;
      bNeedAllocation = false;
      if (pTagInfo)
      {
        pMemory = (uint8*)&pTagInfo->ulTagBinValue;
      }
      else
      {
        bok = false;
      }
    }
    else
    {
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "unknown/unsupported atom %llu",ullOffset);
      #endif
      bok = false;
    }
    //Update Index to end of atom type and size fields
    ulIndex += ucEBMLIdBytes + ucEBMLSizeBytes;
    if( (bok) && (ulMatchedId != 0) )
    {
      //store read value here
      #ifdef MKAV_PARSER_DEBUG
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "ParseTagElement ebmlSizeBytes %d atom size %llu",
        ucEBMLSizeBytes, ullSize);
      #endif
      if(bNeedAllocation && pTagInfo)
      {
        pMemory = (uint8*)MM_Malloc((size_t)ullSize + 1);
        if(!pMemory)
        {
          status = MKAV_API_OUT_OF_MEMORY;
          break;
        }
        ulMaxTargetSize = (uint32)(ullSize + 1);
        switch(ulMatchedId)
        {
        case EBML_TAGS_TAG_STRING_ID:
          pTagInfo->pTagString = pMemory;
          break;

        case EBML_TAGS_TAG_LANG_ID:
          pTagInfo->pTagLang = pMemory;
          break;

        case EBML_TAGS_TAG_NAME_ID:
          pTagInfo->pTagName = pMemory;
          break;

        default:
          MM_Free(pMemory);
          pMemory = NULL;
          break;
        }
        if(pMemory)
        {
          pMemory[ullSize] = '\0';
        }
      }
      if(pMemory)
      {
        if(ullSize <= (uint64)ulMaxTargetSize)
        {
          memcpy(pMemory,m_pDataBuffer + ulIndex,(size_t)ullSize);
          if(false == bNeedAllocation)
          {
            MKV_REVERSE_ENDIAN((uint8*)pMemory,(size_t)ullSize);
          }
        }
        else
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
            "ParseTagElement failed. ullSize %llu > ulMaxTargetSize %lu",
            ullSize, ulMaxTargetSize);
        }
      }//if(pMemory)
    }//if(bok)
    ulIndex += (uint32)ullSize;
  }//while( (ullOffset < ullEndOffset)&&(noerror) )

  return status;
}

/* ============================================================================
  @brief  SetAudioOutputMode.

  @details    This function is used to Audio o/p mode.

  @param[in]      henum               O/p mode to set.

  @return  FILE_SOURCE_SUCCESS if successful in setting output mode
           else returns FILE_SOURCE_FAIL.
  @note       None.
============================================================================ */
FileSourceStatus MKAVParser::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  /* Default mode is FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM and
     FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER.
     We do not support changing output mode during the playback */
  switch (henum)
  {
  case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
    {
      m_eFrameOutputModeEnum = henum;
      status = FILE_SOURCE_SUCCESS;
    }
    break;
  case FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER:
    if (m_eHeaderOutputModeEnum == FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER)
    {
      m_eHeaderOutputModeEnum = henum;
      status = FILE_SOURCE_SUCCESS;
    }
    break;
  default:
    break;
  }
  return status;
}

/* ============================================================================
  @brief  GetAudioOutputMode.

  @details    This function is used to check Audio o/p mode that is set.

  @param[in]      henum               O/p mode to set.
  @param[in/out]  bret                Bool value to check whether
                                      enum set is same as queried or not.

  @return  FILE_SOURCE_SUCCESS if successful in setting output mode
           else returns FILE_SOURCE_FAIL.
  @note       None.
============================================================================ */
FileSourceStatus MKAVParser::GetAudioOutputMode(bool* bret,
                                                FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  /* Default mode is FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM and
     FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER.
     We do not support changing output mode during the playback */
  switch (henum)
  {
    case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
      if (m_eFrameOutputModeEnum == henum)
      {
        *bret = true;
        status = FILE_SOURCE_SUCCESS;
      }
      break;
      case FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER:
      if (m_eHeaderOutputModeEnum == henum)
      {
        *bret = true;
        status = FILE_SOURCE_SUCCESS;
      }
      break;
    default:
      break;
  }
  return status;
}

/* ============================================================================
  @brief  GetOffsetFromSeekHead.

  @details    This function is used to get offset of any atom from seekHead if
              present.

  @param[in]      ulElementId    Element Id
  @param[in/out]  ullOffset      Element offset.

  @return  true if successful in finding offset
           else returns false.
  @note       None.
============================================================================ */
bool MKAVParser::GetOffsetFromSeekHead(uint32 ulElementId,
                                       uint64 &ullOffset)
{
  uint32 ulSeekHeadCount = 0;
  seekhead *pSeekHead = NULL;
  bool bret = false;
  if(m_pSeekHeadInfo && m_pSeekHeadInfo->pSeekHead)
  {
    pSeekHead = m_pSeekHeadInfo->pSeekHead;

    /* Check if Cue elements are present in the clip or not. */
    while((!bret) && (ulSeekHeadCount < pSeekHead->nSeekInfo))
    {
      seek_info *pSeekInfo = &pSeekHead->pSeekInfo[ulSeekHeadCount];
      if (ulElementId == pSeekInfo->ebml_id)
      {
        ullOffset = pSeekInfo->ebml_id_file_posn;
        bret = true;
#ifdef MKAV_PARSER_DEBUG
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                     "Id %lu available at @offset %llu",
                     ulElementId, ullOffset);
#endif
        break;
      }
      ulSeekHeadCount++;
    }
  }
  return bret;
}

/* ============================================================================
  @brief  ParseNextCluster.

  @details    This function is used to check Audio o/p mode that is set.

  @param[in]      ullClusterOffset    Cluster start offset.
  @param[in/out]  ullClusterSize      Cluster size.

  @return  MKAV_API_SUCCESS if successful in parsing next cluster element
           else returns MKAV_API_FAIL.
  @note       None.
============================================================================ */
MKAV_API_STATUS MKAVParser::ParseNextClusterHeader(uint64 &ullClusterOffset,
                                                   uint64 &ullClusterSize)
{
  MKAV_API_STATUS retError = MKAV_API_SUCCESS;
  /* Parse until one cluster is found. Break the loop if any error comes. */
  do
  {
    uint32 ulBytesToRead = DEF_EBML_MAX_SIGNATURE_LENGTH_VAL;
    uint32 ulDataRead    = GetDataFromSource(ullClusterOffset,
                                             ulBytesToRead,
                                             m_pDataBuffer,
                                             DEF_DATA_BUF_SIZE);
    if( ulDataRead < ulBytesToRead)
    {
      MapParserStatetoStatus(m_eParserState, retError);
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                   "ParseNextCluster failed at noffset %llu, error %d",
                   ullClusterOffset, retError);
      break;
    }
    else
    {
      uint8 ucEBMLIdBytes   = AtomIdBytes(m_pDataBuffer);
      uint8 ucEBMLSizeBytes = 0;
      uint8 ucHdrsize       = 0;
      ullClusterSize = AtomSize(m_pDataBuffer + ucEBMLIdBytes,
                                &ucEBMLSizeBytes);

      ucHdrsize = (uint8)(ucEBMLIdBytes + ucEBMLSizeBytes);
      ullClusterOffset += ucHdrsize;

      /* If element is Cluster, then parse it and update the required
      parameters. If not, then update Offset value to end of the
      current element and parse next element. */
      if(!memcmp(m_pDataBuffer, &EBML_CLUSTER_ID,
                 sizeof(EBML_CLUSTER_ID)))
      {
        retError = ParseClusterElement(ullClusterOffset, ullClusterSize, NULL,
                                       ucHdrsize);
        break;
      }
      else if ((0 == ucHdrsize) || (0 == ullClusterSize))
      {
        retError = MKAV_API_INVALID_PARAM;
        break;
      }
      else
      {
        ullClusterOffset += ullClusterSize;
      }
    }
  }while(MKAV_API_SUCCESS == retError);
  return retError;
}

/* ============================================================================
  @brief  UpdateCodecDetails.

  @details    This function is used to update codec type field for Microsoft
              related codecs.

  @param[in]      None.

  @return  MKAV_API_SUCCESS if successful if update is successful
           else returns MKAV_API_FAIL.
  @note       None.
============================================================================ */
MKAV_API_STATUS MKAVParser::UpdateCodecDetails( )
{
  uint32 ulExtraDataSize = 0;
  if(0 == m_nstreams)
  {
    return MKAV_API_INVALID_PARAM;
  }
  if (m_pSampleInfo)
  {
    MM_Free(m_pSampleInfo);
  }

  m_pSampleInfo = (mkav_stream_sample_info*)
                   MM_Malloc(sizeof(mkav_stream_sample_info) * m_nstreams);
  if(m_pSampleInfo)
  {
    memset(m_pSampleInfo,0,sizeof(mkav_stream_sample_info) * m_nstreams);
  }
  else
  {
    return MKAV_API_OUT_OF_MEMORY;
  }
  for(int i = 0; (i< m_nstreams) && m_pTrackEntry; i++)
  {
    if((m_pSampleInfo) && (m_pSegmentInfo))
    {
      m_pSampleInfo[i].nTrackNo = m_pTrackEntry[i].TrackNo;
      m_pSampleInfo[i].nTimeCodeScale = m_pSegmentInfo->nTimeCodeScale;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "UpdateCodecDetails failed to update nTimeCodeScale");
    }
    //GetCodecHeader will parse codec pvt header found in matroska file format
    //and will repackaged it in a way accepted by downstream/decoder.
    (void)GetCodecHeader((uint32)m_pTrackEntry[i].TrackNo);
    if( (m_pTrackEntry[i].CodecType == MKAV_MSFT_MGR_CODEC) &&
        (m_pTrackEntry[i].pCodecPvt) )
    {
      mkav_vfw_info* vfwinfo = (mkav_vfw_info*)m_pTrackEntry[i].pCodecPvt;

      if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_SPARK_VIDEO,
                 FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_SPARK_VIDEO;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_SORENSON_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_SORENSON_VIDEO;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_H263_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_H263_VIDEO;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_DIV3_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_DIV3_VIDEO;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_DIV4_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_DIV3_VIDEO;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_DIVX_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_DIVX_VIDEO;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_XVID_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_MPEG4_VIDEO;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_MPEG4_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_MPEG4_VIDEO;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_DX40_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_DIVX_VIDEO;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_DX50_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_DIVX50_VIDEO;
      }
      /* For WMV codecs, only extra data available at end of file is sufficient*/
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_WMV3_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_WMV3_VIDEO;
        ulExtraDataSize = vfwinfo->biSize - MSFT_MGR_CODEC_FIXED_HEADER_SIZE;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_WMV2_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_WMV2_VIDEO;
        ulExtraDataSize = vfwinfo->biSize - MSFT_MGR_CODEC_FIXED_HEADER_SIZE;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_WMV1_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_WMV1_VIDEO;
        ulExtraDataSize = vfwinfo->biSize - MSFT_MGR_CODEC_FIXED_HEADER_SIZE;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_WVC1_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_WVC1_VIDEO;
        ulExtraDataSize = vfwinfo->biSize - MSFT_MGR_CODEC_FIXED_HEADER_SIZE;
      }
      else if(!memcmp(&vfwinfo->biCompression, MKAV_VFW_WMVA_VIDEO,
                      FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_WMVA_VIDEO;
        ulExtraDataSize = vfwinfo->biSize - MSFT_MGR_CODEC_FIXED_HEADER_SIZE;
      }
      /* Currently there is no support to play MP42 clips.
         If support is added in future, update codec type accordingly.
      */
      else if(memcmp(&vfwinfo->biCompression, MKAV_VFW_MP42_VIDEO,
                     FOURCC_SIGNATURE_BYTES))
      {
        m_pTrackEntry[i].CodecType = MKAV_UNKNOWN_CODEC;
      }
      if (ulExtraDataSize)
      {
        memmove(m_pTrackEntry[i].pCodecPvt,
                m_pTrackEntry[i].pCodecPvt + MSFT_MGR_CODEC_FIXED_HEADER_SIZE,
                ulExtraDataSize);
        m_pTrackEntry[i].nCodecPvtSize = ulExtraDataSize;
      }
    }
    else if( (MKAV_MSFT_MGR_AUDIO_CODEC == m_pTrackEntry[i].CodecType) &&
             (m_pTrackEntry[i].pCodecPvt) )
    {
      mkav_afw_info *pafwinfo   = (mkav_afw_info*)m_pTrackEntry[i].pCodecPvt;
      mkav_audio_info*pAudioInfo= m_pTrackEntry[i].pAudioInfo;

      ulExtraDataSize = pafwinfo->cbSize;

      if (pAudioInfo)
      {
        pAudioInfo->SamplingFrequency = pafwinfo->nSamplesPerSec;
        pAudioInfo->ullNumChannels    = pafwinfo->nChannels;
        pAudioInfo->ullBitsPerSample  = pafwinfo->wBitsPerSample;
        pAudioInfo->usBlockAlign      = pafwinfo->nBlockAlign;
        pAudioInfo->ulBitRate         = pafwinfo->nAvgBytesPerSec * 8;
        pAudioInfo->usFormatTag = pafwinfo->wFormatTag;

        if(MKAV_WAVE_FORMAT_MSAUDIO1 == pafwinfo->wFormatTag)
        {
          m_pTrackEntry[i].CodecType = MKAV_WMA_AUDIO;
        }
        else if(MKAV_WAVE_FORMAT_MSAUDIO2 == pafwinfo->wFormatTag)
        {
          mkav_afw_ms2_audio_info *pMS2AudioInfo =
            (mkav_afw_ms2_audio_info*)((uint8*)m_pTrackEntry[i].pCodecPvt +
                                       WAVEFORMATEX_SIZE);
          m_pTrackEntry[i].CodecType = MKAV_WMA_AUDIO;

          pAudioInfo->ullBitsPerSample = 16;
          if (pAudioInfo->ullNumChannels == 2)
          {
            pAudioInfo->ullChannelMask = 3;
          }
          else if (pAudioInfo->ullNumChannels == 1)
          {
            pAudioInfo->ullChannelMask = 4;
          }
          pAudioInfo->ulSamplesPerBlock = pMS2AudioInfo->ulSamplesPerBlock;
          pAudioInfo->usEncoderOptions  = pMS2AudioInfo->usEncodeOptions;
        }
        else if(MKAV_WAVE_FORMAT_MSAUDIO3 == pafwinfo->wFormatTag)
        {
          mkav_afw_wm3_audio_info *pMS3AudioInfo =
            (mkav_afw_wm3_audio_info*)((uint8*)m_pTrackEntry[i].pCodecPvt +
                                       WAVEFORMATEX_SIZE);
          m_pTrackEntry[i].CodecType   = MKAV_WMA_PRO_AUDIO;
          pAudioInfo->ullBitsPerSample = pMS3AudioInfo->usValidBitsPerSample;
          pAudioInfo->ullChannelMask   = pMS3AudioInfo->ulChannelMask;
          pAudioInfo->usEncoderOptions = pMS3AudioInfo->usEncodeOptions;
          pAudioInfo->usAdvEncoderOptions  =
                    pMS3AudioInfo->usAdvancedEncodeOptions;
          pAudioInfo->ulAdvEncoderOptions2 =
                    pMS3AudioInfo->ulAdvancedEncodeOptions2;
        }
        else if(MKAV_WAVE_FORMAT_MSAUDIO4 == pafwinfo->wFormatTag)
        {
          m_pTrackEntry[i].CodecType = MKAV_WM_LOSSLESS;
        }
        else if(MKAV_WAVE_FORMAT_MSSPEECH == pafwinfo->wFormatTag)
        {
          m_pTrackEntry[i].CodecType = MKAV_WM_SPEECH;

          pAudioInfo->ullBitsPerSample = 16;
          if (pAudioInfo->ullNumChannels  == 2)
          {
            pAudioInfo->ullChannelMask = 3;
          }
          else if (pAudioInfo->ullNumChannels  == 1)
          {
            pAudioInfo->ullChannelMask = 4;
          }
        }
        if (ulExtraDataSize)
        {
          memmove(m_pTrackEntry[i].pCodecPvt,
                  m_pTrackEntry[i].pCodecPvt + WAVEFORMATEX_SIZE,
                  ulExtraDataSize);
          m_pTrackEntry[i].nCodecPvtSize = ulExtraDataSize;
        }
        else
        {
          MM_Free(m_pTrackEntry[i].pCodecPvt);
          m_pTrackEntry[i].pCodecPvt = NULL;
          m_pTrackEntry[i].nCodecPvtSize = 0;
        }
      }
    }
  }
  if(m_pAllClustersInfo && m_pAllClustersInfo->nClusters)
  {
    m_nCurrOffset = m_pAllClustersInfo->pClusterInfo->nOffset;
  }
  return MKAV_API_SUCCESS;
}

/* ============================================================================
  @brief  getBufferedDuration.

  @details    This function is used to calculate the playback time based on the
              given Offset value.

  @param[in]      ulTrackId           Track Id.
  @param[in]      sllAvailBytes       Available offset.
  @param[in/out]  pullBufferedTime    Playback Time.

  @return  "TRUE" if successful in calculating the approximate playback time
           else returns "FALSE".
  @note       None.
=============================================================================*/
bool MKAVParser::getBufferedDuration(uint32  /*ulTrackId*/,
                                     int64   sllAvailBytes,
                                     uint64* pullBufferedTime)
{
  bool bRet        = false;
  uint64 ullOffset = (uint64)sllAvailBytes;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "MKAVParser::getBufferedDuration");
  /************************************************************************/
  /* MKAV Parser parses each cluster properties and estimates the time    */
  /* Each cluster has Time code. By taking difference b/w first and last  */
  /* clusters, Parser will get absolute value. For residual data, Parser  */
  /* will estimate the duration by using previous cluster duration value. */

  /* If at least two cluster properties are not available, then by using  */
  /* Segment Duration and Segment Size fields Parser will estimate the    */
  /* approximate duration value.                                          */
  /************************************************************************/
  if(m_bHttpPlay && pullBufferedTime)
  {
    //Reset output variable
    *pullBufferedTime = 0;

    uint64 ullClusterOffset     = 0;
    uint64 ullClusterTime       = 0;
    uint64 ullFirstClusterTime  = 0;
    uint64 ulPrevClusterTime    = 0;
    uint64 ullClusterSize       = 0;
    uint32 ulCount              = 0;
    cluster_info* pClusterInfo  = NULL;
    //If Downloaded data offset value is not available
    if (-1 == sllAvailBytes )
    {
      (void)MKAVCheckAvailableData((uint64*)&ullOffset, &m_bEndOfData,
                                   m_pUserData);
    }

    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                 "Downloaded data offset %llu", ullOffset);
    if ((m_pAllClustersInfo) && (m_pAllClustersInfo->pClusterInfo))
    {
      pClusterInfo = m_pAllClustersInfo->pClusterInfo;
      ullFirstClusterTime = pClusterInfo[0].nTimeCode;

      // Go to the cluster whose Offset is more than or equal to given Offset
      // If Last Cluster is reached, exit the loop
      while((pClusterInfo[ulCount].nOffset <= ullOffset) &&
            ((1 + ulCount++) < m_pAllClustersInfo->nClusters));
      ullClusterOffset = pClusterInfo[ulCount - 1].nOffset;
      ullClusterSize   = pClusterInfo[ulCount - 1].nSize;

      /* If all the clusters available are parsed and requested data offset is
         more than the current cluster.
         If Available Offset is equal to file size, then we will not read next
         cluster properties. Both file size and Available offset are same, then
         it indicates download component is not able to provide actually
         downloaded data.
         If Last Cluster is reached, exit the loop. */
      if (((ullClusterOffset + ullClusterSize) < ullOffset) &&
           (ullOffset != m_nFileSize) )
      {
        //Go to end of the current Cluster
        ullClusterOffset += ullClusterSize;
        /* Parse all the cluster's header till "nOffset" value reaches */
        do
        {
          // Parse Next Cluster atom properties.
          MKAV_API_STATUS retError = ParseNextClusterHeader(ullClusterOffset,
                                                            ullClusterSize);
          //Class variable may be reallocated
          pClusterInfo = m_pAllClustersInfo->pClusterInfo;
          if (MKAV_API_SUCCESS != retError)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                         "ParseNextClusterHeader is failed %x", retError);
            break;
          }
          else
          {
            ulCount++;
            //Go to end of the current Cluster
            ullClusterOffset += ullClusterSize;
          }
        } while(ullClusterOffset < ullOffset);
        //ulCount = m_pAllClustersInfo->nClusters;
      }

      /* At least two Clusters need to be parsed to estimate duration. */
      if (ulCount > 1)
      {
        ullClusterTime    = pClusterInfo[ulCount - 1].nTimeCode;
        ulPrevClusterTime = pClusterInfo[ulCount - 2].nTimeCode;
        ullClusterSize    = pClusterInfo[ulCount - 2].nSize;

        //Duration till last cluster
        *pullBufferedTime = ullClusterTime - ullFirstClusterTime;

        uint64 ullResidualOffset = ullOffset -
                                   pClusterInfo[ulCount - 1].nOffset;

        /* Use previous cluster duration to estimate duration for residual
           data available.  */
        *pullBufferedTime += ((ullClusterTime - ulPrevClusterTime) *
                              (ullResidualOffset) ) / ullClusterSize;
        bRet = true;
      }
      else if ((m_pSegmentElementInfo) && (m_pSegmentInfo) &&
               (m_pSegmentInfo->nDuration))
      {
        //Discard Metadata worth of data from Segment Size
        uint64 ullMediaDataSize = m_pSegmentElementInfo->nDataSize -
                                  pClusterInfo[0].nOffset;

        *pullBufferedTime = (uint64)m_pSegmentInfo->nDuration;
        /* Approximate the playback duration for downloaded data based on
           Segment Size and Duration */
        if (ullOffset != m_nFileSize)
        {
          *pullBufferedTime = (uint64)((ullOffset * m_pSegmentInfo->nDuration)/
                                       ullMediaDataSize);
        }
        bRet = true;
      }
    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "MKAVParser::getBufferedDuration for offset %llu is %llu",
                 ullOffset, *pullBufferedTime);
  }
  return bRet;
}

/* ============================================================================
  @brief  GetOffsetForTime.

  @details    This function is used to calculate the approximate offset value
              based on the given Playback timestamp value.

  @param[in]      ullPBTime           Given Playback Time.
  @param[in/out]  pullFileOffset      Parameter to store o/p Offset Value.
  @param[in]      ulTrackId           Track Id.
  @param[in]      ullCurPosTimeStamp  Current Playback Time.
  @param[in]      reposTime           Reposition Time.

  @return  "TRUE" if successful in calculating the approximate offset value
           else returns "FALSE".
  @note       None.
=============================================================================*/
bool MKAVParser::GetOffsetForTime(uint64  ullPBTime,
                                  uint64* pullFileOffset,
                                  uint32  /*ulTrackID*/,
                                  uint64  /*ullCurPosTimeStamp*/,
                                  uint64& /*reposTime*/)
{
  bool bRet        = false;
  uint64 ullOffset = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "MKAVParser::GetOffsetForTime");
  /************************************************************************/
  /* MKAV Parser parses each cluster properties and estimates the time    */

  /* Each cluster has Time code. Parser extracts at least two clusters.   */
  /* Parser will get absolute offset value for complete clusters. For     */
  /* residual time stamp, Parser will estimate the Offset value by using  */
  /* Previous cluster duration and size parameters.                       */

  /* If at least two cluster properties are not available, then by using  */
  /* Segment Duration and Segment Size fields Parser will estimate the    */
  /* approximate offset value for given playback time.                  */
  /************************************************************************/
  if(m_bHttpPlay && pullFileOffset)
  {
    //Reset output variable
    *pullFileOffset = 0;

    uint64 ullClusterOffset     = 0;
    uint64 ullClusterTime       = 0;
    uint64 ullFirstClusterTime  = 0;
    uint64 ulPrevClusterTime    = 0;
    uint64 ullClusterSize       = 0;
    uint32 ulCount              = 0;
    cluster_info* pClusterInfo  = NULL;

    (void)MKAVCheckAvailableData((uint64*)&ullOffset, &m_bEndOfData,
                                  m_pUserData);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                 "Downloaded data offset %llu", ullOffset);

    if ((m_pAllClustersInfo) && (m_pAllClustersInfo->pClusterInfo))
    {
      pClusterInfo = m_pAllClustersInfo->pClusterInfo;
      ullFirstClusterTime = pClusterInfo[0].nTimeCode;

      /* Go to nearest cluster whose timestamp is more than or equal to
         Playback Time given. */
      while((pClusterInfo[ulCount].nTimeCode <= ullPBTime) &&
            ((1 + ulCount++) < m_pAllClustersInfo->nClusters));

      ullClusterOffset = pClusterInfo[ulCount - 1].nOffset;
      ullClusterSize   = pClusterInfo[ulCount - 1].nSize;

      /* If all the clusters available are parsed and requested data offset is
         more than the current cluster.
         If Available Offset is equal to file size, then we will not read next
         cluster properties. Both file size and Available offset are same, then
         it indicates download component is not able to provide actually
         downloaded data.
         If Last Cluster is reached, exit the loop. */
      if (((ullClusterOffset + ullClusterSize) < ullOffset) &&
          (ullOffset != m_nFileSize) &&
          (pClusterInfo[ulCount].nTimeCode < ullPBTime))
      {
        //Go to end of the current Cluster
        ullClusterOffset += ullClusterSize;
        /* Parse all the cluster's header till "nOffset" value reaches */
        do
        {
          // Parse Next Cluster atom properties.
          MKAV_API_STATUS retError = ParseNextClusterHeader(ullClusterOffset,
                                                            ullClusterSize);
          //Class variable may be reallocated
          pClusterInfo = m_pAllClustersInfo->pClusterInfo;
          if (MKAV_API_SUCCESS != retError)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                         "ParseNextClusterHeader is failed %x", retError);
            break;
          }
          else
          {
            //Go to end of the current Cluster
            ullClusterOffset += ullClusterSize;
          }
        } while(pClusterInfo[ulCount++].nTimeCode < ullPBTime);
      }

      /* At least two Clusters need to be parsed to estimate Offset. */
      if (ulCount > 1)
      {
        ullClusterTime    = pClusterInfo[ulCount - 1].nTimeCode;
        ulPrevClusterTime = pClusterInfo[ulCount - 2].nTimeCode;
        ullClusterSize    = pClusterInfo[ulCount - 2].nSize;

        /* File Offset till Last cluster. Last cluster offset points to start
           of the cluster. Always use (n-1)th cluster as reference.*/
        *pullFileOffset = pClusterInfo[ulCount - 2].nOffset;

        /* Residual Timestamp, after the last cluster. */
        uint64 ullResidualTime = ullPBTime - ulPrevClusterTime;

        /* Use previous cluster duration to estimate duration for residual
           data available.
           Offset = (Residual_time / Cluster_Dur) * (ClusterSize)*/
        *pullFileOffset += (ullClusterSize * ullResidualTime) /
                           (ullClusterTime - ulPrevClusterTime);
        bRet = true;
      }
      else if ((m_pSegmentElementInfo) && (m_pSegmentInfo) &&
               (m_pSegmentInfo->nDuration))
      {
        //Discard Metadata worth of data from Segment Size
        uint64 ullMediaDataSize = m_pSegmentElementInfo->nDataSize -
                                  pClusterInfo[0].nOffset;

        /* Approximate the playback duration for downloaded data based on
           Segment Size and Duration. Add First Cluster Offset to the output
           calculated value. */
        *pullFileOffset = (uint64)((ullPBTime * ullMediaDataSize) /
                                   m_pSegmentInfo->nDuration) +
                                   pClusterInfo[0].nOffset;
        bRet = true;
      }
    }
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "MKAVParser::GetOffsetForTime for time %llu is %llu",
                 ullPBTime, *pullFileOffset);
  }
  return bRet;
}

/*! ======================================================================
@brief  Repositions given track to ZERO

@detail  Update track position to start of the file

@param[in]
 pSampleInfo: Sample Info to be filled in
 trackid:     Identifies the track to be repositioned.

@return    MKAV_API_SUCCESS if successful other wise returns MKAV_API_FAIL
@note      None.
========================================================================== */
MKAV_API_STATUS MKAVParser::SeekToZERO(mkav_stream_sample_info *pSampleInfo,
                                       uint32 ulTrackID)
{
  MKAV_API_STATUS retError = MKAV_API_FAIL;

  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "MKAVParser::Seek Seeking to the begining...");
  if(m_pAllClustersInfo && m_pAllClustersInfo->nClusters)
  {
    for(uint32 ulTrkCnt = 0; ulTrkCnt< m_nstreams;ulTrkCnt++)
    {
      if(m_pSampleInfo[ulTrkCnt].nTrackNo == ulTrackID)
      {
        //Update sample info with first cluster properties
        retError       = MKAV_API_SUCCESS;
        m_nCurrOffset  = m_pAllClustersInfo->pClusterInfo->nOffset;
        m_nCurrCluster = 0;
        m_pSampleInfo[ulTrkCnt].bsync   = true;
        m_pSampleInfo[ulTrkCnt].noffset = m_nCurrOffset;
        m_pSampleInfo[ulTrkCnt].nsample = 1;
        m_pSampleInfo[ulTrkCnt].ntime = m_pSampleInfo[ulTrkCnt].nFirstSampleTS;
        memcpy(pSampleInfo, m_pSampleInfo + ulTrkCnt,
               sizeof(mkav_stream_sample_info));
        ResetCurrentClusterInfo();
        if(m_pCurrCluster)
        {
          memset(m_pCurrCluster,0,sizeof(cluster_info));
        }
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "MKAVParser::Seek Seeking to the begining successful TIME %llu ",
          m_pSampleInfo[ulTrkCnt].ntime);
        break;
      }//if(m_pSampleInfo[i].nTrackNo == trackid)
    }//for(uint32 i = 0; i< m_nstreams;i++)
  }//if(m_pAllClustersInfo && m_pAllClustersInfo->nClusters)
  return retError;
}

/*! ======================================================================
@brief  Updates the Sample Info properties

@detail  Updates the sample info properties with given seek point

@param[in]
 ulTrackID:     Identifies the track to be repositioned.
 pSampleInfo:   Sample Info to be updated
 pCuePointInfo: Cue point info structure

@return    MKAV_API_SUCCESS if successful other wise returns MKAV_API_FAIL
@note      None.
========================================================================== */
MKAV_API_STATUS MKAVParser::UpdateSeekSampleProperties(
                                     uint32 ulTrackID,
                                     mkav_stream_sample_info* pSampleInfo,
                                     cue_point_info* pCuePointInfo)
{
  MKAV_API_STATUS retError = MKAV_API_INVALID_PARAM;
  mkav_track_type eTrackType = GetTrackType(ulTrackID);
  uint32 ulPosCounter = 0;
  cue_track_posn_info* pCueTrackPosnInfo = NULL;
  if ((NULL == pCuePointInfo) || (NULL == pSampleInfo))
  {
    return retError;
  }
  /* If Track Pos Info structure is NULL, then update Counter value to break
     the for loop directly. */
  pCueTrackPosnInfo = pCuePointInfo->pCueTrackPosnInfo;
  if (NULL == pCueTrackPosnInfo)
  {
    ulPosCounter = (uint32)pCuePointInfo->nCueTrackPosnInfo + 1;
  }

  for(; (ulPosCounter < pCuePointInfo->nCueTrackPosnInfo);ulPosCounter++)
  {
    /* Cue point may or may not have entry for audio.In such case
       seeking to cluster with TS >= SeekTime is much faster as
       opposed to scanning each cluster. */
    if( (pCueTrackPosnInfo[ulPosCounter].nCueTrack == ulTrackID) ||
        (eTrackType == MKAV_TRACK_TYPE_AUDIO) )
    {
      uint32 nCluster   = 0;
      uint8  nhdr       = 0;
      uint32 ulTrkCount = 0;
      bool bmapcluster = MapFileOffsetToCluster(
                               pCueTrackPosnInfo[ulPosCounter].nCueClusterPosn,
                               &nCluster, &nhdr);
      for(ulTrkCount = 0; ulTrkCount< m_nstreams; ulTrkCount++)
      {
        if((m_pSampleInfo[ulTrkCount].nTrackNo == ulTrackID)&& (bmapcluster))
        {
          m_nCurrOffset  = pCueTrackPosnInfo[ulPosCounter].nCueClusterPosn;
          m_nCurrOffset += nhdr;
          m_nCurrCluster = nCluster;
          m_pSampleInfo[ulTrkCount].bsync   = true;
          m_pSampleInfo[ulTrkCount].noffset = m_nCurrOffset;
          m_pSampleInfo[ulTrkCount].nsample = 1;
          m_pSampleInfo[ulTrkCount].ntime   = pCuePointInfo->nCueTime;
          memcpy(pSampleInfo, m_pSampleInfo + ulTrkCount,
                 sizeof(mkav_stream_sample_info));
          retError = MKAV_API_SUCCESS;
          m_pSampleInfo[ulTrkCount].bStartAfterSeek = true;
          m_pSampleInfo[ulTrkCount].nMinTSAfterSeek =
                              m_pSampleInfo[ulTrkCount].ntime;
          ResetCurrentClusterInfo();
          if(m_pCurrCluster)
          {
            memset(m_pCurrCluster,0,sizeof(cluster_info));
          }
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "MKAVParser::Seek Seeking to the sample @ TIME %llu ",
            m_pSampleInfo[ulTrkCount].ntime);
          break;
        }//if(m_pSampleInfo[i].nTrackNo == ulTrackID)
      }//SampleInfo counter loop
    }
  }//CueTrackPosn counter loop
  return retError;
}

/*! ======================================================================
@brief  Updates the Sample Offset by using block num info

@detail  Updates the offset value near to the block number mentioned in
         Cue point structure.

@param[in]
pCuePoint:      Cue point info structure
ulTrackID:      Identifies the track to be repositioned.

@return    true if successful other wise returns false.
@note      None.
========================================================================== */
bool MKAVParser::UpdateSampleOffset(cue_point_info* pCuePoint,
                                    uint32 ulTrackId)
{
  uint32 ulBlockNum   = 0;
  uint32 ulPosCounter = 0;
  uint32 nCluster     = 0;
  uint8  ucBuf[MAX_META_STR_SIZE];
  uint8  nhdr         = 0;
  bool   bStatus      = false;
  cue_track_posn_info* pCuePosnInfo = NULL;
  mkav_track_type eTrackType = GetTrackType(ulTrackId);
  if ((!pCuePoint) || (!pCuePoint->pCueTrackPosnInfo))
  {
    return bStatus;
  }
  pCuePosnInfo = pCuePoint->pCueTrackPosnInfo;
  for(; (ulPosCounter < pCuePoint->nCueTrackPosnInfo); ulPosCounter++)
  {
    if((pCuePosnInfo[ulPosCounter].nCueTrack != ulTrackId) &&
       (MKAV_TRACK_TYPE_AUDIO != eTrackType))
    {
      continue;
    }
    bStatus =MapFileOffsetToCluster(pCuePosnInfo[ulPosCounter].nCueClusterPosn,
                                    &nCluster, &nhdr);
    break;
  }//! for loop

  ulBlockNum = (uint32)pCuePosnInfo->nCueBlockNumber;
  if (bStatus)
  {
    uint32 ulBlkCounter = 0;
    uint32 ulDataRead   = 0;
    uint64 ullOffset    = pCuePosnInfo[ulPosCounter].nCueClusterPosn + nhdr;
    uint32 ulBufSize    = 12;
    do
    {
      ulDataRead = GetDataFromSource(ullOffset, ulBufSize, ucBuf, ulBufSize);
      if (ulDataRead != ulBufSize)
      {
        break;
      }
      uint8 ucEBMLIdBytes   = AtomIdBytes(ucBuf);
      uint8 ucEBMLSizeBytes = 0;
      uint64 ullSize        = AtomSize(ucBuf + ucEBMLIdBytes,
                                       &ucEBMLSizeBytes);
      ullOffset += (ucEBMLIdBytes + ucEBMLSizeBytes);
      /* If block type is either simple block or block (in block group),
         then increment block counter. */
      if((!memcmp(ucBuf , &EBML_CLUSTER_SIMPLE_BLOCK_ID,
                  sizeof(EBML_CLUSTER_SIMPLE_BLOCK_ID))) ||
         (!memcmp(ucBuf , &EBML_CLUSTER_BLOCK_ID,
                  sizeof(EBML_CLUSTER_BLOCK_ID))) )
      {
        ulBlkCounter++;
        ullOffset += ullSize;
      }
      /* If element is other than Block Group, then skip those. If element
         is block group, then do not skip the atom. Block group internally
         will contain blocks. Parser needs to count blocks inside block group*/
      else if(memcmp(ucBuf , &EBML_CLUSTER_BLOCKGROUP_ID,
                      sizeof(EBML_CLUSTER_BLOCKGROUP_ID)))
      {
        ullOffset += ullSize;
      }
    } while (ulBlkCounter < ulBlockNum);
    pCuePosnInfo[ulPosCounter].nCueClusterPosn = ullOffset - nhdr;
  }//! if (bStatus)
  return bStatus;
}

/*! ======================================================================
@brief  Seek to the sync sample using Dynamic seek table.

@detail  If seek table is filled then by using seek table, Parser seeks to
         the closest sync sample. If seek table is not filled or entries in
         seek table are far apart from the desired timestamp, then Parser
         searches clusters linearly and seeks to the closest sync sample.

@param[in]
 nReposTime:    Reposition timestamp value.
 ulTrackID:     Identifies the track to be repositioned.
 bForward:      Seek direction
 pSampleInfo:   Sample Info to be updated

@return    MKAV_API_SUCCESS if successful other wise returns error
@note      None.
========================================================================== */
MKAV_API_STATUS MKAVParser::SeekUsingDynamicSeekTable(uint64 nReposTime,
                                                      uint32 ulTrackId,
                                                      bool bForward,
                                                      bool bIsClosestSeek,
                                          mkav_stream_sample_info *pSampleInfo)
{
  MKAV_API_STATUS retError = MKAV_API_FAIL;
  uint32 ulIndex           = m_SeekTableInfo.ulLowerBound;
  uint64 ullOffset         = 0;
  uint64 ullTimeStamp      = 0;
  uint64 ullPrevTime       = 0;
  uint64 ullPrevOffset     = 0;
  bool   bValidEntry       = false;
  bool   bTwoEntries       = false;
  seek_table_entry* pSeekTableEntries = m_SeekTableInfo.pSeekTableEntries;

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "MKAVParser::Seek to TIME %llu using Seek Table",
               nReposTime);
  for (; ulIndex <= m_SeekTableInfo.ulUpperBound; ulIndex++)
  {
    //! Break the loop, if at least one entry is not available
    if ((uint32)-1 == m_SeekTableInfo.ulLowerBound)
    {
      break;
    }
    //! Offset value will never be ZERO in valid case
    if (!pSeekTableEntries[ulIndex].ullOffset)
    {
      continue;
    }
    bValidEntry   = true;
    ullPrevOffset = ullOffset;
    ullPrevTime   = ullTimeStamp;

    ullOffset    = pSeekTableEntries[ulIndex].ullOffset;
    ullTimeStamp = pSeekTableEntries[ulIndex].ullTime;
    //! Make sure both current and previous frames have valid data
    if ((ullTimeStamp  >= nReposTime) &&
        (ullPrevOffset >= m_pAllClustersInfo->pClusterInfo[0].nOffset))
    {
      bTwoEntries = true;
      retError    = MKAV_API_SUCCESS;
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "MKAVParser::Seek prev, next closest frames time %llu, %llu",
              ullPrevTime, ullTimeStamp);
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "MKAVParser::Seek prev, next closest frames offset %llu, %llu",
            ullPrevOffset, ullOffset);
      break;
    }
  }
  //! If two entries are found, check if both are sync samples or not
  if (bTwoEntries)
  {
    //! If both are sync samples, then check if they are close to seek time
    if((pSeekTableEntries[ulIndex].ucIsKeyFrame) &&
       (pSeekTableEntries[ulIndex - 1].ucIsKeyFrame))
    {
      uint64 ullFwdSyncFrameDelta = pSeekTableEntries[ulIndex].ullTime -
                                    nReposTime;
      uint64 ullRwdSyncFrameDelta = nReposTime -
                                    pSeekTableEntries[ulIndex - 1].ullTime;
      /* If either of forward or backward seek entries are closer to
         seek time requested */
      if ((ullFwdSyncFrameDelta < SEEK_ENTRY_DELTA * 2) ||
          (ullRwdSyncFrameDelta < SEEK_ENTRY_DELTA * 2))
      {
        uint32 ulClusterNum = 0;
        uint8  ucHdrSize    = 0;
        bool   bStatus      = false;
        //! If closest seek is selected, then check the closer entry
        if ((bIsClosestSeek) &&
            (ullFwdSyncFrameDelta > ullRwdSyncFrameDelta))
        {
          ulIndex--;
        }
        //! If user selects rewind, then seek to previous entry
        if (!bForward)
        {
          ullOffset = ullPrevOffset;
        }
        bStatus = MapFileOffsetToCluster(ullOffset, &ulClusterNum,
                                         &ucHdrSize);
        if (bStatus)
        {
          m_nCurrCluster = ulClusterNum;
          m_nCurrOffset  = ullOffset;
        }
      }//! if ((ullFwdSyncFrameDelta < SEEK_ENTRY_DELTA * 2) || ..)
    }//! if((pSeekTableEntries[ulIndex].ucIsKeyFrame) && ..)
  }//! if (bTwoEntries)
  //! If entries are not available in seek table or the previous entry is far
  //! from requested seek time, then search for nearest cluster.
  if ((!bValidEntry) || ((nReposTime - ullPrevTime) > MAX_SEEK_DELTA))
  {
    retError = FindNearestCluster(ulTrackId, nReposTime, bForward,
                                  &ullPrevOffset);
  }
  //! If the previous entry is close enough, then search linearly
  else
  {
    retError = MKAV_API_SUCCESS;
  }
  if(MKAV_API_SUCCESS == retError)
  {
    retError = FindNearestSyncSample(ulTrackId, ullPrevOffset, nReposTime,
                                     bForward, bIsClosestSeek, pSampleInfo);
  }
  return retError;
}

/* ============================================================================
  @brief  Update Seek Table

  @details        This function is used to update internal seek table.

  @param[in]      pBlockGroup   Block Group Info Structure Ptr.
  @param[in]      pBlockInfo    Block Info Structure Ptr.

  @return     Nothing.
  @note       This function will be called only if CUES are not available.
============================================================================ */
void MKAVParser::UpdateSeekTable(blockgroup_info *pBlockGroup,
                                 blockinfo       *pBlockInfo)
{
  if (pBlockInfo)
  {
    seek_table_entry *pSeekTableEntries = m_SeekTableInfo.pSeekTableEntries;
    uint32 ulEntry      = 0;
    uint64 ullTime      = m_pCurrCluster->nTimeCode + pBlockInfo->nTimeCode;
    uint64 ullOffset    = 0;
    uint8  ucIsKeyFrame = 0;
    if (pBlockGroup)
    {
      ullOffset = pBlockGroup->nBlockStartOffset;
      if (!pBlockGroup->nRefBlock)
      {
        ucIsKeyFrame = 1;
      }
    }
    else
    {
      ullOffset = pBlockInfo->nDataOffset - pBlockInfo->nBlockHeaderSize;
      if (pBlockInfo->nFlags & 0x80)
      {
        ucIsKeyFrame = 1;
      }
    }
    if (m_pSegmentInfo->nTimeCodeScale)
    {
      ullTime  = (ullTime * m_pSegmentInfo->nTimeCodeScale);
      ullTime /= MKAV_MILLISECONDS_TIMECODE;;
    }
    /* Calculate entry offset and store in the seek table, only if its
       TimeStamp is less than the already stored entry or new entry.
       Update the entry, if current sample is sync sample and previously
       stored entry is not sync sample. */
    ulEntry = uint32(ullTime / m_SeekTableInfo.ulDelta);
    //! If the calculated entry crosses total entries, restrict to max value
    if (ulEntry >= m_SeekTableInfo.ulEntries)
    {
      ulEntry = m_SeekTableInfo.ulEntries - 1;
    }
    if ((0 == pSeekTableEntries[ulEntry].ullOffset) ||
        (pSeekTableEntries[ulEntry].ullTime > ullTime) ||
        ((0 == pSeekTableEntries[ulEntry].ucIsKeyFrame) &&
         (ucIsKeyFrame)))
    {
      pSeekTableEntries[ulEntry].ullTime      = ullTime;
      pSeekTableEntries[ulEntry].ullOffset    = ullOffset;
      pSeekTableEntries[ulEntry].ucIsKeyFrame = ucIsKeyFrame;
    }
    if (ulEntry > m_SeekTableInfo.ulUpperBound)
    {
      m_SeekTableInfo.ulUpperBound = ulEntry;
    }
    if (ulEntry < m_SeekTableInfo.ulLowerBound)
    {
      m_SeekTableInfo.ulLowerBound = ulEntry;
    }
  }
}

/* ============================================================================
  @brief  Find Nearest Cluster to the seek time requested.

  @details        This function is used to find the nearest cluster based on
                  the seek time requested.

  @param[in]      ulTrackID       TrackID for which seek request came.
  @param[in]      ullReposTime    Seek time requested.
  @param[in]      bForward        Flag to check whether fwd/backward seek.
  @param[in/out]  pullOffset      Parameter to store nearest cluster offset.

  @return     Nothing.
  @note       This function will be called only if CUES are not available.
============================================================================ */
MKAV_API_STATUS MKAVParser::FindNearestCluster(uint32 ulTrackID,
                                               uint64 ullReposTime,
                                               bool   bForward,
                                               uint64 *pullOffset)
{
  MKAV_API_STATUS retError     = MKAV_API_INVALID_PARAM;
  mkav_media_codec_type eCodec = GetTrackCodecType(ulTrackID);
  uint32 ulClusterIndex        = 0;
  cluster_info *pClusterInfo   = NULL;
  //Validate input params
  if ( (!m_pAllClustersInfo )|| (MKAV_UNKNOWN_CODEC == eCodec) ||
       (!pullOffset) || (!m_pAllClustersInfo->pClusterInfo))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
              "FindNearestCluster failed 'invalid params'");
    return retError;
  }

  pClusterInfo = m_pAllClustersInfo->pClusterInfo;
  /* Check the clusters so far parsed, if cluster with start time more than
     seek requested time. */
  for (; ulClusterIndex < m_pAllClustersInfo->nClusters; ulClusterIndex++)
  {
    if (pClusterInfo[ulClusterIndex].nTimeCode >= ullReposTime)
    {
      break;
    }
  }
  /* If all the clusters parsed whose timestamp is less than seek time
     requested, then parse subsequent cluster headers and try to find the
     nearest cluster. */
  if ((ulClusterIndex == m_pAllClustersInfo->nClusters) ||
      (pClusterInfo[ulClusterIndex].nTimeCode < ullReposTime))
  {
    //Reduce it by 1, to point to the last entry
    ulClusterIndex--;
    uint64 ullClusterSize      = 0;
    uint64 ullNextClusterOfset = pClusterInfo[ulClusterIndex].nOffset +
                                 pClusterInfo[ulClusterIndex].nSize;
    do
    {
      retError = ParseNextClusterHeader(ullNextClusterOfset, ullClusterSize);
      pClusterInfo = m_pAllClustersInfo->pClusterInfo;
      if ((MKAV_API_SUCCESS != retError) || (!pClusterInfo))
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "MKAVParser::FindNearestCluster failed with error %x @ %llu",
                  retError, ullNextClusterOfset);
        break;
      }
      //Increase cluster index to point to the last entry
      ulClusterIndex++;
      ullNextClusterOfset = pClusterInfo[ulClusterIndex].nOffset +
                            ullClusterSize;
    } while(pClusterInfo[ulClusterIndex].nTimeCode <= ullReposTime);
  }
  if (pClusterInfo)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
       "MKAVParser::FindNearestCluster next cluster time and offset %llu, %llu",
       pClusterInfo[ulClusterIndex].nTimeCode,
       pClusterInfo[ulClusterIndex].nOffset);
    //! If Clip has only one cluster, ulClusterIndex will be ZERO
    if(ulClusterIndex)
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
      "MKAVParser::FindNearestCluster prev cluster time and offset %llu, %llu",
       pClusterInfo[ulClusterIndex - 1].nTimeCode,
      pClusterInfo[ulClusterIndex - 1].nOffset);

    /* If Seek request is in forward direction, go to the cluster whose time
       is equal to the seek time requested. */
    if (((bForward) && (ulClusterIndex < m_pAllClustersInfo->nClusters) &&
        (ullReposTime == pClusterInfo[ulClusterIndex].nTimeCode)) ||
        (!ulClusterIndex) )
    {
      *pullOffset = pClusterInfo[ulClusterIndex].nOffset;
    }
    else
    {
      *pullOffset = pClusterInfo[ulClusterIndex - 1].nOffset;
    }
  }

  retError = MKAV_API_SUCCESS;
  return retError;
}

/* ============================================================================
  @brief  Find Nearest sync sample.

  @details        This function is used to find the nearest sync sample based
                  on the seek time requested.

  @param[in]      ulTrackID       TrackID for which seek request came.
  @param[in]      ullPrevOffset   Current cluster offset.
  @param[in]      ullReposTime    Reposition time.
  @param[in]      bForward        Flag to check whether fwd/backward seek.
  @param[in/out]  pSampleInfo     Structure to store sample properties.

  @return     Nothing.
  @note       This function will be called only if CUES are not available.
============================================================================ */
MKAV_API_STATUS MKAVParser::FindNearestSyncSample(uint32 ulTrackId,
                                                  uint64 ullPrevOffset,
                                                  uint64 ullReposTime,
                                                  bool   bForward,
                                                  bool   bIsClosestSeek,
                                          mkav_stream_sample_info* pSampleInfo)
{
  MKAV_API_STATUS retError   = MKAV_API_INVALID_PARAM;
  mkav_track_type eTrackType = GetTrackType(ulTrackId);
  blockinfo      *pBlockInfo = NULL;

  uint64 ullOffset        = 0;
  uint64 ullPrevTime      = 0;
  int32  ulFrameSize      = 0;
  uint64 ullTimeStamp     = 0;
  uint32 ulClusterNum     = 0;
  uint8  ucClusterHdrSize = 0;

  bool bPrevSyncFrm  = false;
  bool bIsCurFrmSync = false;
  bool bStatus = MapFileOffsetToCluster(ullPrevOffset, &ulClusterNum,
                                        &ucClusterHdrSize);
  if (!pSampleInfo)
  {
    return retError;
  }
  else if (false == bStatus)
  {
    MapParserStatetoStatus(m_eParserState, retError);
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                 "FindNearestSyncSample: MapFileOffsetToCluster failed "
                 "with error %x @ offset %llu", retError, ullPrevOffset);
    return retError;
  }
  m_nCurrCluster = ulClusterNum;
  m_nCurrOffset  = ullPrevOffset;
  ResetCurrentClusterInfo();
  if (m_pCurrCluster)
  {
    m_pCurrCluster->nTimeCode =
      m_pAllClustersInfo->pClusterInfo[m_nCurrCluster].nTimeCode;
  }
  do
  {
    /* Maintain previous sync offset details. In case of backward seek, Parser
       will use these details to sync to the sample whose timestamp is less
       thank seek requested time. */
    if (bIsCurFrmSync)
    {
      ullPrevOffset = ullOffset;
      ullPrevTime   = ullTimeStamp;
      bPrevSyncFrm  = true;
    }
    /* During seek, Block properties are sufficient to check whether sample
       is key frame or not. So by passing NULL values to buffer pointer,
       Parser reduces the overhead of extra memcpy. */
    retError = GetCurrentSample(ulTrackId, NULL /* Buf */, 0 /* BufSize */,
                                &ulFrameSize, pSampleInfo);
    if (MKAV_API_SUCCESS != retError)
    {
      MapParserStatetoStatus(m_eParserState, retError);
      break;
    }
    ullTimeStamp = pSampleInfo->ntime;
    /* If Block group is present, then for non-sync samples Reference block
       Timestamp will be present. Use that detail to check whether given frame
       is sync frame or not. */
    if (m_pCurrCluster->pBlockGroup)
    {
      ullOffset     = m_pCurrCluster->pBlockGroup->nBlockStartOffset;
      pBlockInfo    = m_pCurrCluster->pBlockGroup->pBlockInfo;
      bIsCurFrmSync = (m_pCurrCluster->pBlockGroup->nRefBlock)?false:true;
    }
    /* If Simple block is present, then Sample properties flag will indicate
       whether given frame is sync frame or not .
       Sample Properties flag: From MSB
       Bit  0x80 -- KeyFrame or not
       Bits 0x06 -- Lace type
                    00 -- no lacing
                    01 -- Xiph lacing
                    10 -- Fixed size lacing
                    11 -- EBML lacing
      Bit  0x08 -- Invisible
      Bit  0x01 -- Discardable
    */
    else if (m_pCurrCluster->pSimpleBlocks)
    {
      pBlockInfo    = m_pCurrCluster->pSimpleBlocks;
      ullOffset     = pBlockInfo->nDataOffset -
                      pBlockInfo->nBlockHeaderSize;
      bIsCurFrmSync = false;
      if(0x80 == (pBlockInfo->nFlags & 0x80))
        bIsCurFrmSync = true;
      ResetCurrentClusterInfo();
    }
    else
    {
      bIsCurFrmSync = false;
      break;
    }
    //For non-video tracks, all samples will be considered as sync samples
    if (MKAV_TRACK_TYPE_VIDEO != eTrackType)
    {
      bIsCurFrmSync = true;
    }
    //If Sync frame with TS more than Seek-time is found, then break loop
    if (bIsCurFrmSync && ullTimeStamp >= ullReposTime)
    {
      break;
    }
  } while (pSampleInfo);

  /* If closest seek is selected by application, check if both entries are
     present or not. If both are available and Previous entry is closer to the
     requested seek time, then update current entry offset with previous entry
     and mark forward flag as true. In the subsequent condition, it will set
     class variable current offset entry to the previous offset value.
    */
  if ((bIsClosestSeek) && (bIsCurFrmSync) && (bPrevSyncFrm))
  {
    if((ullTimeStamp - ullReposTime) > (ullReposTime - ullPrevTime))
    {
      ullOffset = ullPrevOffset;
      bForward  = true;
    }
  }

  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
      "MKAVParser::FindNearestSyncSample prev and next sync frame "
      "offset values %llu, %llu", ullPrevOffset, ullOffset);
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
      "MKAVParser::FindNearestSyncSample prev and next sync frame "
      "timestamp values %llu, %llu", ullPrevTime, ullTimeStamp);
  /* If forward seek is selected, then check whether forward entry is found
     or not*/
  if ((bForward) && (MKAV_API_SUCCESS == retError) && (bIsCurFrmSync))
  {
    bStatus = MapFileOffsetToCluster(ullOffset, &ulClusterNum,
                                     &ucClusterHdrSize);
    m_nCurrOffset = ullOffset;
  }
  /* If backward seek is selected, then check whether backward entry is found
     or not*/
  else if((true == bPrevSyncFrm) && (!bForward))
  {
    bStatus = MapFileOffsetToCluster(ullPrevOffset, &ulClusterNum,
                                     &ucClusterHdrSize);
    m_nCurrOffset = ullPrevOffset;
  }
  else
  {
    bStatus = false;
  }
  m_nCurrCluster = ulClusterNum;
  if (false == bStatus)
  {
    MapParserStatetoStatus(m_eParserState, retError);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "MapFileOffsetToCluster failed, error %x", retError);
    return retError;
  }
  return retError;
}
#endif //#ifdef FEATURE_FILESOURCE_MKV_PARSER

