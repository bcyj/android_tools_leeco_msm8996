/* =======================================================================
                              TSHeaderParser.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2009-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/src/TSHeaderParser.cpp#86 $
========================================================================== */
#include "SectionHeaderParser.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "MMMalloc.h"
#include "filebase.h"
#include "H264HeaderParser.h"

/* ============================================================================
  @brief      Create MP2 TS Header parsing context.

  @details    This function is used to initialize the context.

  @param[in/out]    pContext        Context pointer.

  @return   true if memory allocated successfully, else false.

  @note       None.
============================================================================ */
bool CreateMP2ParserContext(MP2ParserContext* pContext)
{
  memset(pContext, 0, sizeof(MP2ParserContext));
  //! Allocate memory to store tables
  pContext->pPATData = (SectionData*)MM_Malloc(sizeof(SectionData));
  if (pContext->pPATData)
    memset(pContext->pPATData, 0, sizeof(SectionData));
  pContext->pPMTData = (SectionData*)MM_Malloc(sizeof(SectionData));
  if (pContext->pPMTData)
    memset(pContext->pPMTData, 0, sizeof(SectionData));
  pContext->pCATData = (SectionData*)MM_Malloc(sizeof(SectionData));
  if (pContext->pCATData)
    memset(pContext->pCATData, 0, sizeof(SectionData));
  if ((!pContext->pPATData) || (!pContext->pPMTData) || (!pContext->pCATData))
  {
    return false;
  }
  //! This flag is to indicate parsing is pending
  pContext->bInitialParsingPending = true;

  return true;
}

/* ==========================================================================
  @brief      Free MP2 TS Header parsing context.

  @details    This function is used to de-initialize the context.

  @param[in/out]    pContext        Context pointer.

  @return   true if memory allocated successfully, else false.

  @note       None.
========================================================================== */
void FreeMP2ParserContext(MP2ParserContext* pContext)
{
  //! Need to see, whether read buffer has to be freed as part of this or not

  if (pContext->pPATData)
  {
    MM_Free(pContext->pPATData);
  }
  if (pContext->pPMTData)
  {
    MM_Free(pContext->pPMTData);
  }
  if (pContext->pCATData)
  {
    MM_Free(pContext->pCATData);
  }
  if (pContext->pCATSection)
  {
    FreeDescriptors(pContext->pCATSection->pDescList);
    MM_Free(pContext->pCATSection);
  }
  //! Call function to free the memory allocated to Program Map Section
  freePMT(pContext->pMapSection);
  freePAT(&pContext->sPATSection);

  if (pContext->pAudioStreamIds)
  {
    MM_Free(pContext->pAudioStreamIds);
  }
  if (pContext->pVideoStreamIds)
  {
    MM_Free(pContext->pVideoStreamIds);
  }
  if (pContext->pStreamInfo)
  {
    for(uint16 usIndex = 0; usIndex < pContext->usNumStreams; usIndex++)
    {
      if (pContext->pStreamInfo[usIndex].pCADesc)
      {
        MM_Free(pContext->pStreamInfo[usIndex].pCADesc);
      }
    }
    MM_Free(pContext->pStreamInfo);
  }
  if(pContext->pAVCCodecBuf)
  {
    if(pContext->pAVCCodecBuf->codecInfoBuf)
      MM_Free(pContext->pAVCCodecBuf->codecInfoBuf);
    MM_Free(pContext->pAVCCodecBuf);
  }
}

/* ============================================================================
  @brief      Function to parse a TS Packet.

  @details    This function is used to read TS Packet Size worth of data and
              parse it.

  @param[in/out]    pContext        Context pointer.

  @return   MP2STREAM_SUCCESS if parsed successfully, else corresponding error.

  @note       None.
============================================================================ */
MP2StreamStatus parseTransportStreamPacket(MP2ParserContext *pContext)
{
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  bool bContinue = true;
  uint8 ucAdapationField = 0;

  ProgramMapSection*     pCurPMT   = pContext->pMapSection;
  MP2TransportStreamPkt* pTSPacket = &pContext->sTSPacket;
  uint8* pucBuf     = pContext->pucBuf;
  uint32 ulBufSize  = pContext->ulBufSize;
  uint32 ulIndex    = pContext->ulBufIndex;
  uint32 ulDataRead = 0;
  uint64 ullOffset  = pContext->ullOffset;

  //! Check whether TS Packet started with sync marker or not
  if(TS_PKT_SYNC_BYTE != pucBuf[ulIndex])
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseTransportStreamPacket Sync byte(0x47) not found!!");
    retError = MP2STREAM_CORRUPT_DATA;
    bContinue = false;
  }
  if(bContinue)
  {
    uint8 ucVal = 0;
    retError = MP2STREAM_SUCCESS;
    pTSPacket->sync_byte = TS_PKT_SYNC_BYTE;
    pTSPacket->noffset   = ullOffset;

    getByteFromBitStream(&ucVal,&pucBuf[1],0,1);
    pTSPacket->t_error_indicator = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[1],1,1);
    pTSPacket->pyld_unit_start_indicator = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[1],2,1);
    pTSPacket->transport_priority = ucVal;

    pTSPacket->PID  = (uint16)((pucBuf[1] & 0x1F)<< 8);
    pTSPacket->PID  = (uint16)(pTSPacket->PID |  pucBuf[2]);

    getByteFromBitStream(&ucVal,&pucBuf[3],0,2);
    pTSPacket->transport_scrambling_control = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[3],2,2);
    pTSPacket->adaption_field_control = ucVal;
    ucAdapationField = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[3],4,4);
    pTSPacket->continuity_counter = ucVal;

    ulIndex = TS_PKT_HDR_BYTES;

    int nProgMatchedIndex = 0;
    uint64 startOffset = pTSPacket->noffset;
    pTSPacket->adaption_field.adaption_field_length = 0;

    //! Update the index after parsing TS header and adaptation fields
    pContext->ulBufIndex = ulIndex;
    pTSPacket->ucHeaderBytes = TS_PKT_HDR_BYTES;
    //Check if transport packet contains adaption field
    if( (TS_ADAPTION_FILED_PRESENT_NO_PYLD == ucAdapationField )||
        (TS_ADAPTION_FILED_DATA_PRSENT == ucAdapationField) )
    {
      //Parse the adaption field
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
        "MP2StreamParser PID %u contains adaption field", pTSPacket->PID);

      retError = parseAdaptationField(pContext);
      //! Update TS Packet Header Size (Include adaptation field len as well)
      pTSPacket->ucHeaderBytes = (uint8)(pTSPacket->ucHeaderBytes + (uint8)
                 (pTSPacket->adaption_field.adaption_field_length + 1));
    }

    /* If PID matches with PCR PID and it has no payload data, then check
       whether discontinuity is set or not. If discontinuity flag is set,
       then update base time accordingly in following functions. */
    if((pCurPMT) && (pCurPMT->PCR_PID == pTSPacket->PID) &&
       (TS_ADAPTION_FILED_PRESENT_NO_PYLD == ucAdapationField))
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                  "MP2StreamParser PCR PID is found");
      pContext->ulDataRead = 0;
      //! Set disc flag
      if (pTSPacket->adaption_field.discontinuity_indicator)
      {
        pContext->bDiscFlag = true;
      }
    }
    //! If TS Packet has no payload data, then skip it
    else if((TS_ADAPTION_FILED_PRESENT_NO_PYLD == ucAdapationField) ||
            ((TS_ADAPTION_FILED_RESERVED ==  ucAdapationField )))
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                   "MP2StreamParser contains adaption field only");
      pContext->ulDataRead = 0;
    }
    else if( (pContext->bInitialParsingPending) ||
             (pTSPacket->PID == pContext->usVideoPIDSelected) ||
             (pTSPacket->PID == pContext->usAudioPIDSelected) ||
             (isPSI(pTSPacket->PID, pContext)) )
    {
      /* If Scrambling control bits are set as 2 (0x10) or 3 (0x11), it means
         TS Packet is encrypted. Do not process TS Packet further, update TS
         header bytes and return. Processing of TS Packet depends on
         callee function. */
      if (pTSPacket->transport_scrambling_control > 1)
      {
        retError = MP2STREAM_SUCCESS;
      }
      else if((pContext->bInitialParsingPending) &&
              (isPSI(pTSPacket->PID, pContext)) )
      {
        retError = parseTablesData(pContext);
      } //! if((m_bInitialParsingPending) && isPSI(pTSPacket->PID))
      else if ((pTSPacket->PID > TS_GENERAL_PURPOSE_PID_START) &&
               (pTSPacket->PID < TS_GENERAL_PURPOSE_PID_END) )
      {
        retError = parsePayloadData(pContext);
      } //if ((m_currTSPkt.PID > TS_GENERAL_PURPOSE_PID_START) &&..)
    }
  }//!if(bContinue)
  return retError;
}

/* ==========================================================================
  @brief      Function to parse various Descriptor tables.

  @details    This function is used to parse various metadata tables such as
              PAT, PMT and CAT. It first checks whether sufficient data

  @param[in]      pContext      Context pointer.

  @return     MP2STREAM_SUCCESS indicating metadata read successfully.
              Else, it will report corresponding error.
  @note       BufferSize should be less than metadata size value (188).
========================================================================== */
MP2StreamStatus parseTablesData(MP2ParserContext* pContext)
{
  MP2StreamStatus retError = MP2STREAM_SUCCESS;;
  uint8* pucBuf     = pContext->pucBuf;
  uint32 ulBufIndex = pContext->ulBufIndex;
  MP2TransportStreamPkt* pTSPacket = &pContext->sTSPacket;
  SectionData* pPATInfo = pContext->pPATData;
  SectionData* pCATInfo = pContext->pCATData;
  SectionData* pPMTInfo = pContext->pPMTData;
  bool   bContinue         = true;
  bool   bPayloadStart     = pTSPacket->pyld_unit_start_indicator;
  uint16 usSectionLen      = 0;
  uint16 usDataAvailable   = 0;
  uint16 usProgIndex       = 0;

  if((pContext->bInitialParsingPending) && isPSI(pTSPacket->PID, pContext))
  {
    if( (TS_PROG_ASSOCIATION_TBL_PID == pTSPacket->PID) &&
        (!pContext->bGetLastPTS) )
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "MP2StreamParser encountered PAT @ %llu", pContext->ullOffset);
      //! Calculate Section Length, if not calculated already
      if (0 == pPATInfo->usSectionLen)
      {
        pPATInfo->usSectionLen = CalcSectionLength(pucBuf, ulBufIndex,
                                                   bPayloadStart);
      }
      usSectionLen    = pPATInfo->usSectionLen;
      usDataAvailable = (uint16)(FILESOURCE_MIN(TS_PKT_SIZE - (uint16)ulBufIndex,
                                       usSectionLen - pPATInfo->usFilledLen));

      memcpy(pPATInfo->pucSectionDataBuf + pPATInfo->usFilledLen,
             pucBuf + ulBufIndex, usDataAvailable);
      pPATInfo->usFilledLen = (uint16)(pPATInfo->usFilledLen + usDataAvailable);
      ulBufIndex           += usDataAvailable;
      //! If complete PAT is read into local cache, then parse PAT
      if(pPATInfo->usSectionLen == pPATInfo->usFilledLen)
      {
        retError = parseProgAssociationTable(pContext);
        pPATInfo->usSectionLen = 0;
      }
    } // if (m_currTSPkt.PID == TS_PROG_ASSOCIATION_TBL_PID)
    else if( (TS_CONDITIONAL_ACCESS_TBL_PID == pTSPacket->PID) &&
             (!pContext->bGetLastPTS) )
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
           "MP2StreamParser encountered CAT PID %u",pTSPacket->PID);
      //! Calculate Section Length, if not calculated already
      if (0 == pCATInfo->usSectionLen )
      {
        pCATInfo->usSectionLen = CalcSectionLength(pucBuf, ulBufIndex,
                                                   bPayloadStart);
      }
      usSectionLen    = pCATInfo->usSectionLen;
      usDataAvailable = (uint16)(FILESOURCE_MIN(TS_PKT_SIZE - (uint16)ulBufIndex,
                                       usSectionLen - pCATInfo->usFilledLen));
      memcpy(pCATInfo->pucSectionDataBuf + pCATInfo->usFilledLen,
             pucBuf + ulBufIndex, usDataAvailable);
      pCATInfo->usFilledLen = (uint16)(pCATInfo->usFilledLen + usDataAvailable);
      ulBufIndex           += usDataAvailable;
      //! If complete CAT is read into local cache, then parse CAT
      if(pCATInfo->usSectionLen == pCATInfo->usFilledLen)
      {
        retError = parseCondAccessTable(pContext);
        pCATInfo->usSectionLen = 0;
      }
    } // if (TS_CONDITIONAL_ACCESS_TBL_PID == pTSPacket->PID)
    else if( (isProgramMapPacket(pTSPacket->PID, &usProgIndex, pContext)) &&
             (!pContext->bGetLastPTS) )
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "MP2StreamParser encountered PMT @ %llu", pContext->ullOffset);
      //! Calculate Section Length, if not calculated already
      if (0 == pPMTInfo->usSectionLen)
      {
        pPMTInfo->usSectionLen = CalcSectionLength(pucBuf, ulBufIndex,
                                                   bPayloadStart);
      }
      usSectionLen    = pPMTInfo->usSectionLen;
      usDataAvailable = (uint16)FILESOURCE_MIN(TS_PKT_SIZE - (uint16)ulBufIndex,
                                       usSectionLen - pPMTInfo->usFilledLen);
      memcpy(pPMTInfo->pucSectionDataBuf + pPMTInfo->usFilledLen,
             pucBuf + ulBufIndex, usDataAvailable);
      pPMTInfo->usFilledLen = (uint16)(pPMTInfo->usFilledLen + usDataAvailable);
      ulBufIndex           += usDataAvailable;
      //! If complete PMT is read into local cache, then parse PMT
      if(pPMTInfo->usSectionLen == pPMTInfo->usFilledLen)
      {
        retError = parseProgMapTable(pContext);
        pPMTInfo->usSectionLen = 0;
        pPMTInfo->usFilledLen  = 0;
        if (MP2STREAM_SUCCESS == retError)
        {
          retError = UpdateTrackInfoUsingDescriptors(pContext);
        }
        //! It means same PMT is found again
        else if(MP2STREAM_DEFAULT_ERROR == retError)
          retError = MP2STREAM_SUCCESS;
      }
    } // if( (isProgramMapPacket(m_currTSPkt.PID,&nProgMatchedIndex)) && ..
  } // if((m_bInitialParsingPending) && isPSI(m_currTSPkt.PID))
  return retError;
}

/* ==========================================================================
  @brief      Function to parse payload data available in TS Packet.

  @details    This function is used to check whether Payload is started in
              this TS Packet or not.
              If it is middle of TS Packet, it will read payload data only.

  @param[in]      pContext        Context pointer.

  @return     MP2STREAM_SUCCESS indicating metadata read successfully.
              Else, it will report corresponding error.
  @note       None.
========================================================================== */
MP2StreamStatus parsePayloadData(MP2ParserContext* pContext)
{
  MP2StreamStatus   retError = MP2STREAM_SUCCESS;
  MP2TransportStreamPkt* pTSPkt = &pContext->sTSPacket;
  uint8* pucBuf    = pContext->pucBuf;
  uint32 ulIndex   = pContext->ulBufIndex;

  //New PES packet starts when pyld_unit_start_indicator = 1
  //This is handled separately.

  if((TS_PKT_SIZE > ulIndex) && (!pTSPkt->pyld_unit_start_indicator))
  {
    memmove(pucBuf, pucBuf + ulIndex,
            TS_PKT_SIZE - ulIndex);
    pContext->ulDataRead = (uint32)(TS_PKT_SIZE - ulIndex);
  }
  return retError;
}

/* ==========================================================================
  @brief      Calculate Section Data Length.

  @details    This function is used to calculate the section length for
              PAT, PMT and CAT.

  @param[in]    pucBuf        Data Buffer pointer.
  @param[in]    ulIndex       read Index.
  @param[in]    bPayloadStart Flag to indicate payload start.

  @return   true if memory allocated successfully, else false.

  @note       None.
========================================================================== */
uint16 CalcSectionLength(uint8* pucBuf, uint32& ulIndex, bool bPayloadStart)
{
  uint16 usSectionLen = 0;
  if((0x01 == bPayloadStart) && ulIndex < TS_PKT_SIZE)
  {
    //First byte contains pointer field.
    ulIndex += pucBuf[ulIndex];
    ulIndex++;

    //! At least 3 bytes needed to calculate PAT Length
    if (ulIndex + 3 < TS_PKT_SIZE)
    {
      //! First byte contains Table ID and first 4 bits of 2nd byte contains
      //! other flags. Section length is 12 bit field
      usSectionLen = (uint16)((pucBuf[ulIndex + 1] & 0x0F) << 12);
      usSectionLen = (uint16)(usSectionLen | pucBuf[ulIndex + 2]);
    }
  }//if(bOK)
  return usSectionLen;
}

/* ============================================================================
  @brief      Function to check for registration descriptors.

  @details    This function is used to check whether requested descriptor
              is available in given list or not.

  @param[in]      pList              Descriptor list.
  @param[in]      ulRegDesctiptor    Registration descriptor.

  @return     True if found, else false.
  @note       None.
============================================================================ */
bool SearchRegDescriptor(Desciptor* pList, uint32 ulRegDesctiptor)
{
  bool bIsPresent = false;

  while (pList)
  {
    uint32 ulDescValue = 0;
    uint8* pucBuf = NULL;
    if((TS_REGISTRATION_DESC_TAG == pList->ucDescriptorTag) &&
       (pList->pucDescrData))
    {
      pucBuf = pList->pucDescrData;
      ulDescValue = pucBuf[0] <<24 | pucBuf[1] << 16;
      ulDescValue |= pucBuf[2] << 8 | pucBuf[3];
      //memcpy(&ulDescValue, pList->pucDescrData, 4);
      if (ulDescValue == ulRegDesctiptor)
      {
        bIsPresent = true;
        break;
      }
    }
    pList = pList->pNext;
  }
  return bIsPresent;
}

/* ============================================================================
  @brief    Function to check for whether required descriptor is present or not.

  @details    This function is used to check whether requested descriptor
              is available in given list or not.

  @param[in]      pList              Descriptor list.
  @param[in]      ulRegDesctiptor    Descriptor tag.

  @return     True if found, else false.
  @note       None.
============================================================================ */
bool FindDescriptor(Desciptor* pList, uint8 ucDesctiptorTag)
{
  bool bIsPresent = false;
  while (pList)
  {
    if (ucDesctiptorTag == pList->ucDescriptorTag)
    {
      bIsPresent = true;
      break;
    }
    pList = pList->pNext;
  }
  return bIsPresent;
}

/* ==========================================================================
@brief      Function to update track info using descriptors.

@details    This function will update track info with the given descriptors.

@param[in]      pContext        Context pointer.

@return     Pointer of first entry in list will be returned in success case,
            Else, NULL pointer.
@note       None.
========================================================================== */
MP2StreamStatus UpdateTrackInfoUsingDescriptors(MP2ParserContext* pContext)
{
  MP2StreamStatus retError = MP2STREAM_SUCCESS;
  bool   bIsEncrypted      = false;
  bool   bESStreamTypeSet  = false;
  uint8  ucDescriptorTag   = 0xFF;
  uint32 ulESDescIndex     = 0;
  ESDescriptor* pESDesc    = NULL;
  audio_info*   pAudioInfo = NULL;
  video_info*   pVideoInfo = NULL;
  ProgramMapSection* pMAPSection = pContext->pMapSection;
  stream_info*       pStreamInfo = pContext->pStreamInfo;
  CADescriptor sCADesc;
  //! Validate input params
  if((!pMAPSection) || ( NULL == pMAPSection->ESDescData) || (!pStreamInfo))
  {
    return retError;
  }

  pESDesc = pMAPSection->ESDescData;
  /* Check whether all the programs are encrypted or not. */
  bIsEncrypted = ParseDescriptor(pMAPSection->pPIDescList,
                                 TS_CA_DESC_TAG, (void*)&sCADesc);
  if (bIsEncrypted)
  {
    pMAPSection->bIsEncrypted = true;
    pMAPSection->pCADesc = (CADescriptor*)MM_Malloc(sizeof(CADescriptor));
    if (pMAPSection->pCADesc)
    {
      memcpy(pMAPSection->pCADesc, &sCADesc, sizeof(CADescriptor));
    }
  }
  while(pESDesc )
  {
    uint32 ulStrIndex = 0;
    for( ; ulStrIndex < pContext->usNumStreams; ulStrIndex++)
    {
      if(pContext->pStreamInfo[ulStrIndex].stream_id == pESDesc->elementary_pid)
      {
        pStreamInfo = &pContext->pStreamInfo[ulStrIndex];
        pVideoInfo  = &pStreamInfo->video_stream_info;
        pAudioInfo  = &pStreamInfo->audio_stream_info;
        bESStreamTypeSet  = pStreamInfo->bParsed;
        break;
      }
    } // for(..)
    //! If Stream is already updated with all the info, then ignore it
    if ((bESStreamTypeSet) || (!pAudioInfo))
    {
      pESDesc = pESDesc->pNext;
      continue;
    }
    //! If HDMV flag is set and stream type is beyond
    if (pContext->bHDMVFlag && pESDesc->stream_type >= HDMV_LPCM_STREAM_TYPE)
    {
      switch(pESDesc->stream_type)
      {
        case HDMV_LPCM_STREAM_TYPE:
          ucDescriptorTag = TS_REGISTRATION_DESC_TAG;
          break;
        case HDMV_AC3_STREAM_TYPE:
        case HDMV_AC3_LOSSLESSSTREAM_TYPE:
        case HDMV_EAC3_STREAM_TYPE:
          ucDescriptorTag = TS_AC3_AUDIO_DESC_TAG;
          break;
        case HDMV_DTS_STREAM_TYPE:
          ucDescriptorTag = TS_DTS_DESC_TAG;
          break;
        case DTS_HD_STREAM_TYPE:
        case DTS_HD_EXCEPT_XLLSTREAM_TYPE:
          ucDescriptorTag = TS_DTSHD_DESC_TAG;
          break;
        case HDMV_DRA_STREAM_TYPE:
          ucDescriptorTag = 0xFF;
          break;
      }
    }
    else if(LPCM_AUDIO_STREAM_TYPE == pESDesc->stream_type)
    {
      ucDescriptorTag = TS_DVD_LPCM_AUDIO_DESC_TAG;
      pAudioInfo->Audio_Codec = AUDIO_CODEC_LPCM;
    }
    else if(AC3_AUDIO_STREAM_TYPE == pESDesc->stream_type )
    {
      ucDescriptorTag = TS_AC3_AUDIO_DESC_TAG;
      pAudioInfo->Audio_Codec = AUDIO_CODEC_AC3;
    }
    else if (HDMV_DTS_STREAM_TYPE == pESDesc->stream_type )
    {
      ucDescriptorTag = TS_DTS_DESC_TAG;
      pAudioInfo->Audio_Codec = AUDIO_CODEC_DTS;
    }
    else if( DTS_HD_STREAM_TYPE == pESDesc->stream_type )
    {
      ucDescriptorTag = TS_DTSHD_DESC_TAG;
      pAudioInfo->Audio_Codec = AUDIO_CODEC_DTS;
    }
    else if(PES_PVT_STREAM_TYPE == pESDesc->stream_type )
    {
      bool isDTS1 = SearchRegDescriptor(pESDesc->pESDescList,
                                        TS_REGIS_FORMATID_DTS1);
      bool isDTS2 = SearchRegDescriptor(pESDesc->pESDescList,
                                        TS_REGIS_FORMATID_DTS2);
      bool isDTS3 = SearchRegDescriptor(pESDesc->pESDescList,
                                        TS_REGIS_FORMATID_DTS3);
      bool isDTSHD = SearchRegDescriptor(pESDesc->pESDescList,
                                         TS_REGIS_FORMATID_DTSH);

      //! If one of these descriptors are available, then look for DTS
      if ((isDTS3) || (isDTS2) || (isDTS1))
      {
        ucDescriptorTag = TS_DTS_DESC_TAG;
        pAudioInfo->Audio_Codec = AUDIO_CODEC_DTS;
      }
      else if (isDTSHD)
      {
        ucDescriptorTag = TS_DTSHD_DESC_TAG;
        pAudioInfo->Audio_Codec = AUDIO_CODEC_DTS;
      }
      else
      {
        bool isAC3 = SearchRegDescriptor(pESDesc->pESDescList,
                                         AC3_REG_DESCRIPTOR);
        bool isEAC3 = SearchRegDescriptor(pESDesc->pESDescList,
                                          EC3_REG_DESCRIPTOR);
        if (isAC3)
        {
          pAudioInfo->Audio_Codec = AUDIO_CODEC_AC3;
          ucDescriptorTag = AC3_SYS_B_AUDIO_DESC_TAG;
        }
        else if(isEAC3)
        {
          pAudioInfo->Audio_Codec = AUDIO_CODEC_EAC3;
          ucDescriptorTag = EC3_SYS_B_AUDIO_DESC_TAG;
        }
      }
    } // else if(PES_PVT_STREAM_TYPE == ESDescData->stream_type )
    //! Check whether descriptor is available
    if (0xFF != ucDescriptorTag)
    {
      bool bRet = ParseDescriptor(pESDesc->pESDescList,
                                  ucDescriptorTag,
                                  (void*)pAudioInfo);
      if(bRet)
      {
        pStreamInfo->bParsed = true;
      }
    }
    (void)ParseDescriptor(pESDesc->pESDescList,
                          TS_ISO_639_LANG_DESC_TAG,
                          (void*)pAudioInfo);

    /* Check whether Elementary stream is encrypted or not. */
    memset(&sCADesc, 0, sizeof(CADescriptor));
    bool bRet = ParseDescriptor(pESDesc->pESDescList,
                                TS_CA_DESC_TAG,
                                (void*)&sCADesc);
    if (bRet)
    {
      CADescriptor* pCADesc = NULL;
      pCADesc = (CADescriptor*)MM_Malloc(sizeof(CADescriptor));
      pStreamInfo->pCADesc      = pCADesc;
      pStreamInfo->bIsEncrypted = true;
      if (pCADesc)
      {
        memcpy(pCADesc, &sCADesc, sizeof(CADescriptor));
      }
    } // if (bRet)
    pESDesc = pESDesc->pNext;
  } // while(pESDesc )
  return retError;
}

/* ============================================================================
  @brief      Function to parse required descriptors.

  @details    This function is used to check whether requested descriptor
              is available in given list and parses the descriptor if
              available.

  @param[in]      pList              Descriptor list.
  @param[in]      ucDescrTag         Registration descriptor.
  @param[in/out]  pVoid              Structure to keep descriptor data.

  @return     True if found and parsed successfully, else false.

  @note       None.
============================================================================ */
bool ParseDescriptor(Desciptor* pDescList, uint8 ucDescrTag, void* pVoid)
{
  bool bRet = false;
  MP2StreamStatus retStatus = MP2STREAM_DEFAULT_ERROR;
  CADescriptor* pCADesc     = (CADescriptor*)pVoid;
  audio_info*   pAudioInfo  = (audio_info*)pVoid;

  while(pDescList)
  {
    if(ucDescrTag == pDescList->ucDescriptorTag)
    {
      switch(ucDescrTag)
      {
        case TS_DTS_DESC_TAG:
          retStatus = parseDTSAudioDescriptor(pDescList, pAudioInfo);
          break;
        case TS_DTSHD_DESC_TAG:
          retStatus = parseDTSHDAudioDescriptor(pDescList, pAudioInfo);
          break;
        case TS_CA_DESC_TAG:
          retStatus = parseCADescriptor(pDescList, pCADesc);
          break;
        case TS_DVD_LPCM_AUDIO_DESC_TAG:
          retStatus = parseDVDLPCMAudioDescriptor(pDescList, pAudioInfo);
          break;
          //! This is a special case, HD-LPCM codec data in Registration descr
        case TS_REGISTRATION_DESC_TAG:
          retStatus = parseHDMVLPCMAudioDescriptor(pDescList, pAudioInfo);
          break;
        case TS_ISO_639_LANG_DESC_TAG:
          {
            LanguageDescriptor sLang;
            MP2StreamStatus eStatus = parseLanguageDescriptor(pDescList, &sLang);
            if (MP2STREAM_SUCCESS == eStatus)
            {
              memcpy(pAudioInfo->ucLangCode, &sLang.ucISO639LangCode, 4);
              pAudioInfo->ucLangCode[3] = '\0';
            }
          }
          break;
        case TS_AC3_AUDIO_DESC_TAG:
          retStatus = parseAC3AudioDescriptor(pDescList, pAudioInfo);
          break;
        default:
          bRet = false;
          break;
      }
    } // if(ucDescrTag == pDescList->ucDescriptorTag)
    pDescList = pDescList->pNext;
  } // while(pDescList)

  if (MP2STREAM_SUCCESS == retStatus)
  {
    bRet = true;
  }
  return bRet;
}

/* ==========================================================================
@brief      Function to parse Program Association Table.

@details    This function is used to parse PAT read into PAT Section buffer.

@param[in]      pContext        Context pointer.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseProgAssociationTable(MP2ParserContext* pContext)
{
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  bool bOK = true;
  uint8 ucVal = 0;
  uint16 usIndex = 0;
  ProgramAssociationSection tempPAT;

  MP2TransportStreamPkt* pTSPkt   = &pContext->sTSPacket;
  SectionData*           pPATInfo = pContext->pPATData;
  uint8* pucDataBuf = pPATInfo->pucSectionDataBuf;
  uint32 ulBufSize  = pPATInfo->usFilledLen;
  ProgramAssociationSection* pPATSection = &pContext->sPATSection;

  memset(&tempPAT,0,sizeof(ProgramAssociationSection));

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "parseProgAssociationTable @ offset %llu", pTSPkt->noffset);

  if((!pContext->bStartOffsetSet) && (!pContext->bGetLastPTS))
  {
    pContext->ullStartOffset  = pTSPkt->noffset;
    pContext->bStartOffsetSet = true;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
      "parseProgAssociationTable m_nStartOffset %llu",pTSPkt->noffset);
  }

  getByteFromBitStream(&ucVal,&pucDataBuf[usIndex],0,8);
  tempPAT.common_sect_data.table_id = ucVal;
  usIndex++;

  getByteFromBitStream(&ucVal,&pucDataBuf[usIndex],0,1);
  tempPAT.common_sect_data.sect_synt_indtor = ucVal;

  if(tempPAT.common_sect_data.sect_synt_indtor != 0x01)
  {
    bOK = false;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseProgAssociationTable sect_synt_indtor != 0x01");
    return MP2STREAM_CORRUPT_DATA;
  }

  getByteFromBitStream(&ucVal,&pucDataBuf[usIndex],1,1);
  tempPAT.common_sect_data.zero_field = ucVal;

  getByteFromBitStream(&ucVal,&pucDataBuf[usIndex],2,2);
  tempPAT.common_sect_data.reserved = ucVal;

  tempPAT.common_sect_data.sect_length = (uint16)((pucDataBuf[usIndex++] & 0x0F) << 12);

  tempPAT.common_sect_data.sect_length = (uint16)
 (tempPAT.common_sect_data.sect_length | pucDataBuf[usIndex++]);

  // m_ProgramAssociationSect.common_sect_data.sect_length is 12 bit
  if((tempPAT.common_sect_data.sect_length & 0xC00) ||
    (tempPAT.common_sect_data.sect_length > MAX_SECT_LENGTH))
  {
    //First 2 bits are not '00', stream is corrupted.
    bOK = false;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseProgAssociationTable section length is corrupted");
    return MP2STREAM_CORRUPT_DATA;
  }
  else
  {
    //Reset number of bytes consumed before we start reading further.
    //counting number of bytes consumed in this section will help in
    //determining whether PAT is complete or not.
    tempPAT.nBytesConsumed = 0;
    tempPAT.transport_stream_id = (uint16)(pucDataBuf[usIndex++]<<8);
    tempPAT.nBytesConsumed++;
    tempPAT.transport_stream_id = (uint16)
       (tempPAT.transport_stream_id | pucDataBuf[usIndex++]);
    tempPAT.nBytesConsumed++;

    tempPAT.version_no = (uint8)((pucDataBuf[usIndex] & 0x3E)>> 1);
    tempPAT.current_next_indicator = pucDataBuf[usIndex] & 0x01;
    usIndex++;
    tempPAT.nBytesConsumed++;
    tempPAT.section_no = pucDataBuf[usIndex++];
    tempPAT.nBytesConsumed++;
    tempPAT.last_sect_no = pucDataBuf[usIndex++];
    tempPAT.nBytesConsumed++;
    tempPAT.isAvailable = true;
  }

  if(pPATSection->isAvailable)
  {
    //Reset selected program pids
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "Resetting m_nVideoPIDSelected & m_nAudioPIDSelected");

    if(pPATSection->current_next_indicator)
    {
      //We are looking into current PAT
      if(tempPAT.version_no == pPATSection->version_no)
      {
        if(tempPAT.section_no <= pPATSection->section_no)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"Rewriting PAT");
          //Free old buffers before alloc and copying new
          freePAT(pPATSection);
          memcpy(pPATSection,&tempPAT,sizeof(ProgramAssociationSection));
        }
        else
        {
          //Current PAT is continuing here
          //We will take care of re-alloc while copying
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"Current PAT is continuing");
        }
      }
      else
      {
        //Found PAT with new version number, rewriting our members with new PAT
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"PAT with new version#, rewrite");
        //Free old buffers before alloc and copying new
        freePAT(pPATSection);
        memcpy(pPATSection,&tempPAT,sizeof(ProgramAssociationSection));
      }
    }
    else
    {
      //This is next PAT to become applicable
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"New PAT available, rewrite");
      //Free old buffers before alloc and copying new
      freePAT(pPATSection);
      memcpy(pPATSection,&tempPAT,sizeof(ProgramAssociationSection));
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"Found PAT");
    memcpy(pPATSection,&tempPAT,sizeof(ProgramAssociationSection));
  }

  uint16 usCount = (uint16)((pPATSection->common_sect_data.sect_length -
                    SECTION_HDR_CRC_BYTES) / PROGRAM_NO_PID_BYTES);
  //We are not going to parse CRC at the end of this section,
  //so count it in bytes consumed.
  pPATSection->nBytesConsumed = (uint16)(pPATSection->nBytesConsumed + CRC_BYTES);
  if(usCount > 0)
  {
    uint16 usNumPrograms = pPATSection->nPrograms;
    if(pPATSection->nPrograms == 0)
    {
      pPATSection->nPrograms = usCount;

      pPATSection->pusProgNum = (uint16*)MM_Malloc(sizeof(uint16)* usCount);
      pPATSection->pusTSPID = (uint16*)MM_Malloc(sizeof(uint16)* usCount);
    }
    else
    {
      if(tempPAT.section_no > pPATSection->section_no)
      {
        //program_numbers & ts_PID should be realloc. We will copy these into TEMP
        //before we do the memcpy of TEMP(latest PAT) into member so we dont lose it.
        tempPAT.pusProgNum = pPATSection->pusProgNum;
        tempPAT.pusTSPID = pPATSection->pusTSPID;
        //memcpy everything from latest PAT into member
        memcpy(pPATSection,&tempPAT,sizeof(ProgramAssociationSection));
        pPATSection->nPrograms = (uint16)(pPATSection->nPrograms + usCount);

        //Now Realloc
        pPATSection->pusProgNum =
          (uint16*)MM_Realloc(pPATSection->pusProgNum,(sizeof(uint16)* pPATSection->nPrograms));
        pPATSection->pusTSPID =
          (uint16*)MM_Realloc(pPATSection->pusTSPID,(sizeof(uint16)* pPATSection->nPrograms));
      }
    }
    if( (!pPATSection->pusProgNum) ||
      (!pPATSection->pusTSPID) )
    {
      bOK = false;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
        "parseProgAssociationTable memory allocation failed, nCount %d", usCount);
      retError = MP2STREAM_OUT_OF_MEMORY;
    }
    else
    {
      for(int count = usNumPrograms; count < pPATSection->nPrograms;count++)
      {
        uint16 usProgNum = (uint16)(pucDataBuf[usIndex++] << 8);
        pPATSection->nBytesConsumed++;
        usProgNum = (uint16)(usProgNum | pucDataBuf[usIndex++]);
        pPATSection->nBytesConsumed++;
        uint16 usTSPID = (pucDataBuf[usIndex++] & 0x1F);
        usTSPID = (uint16)(usTSPID << 8);
        pPATSection->nBytesConsumed++;
        usTSPID = (uint16)(usTSPID | pucDataBuf[usIndex++]);
        pPATSection->nBytesConsumed++;
        pPATSection->pusProgNum[count] = usProgNum;
        pPATSection->pusTSPID[count] = usTSPID;
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
          "parseProgAssociationTable prog_no %d ts_pid %d",
          usProgNum, usTSPID);
      }
    }
  }

  if(bOK)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
      "parseProgAssociationTable sect_no %d last_sect_no %d",
      pPATSection->section_no, pPATSection->last_sect_no);
    if( (pPATSection->nBytesConsumed == pPATSection->common_sect_data.sect_length) &&
      (pPATSection->section_no == pPATSection->last_sect_no) )
    {
      if(pPATSection->current_next_indicator == 0x01)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "Program Association Table is complete, set bPATComplete to TRUE");
        pPATSection->bPATComplete = true;
      }
    }
    retError = MP2STREAM_SUCCESS;
  }
  return retError;
}

/* ==========================================================================
@brief      Function to parse Adaptation Field in TS Packet Header.

@details    This function is used to parse adaptation field in TS Pkt.

@param[in]      pContext        Context pointer.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseAdaptationField(MP2ParserContext* pContext)
{
  MP2StreamStatus retError = MP2STREAM_SUCCESS;
  uint8* pucBuf    = pContext->pucBuf;
  uint32 ulBufSize = pContext->ulBufSize;
  uint32 ulIndex   = pContext->ulBufIndex;
  uint8  ucVal     = 0;
  uint32 ulStart   = ulIndex + 1;

  MP2TransportStreamPkt* pTSPkt       = &pContext->sTSPacket;
  MP2TStreamAdaptionField* pAdapField = &pTSPkt->adaption_field;

  //! Get Adaptation field length
  pAdapField->adaption_field_length = pucBuf[ulIndex];

  if(pTSPkt->adaption_field_control == TS_ADAPTION_FILED_DATA_PRSENT)
  {
    if(pAdapField->adaption_field_length > TS_ADPT_PLYD_MAX_LEN)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "adaption_field_length %d > TS_ADPT_PLYD_MAX_LEN, NON STANDARD value",
                  pAdapField->adaption_field_length);
    }
  }
  else if(pTSPkt->adaption_field_control == TS_ADAPTION_FILED_PRESENT_NO_PYLD)
  {
    if(pAdapField->adaption_field_length > TS_ADPT_NOPLYD_MAX_LEN)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "adaption_field_length %d > TS_ADPT_NOPLYD_MAX_LEN, NON STANDARD value",
                  pAdapField->adaption_field_length);
    }
  }
  //! One byte is used to to store Adaptation field length
  ulIndex++;

  if(pAdapField->adaption_field_length)
  {
    getByteFromBitStream(&ucVal,&pucBuf[ulIndex],0,1);
    pAdapField->discontinuity_indicator = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[ulIndex],1,1);
    pAdapField->random_access_indicator = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[ulIndex],2,1);
    pAdapField->es_priority_indicator = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[ulIndex],3,1);
    pAdapField->PCR_flag = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[ulIndex],4,1);
    pAdapField->OPCR_flag = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[ulIndex],5,1);
    pAdapField->splicing_point_flag = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[ulIndex],6,1);
    pAdapField->transport_pvt_data_flag = ucVal;

    getByteFromBitStream(&ucVal,&pucBuf[ulIndex],7,1);
    pAdapField->adaption_field_extn_flag = ucVal;

    ulIndex++;

    if(pAdapField->PCR_flag == 1)
    {
      pAdapField->prog_clk_ref_base = getBytesValue(4,pucBuf+ulIndex) << 1;
      pAdapField->prog_clk_ref_base |= ((pucBuf[ulIndex+4] & 0x01)<< 8);

      //Reserved
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],34,6);

      ulIndex += 4;

      uint8 pcrextpart1     = (uint8)((pucBuf[ulIndex++] & 0x03)<<6);
      //get lower 7 bits out of 9 bit pcr extension
      uint8 pcrextpart2     = (pucBuf[ulIndex++] & 0xFE);
      pAdapField->prog_clk_ref_extn = make9BitValue(pcrextpart1,pcrextpart2);
      //! "ullCurPCRVal" value is in micro-seconds.
      //! Timescale value is same as the value used in FileSource
      if(!pContext->bBasePCRFlag)
      {
        pContext->ullRefPCR = ( (pAdapField->prog_clk_ref_base * 300) +
                                pAdapField->prog_clk_ref_extn) / 27;
        pContext->bBasePCRFlag = true;
        pContext->ullCurPCRVal = pContext->ullRefPCR;
        pContext->ullRefPCR   /= MILLISEC_TIMESCALE_UNIT;
      }
      else
      {
        pContext->ullCurPCRVal = ( (pAdapField->prog_clk_ref_base * 300) +
                                    pAdapField->prog_clk_ref_extn) / 27;
      }
      ulIndex += 2;
    }
    MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_LOW,
      "pid %lu, disc flag %d, cont counter %d, pcr base %llu, pcr extn %lu",
      (uint32)pTSPkt->PID,
      pAdapField->discontinuity_indicator, pTSPkt->continuity_counter,
      pAdapField->prog_clk_ref_base,
      (uint32)pAdapField->prog_clk_ref_extn);
    if(pAdapField->OPCR_flag == 1)
    {
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],0,33);
      pAdapField->orig_prog_clk_ref_base = ucVal;
      //Reserved
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],34,6);
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],39,9);
      pAdapField->orig_prog_clk_ref_extn = ucVal;
      ulIndex = ulIndex + 6;
    }
    if(pAdapField->splicing_point_flag == 1)
    {
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],0,8);
      pAdapField->splice_countdown = ucVal;
      ulIndex = ulIndex + 1;
    }
    if(pAdapField->transport_pvt_data_flag == 1)
    {
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],0,8);
      pAdapField->transport_pvt_Data_length = ucVal;
      ulIndex = ulIndex + 1;
      if(pAdapField->transport_pvt_Data_length)
      {
        //Add to localOffset
      }
    }
    if(pAdapField->adaption_field_extn_flag == 1)
    {
      //Todo - check again
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],0,8);
      pAdapField->adaption_field_extn_length = ucVal;
      ulIndex = ulIndex + 1;
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],0,1);
      pAdapField->ltw_flag = ucVal;
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],1,1);
      pAdapField->piecewise_rate_flag = ucVal;
      getByteFromBitStream(&ucVal,&pucBuf[ulIndex],2,1);
      pAdapField->seamless_splice_flag = ucVal;
      // 5 bits reserved
      ulIndex = ulIndex + 1;
      if(pTSPkt->adaption_field.ltw_flag)
      {
        getByteFromBitStream(&ucVal,&pucBuf[ulIndex],0,1);
        pAdapField->ltw_valid_flag = ucVal;
        getByteFromBitStream(&ucVal,&pucBuf[ulIndex],1,15);
        pAdapField->ltw_offset = ucVal;
        ulIndex = ulIndex + 2;
      }
      if(pAdapField->piecewise_rate_flag)
      {
        //Reserved
        getByteFromBitStream(&ucVal,&pucBuf[ulIndex],0,2);
        getByteFromBitStream(&ucVal,&pucBuf[ulIndex],2,22);
        pAdapField->piecewise_rate = ucVal;
        ulIndex = ulIndex + 3;
      }
      if(pAdapField->seamless_splice_flag)
      {
        //getByteFromBitStream(&val,&pucBuf[localOffset],0,2);
        //getByteFromBitStream(&val,&pucBuf[localOffset],2,22);
        //m_currTSPkt.adaption_field.piecewise_rate = val;
        ulIndex = ulIndex + 5;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                  "parseAdaptationField adaption_field_extn_flag is 0");
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "parseAdaptationField adaption_field_length is 0");
  }
  if(ulIndex == (ulStart + pAdapField->adaption_field_length ))
  {
    retError = MP2STREAM_SUCCESS;
  }
  else
  {
    ulIndex = ulStart + pAdapField->adaption_field_length;
  }
  //! Update index before exiting the function
  pContext->ulBufIndex = ulIndex;
  return retError;
}

/* ==========================================================================
@brief      Function to parse Conditional Access Table.

@details    This function is used to parse PAT read into CAT Section buffer.

@param[in]      pContext        Context pointer.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseCondAccessTable(MP2ParserContext* pContext)
{
  MP2TransportStreamPkt*    pTSPacket   = &pContext->sTSPacket;
  ConditionalAccessSection* pCATSection = pContext->pCATSection;
  SectionData*              pCATInfo    = pContext->pCATData;

  MP2StreamStatus retError   = MP2STREAM_DEFAULT_ERROR;
  uint8*          pucDataBuf = pCATInfo->pucSectionDataBuf;
  uint32          ulBufSize  = pCATInfo->usFilledLen;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"parseCondAccessTable");

  //! Allocate memory if not allocated already
  if (!pCATSection)
  {
    pCATSection = (ConditionalAccessSection*)
                  MM_Malloc(sizeof(ConditionalAccessSection));
    pContext->pCATSection = pCATSection;
  }
  if((pucDataBuf) && (ulBufSize) && (pCATSection))
  {
    uint8  ucVal   = 0;
    uint32 ulIndex = 0;

    getByteFromBitStream(&ucVal,&pucDataBuf[ulIndex],0,8);
    pCATSection->common_sect_data.table_id = ucVal;
    ulIndex++;

    getByteFromBitStream(&ucVal,&pucDataBuf[ulIndex],0,1);
    pCATSection->common_sect_data.sect_synt_indtor = ucVal;

    if(pCATSection->common_sect_data.sect_synt_indtor != 0x01)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "parseCondAccessTable sect_synt_indtor != 0x01");
      retError = MP2STREAM_CORRUPT_DATA;
    }

    getByteFromBitStream(&ucVal,&pucDataBuf[ulIndex],1,1);
    pCATSection->common_sect_data.zero_field = ucVal;

    getByteFromBitStream(&ucVal,&pucDataBuf[ulIndex],2,2);
    pCATSection->common_sect_data.reserved = ucVal;

    pCATSection->common_sect_data.sect_length = (uint16)((pucDataBuf[ulIndex] & 0x0F) << 12);
    ulIndex++;
    pCATSection->common_sect_data.sect_length = (uint16)
     (pCATSection->common_sect_data.sect_length | pucDataBuf[ulIndex]);
    // m_CondAccessSection.common_sect_data.sect_length is 12 bit
    if((pCATSection->common_sect_data.sect_length & 0xC00) ||
       (pCATSection->common_sect_data.sect_length > MAX_SECT_LENGTH))
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                   "parseCondAccessTable Section length corrupted %u",
                   pCATSection->common_sect_data.sect_length);
      retError = MP2STREAM_CORRUPT_DATA;
    }
    else
    {
      pCATSection->version_no             = (uint8)((pucDataBuf[ulIndex] & 0x3E)>>1);
      pCATSection->current_next_indicator = pucDataBuf[ulIndex++] & 0x01;

      pCATSection->section_no   = pucDataBuf[ulIndex++];
      pCATSection->last_sect_no = pucDataBuf[ulIndex++];
      uint32 ulDescSize = pCATSection->common_sect_data.sect_length -
                          SECTION_HDR_CRC_BYTES;
      pCATSection->pDescList = prepareDescriptorList(pucDataBuf + ulIndex,
                                                     ulDescSize);
    }
  }// if((pucDataBuf) && (ulBufSize))
  return retError;
}

/* ==========================================================================
@brief      Function to parse Program Map Table.

@details    This function is used to parse PAT read into PMT Section buffer.

@param[in]      pContext        Context pointer.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseProgMapTable(MP2ParserContext* pContext)
{
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  bool bOK      = true;
  bool bnewPMT  = false;
  SectionData*       pPMTData    = pContext->pPMTData;
  ProgramMapSection* pMapSection = pContext->pMapSection;
  ProgramMapSection *pLastPMT    = NULL;
  uint8* pucDataBuf = pPMTData->pucSectionDataBuf;
  uint32 ulBufSize  = pPMTData->usFilledLen;

  //! Allocate memory, if memory is not allocated already
  if(!pContext->pMapSection)
  {
    // do we need to allocate memory for all programs??
    pMapSection =(ProgramMapSection*)MM_Malloc(sizeof(ProgramMapSection));
    pContext->pMapSection = pMapSection;
    if(pMapSection)
    {
      memset(pMapSection, 0, sizeof(ProgramMapSection) );
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "parseProgMapTable m_ProgMapSection malloc failed!");
      retError = MP2STREAM_OUT_OF_MEMORY;
      return retError;
    }
  }//! if(!pContext->pMapSection)
  ProgramMapSection* pCurPMT;
  if(pMapSection && (pMapSection->bProgParseComplete))
  {
    pCurPMT = (ProgramMapSection*)MM_Malloc(sizeof(ProgramMapSection) );
    if(pCurPMT)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"parseProgMapTable found new PMT");
      memset(pCurPMT, 0, sizeof(ProgramMapSection));
      bnewPMT = true;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
          "parseProgMapTable pCurrProgMapSection malloc failed!");
      retError = MP2STREAM_OUT_OF_MEMORY;
      bOK = false;
    }
  }
  else
  {
    pCurPMT = pMapSection;
  }

  if(bOK)
  {
    uint8  ucVal   = 0;
    uint32 ulIndex = 0;

    getByteFromBitStream(&ucVal,&pucDataBuf[ulIndex],0,8);
    pCurPMT->common_sect_data.table_id = ucVal;
    ulIndex++;
    if(TS_PSI_PM_TABLE_ID != pCurPMT->common_sect_data.table_id)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "parseProgMapTable table id != TS_PSI_PM_TABLE_ID");
      //Stream is corrupted
      bOK = false;
      retError = MP2STREAM_CORRUPT_DATA;
    }

    getByteFromBitStream(&ucVal,&pucDataBuf[ulIndex],0,1);
    pCurPMT->common_sect_data.sect_synt_indtor = ucVal;

    if(pCurPMT->common_sect_data.sect_synt_indtor != 0x01)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "parseProgMapTable sect_synt_indtor != 0x01");
      //Stream is corrupted
      bOK = false;
      retError = MP2STREAM_CORRUPT_DATA;
    }

    getByteFromBitStream(&ucVal,&pucDataBuf[ulIndex],1,1);
    pCurPMT->common_sect_data.zero_field = ucVal;

    getByteFromBitStream(&ucVal,&pucDataBuf[ulIndex],2,2);
    pCurPMT->common_sect_data.reserved = ucVal;

    pCurPMT->common_sect_data.sect_length = (uint16)((pucDataBuf[ulIndex] & 0x0F) << 12);
    ulIndex++;
    pCurPMT->common_sect_data.sect_length = (uint16)
      (pCurPMT->common_sect_data.sect_length | pucDataBuf[ulIndex]);
    ulIndex++;
    // pCurrProgMapSection->common_sect_data.sect_length is 12 bit
    if((pCurPMT->common_sect_data.sect_length & 0xC00) ||
       (pCurPMT->common_sect_data.sect_length > MAX_SECT_LENGTH) )
    {
      bOK = false;
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                      "parseProgMapTable sect_length %d > MAX_SECT_LENGTH %d",
                      pCurPMT->common_sect_data.sect_length,MAX_SECT_LENGTH);
      retError = MP2STREAM_CORRUPT_DATA;
    }
    else
    {
      //Reset count of bytes consumed before we start
      //counting as we parse remaining section.
      pCurPMT->nBytesConsumed = 0;

      pCurPMT->usProgNum   = pucDataBuf[ulIndex++];
      pCurPMT->usProgNum = (uint8)(pCurPMT->usProgNum << 8);
      pCurPMT->usProgNum = (uint8)(pCurPMT->usProgNum | pucDataBuf[ulIndex++]);
      if(!pContext->usSelectedProgNum)
        pContext->usSelectedProgNum = pCurPMT->usProgNum;

      pCurPMT->nBytesConsumed = (uint16)(pCurPMT->nBytesConsumed + 2);

      pCurPMT->version_no = (uint8)((pucDataBuf[ulIndex]& 0x3E)>> 1);

      pCurPMT->current_next_indicator = (pucDataBuf[ulIndex++]& 0x01);
      pCurPMT->nBytesConsumed++;

      pCurPMT->section_no = pucDataBuf[ulIndex++];
      pCurPMT->last_sect_no = pucDataBuf[ulIndex++];
      pCurPMT->nBytesConsumed = (uint16)(pCurPMT->nBytesConsumed + 2);

      pCurPMT->PCR_PID   = (pucDataBuf[ulIndex++] & 0x1F);
      pCurPMT->PCR_PID   = (uint16)(pCurPMT->PCR_PID << 8);
      pCurPMT->PCR_PID   = (uint16)(pCurPMT->PCR_PID | pucDataBuf[ulIndex++]);
      pCurPMT->nBytesConsumed = (uint16)(pCurPMT->nBytesConsumed + 2);

      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "parseProgMapTable program %u, version %d, PCR PID %u"
                   ,pCurPMT->usProgNum, pCurPMT->version_no,
                   pCurPMT->PCR_PID);
      pCurPMT->usProgDescLen   = pucDataBuf[ulIndex++]& 0x0F;
      pCurPMT->usProgDescLen   = (uint16)(pCurPMT->usProgDescLen << 8);
      pCurPMT->usProgDescLen   = (uint8)(pCurPMT->usProgDescLen |
                                         pucDataBuf[ulIndex++]);
      pCurPMT->nBytesConsumed  = (uint16)(pCurPMT->nBytesConsumed + 2);

      //We won't be parsing CRC at the end of this section,
      //so just count number of bytes for CRC
      pCurPMT->nBytesConsumed  =(uint16)(pCurPMT->nBytesConsumed + CRC_BYTES);

      //! Check if there is a change in PMT or not.
      if(bOK)
      {
        ProgramMapSection *pTemp = pMapSection;
        while(pTemp)
        {
          /* If either of these three numbers are matching with already parsed
             entry, then skip the entry. */
          if (((pTemp->version_no == pCurPMT->version_no) ||
               (pTemp->usProgNum == pCurPMT->usProgNum) ||
               (pTemp->section_no == pCurPMT->section_no) ) &&
              (pTemp->bProgParseComplete))
          {
            bOK = false;
            break;
          }
          //! Store last successfully parsed PMT pointer
          pLastPMT = pTemp;
          pTemp = pTemp->pNext;
        }
      }
      if(bOK)
      {
        //! Parse Desriptors available as part of PMT
        pCurPMT->pPIDescList = prepareDescriptorList(pucDataBuf + ulIndex,
                                                     pCurPMT->usProgDescLen);

        //! Check if "HDMV" descriptor is available or not
        //! Stream type numbers are different for "HDMV" complaint streams
        pContext->bHDMVFlag = SearchRegDescriptor(pCurPMT->pPIDescList,
                                                    HDMV_REG_DESCRIPTOR);
        uint16 usESInfoSize = (uint16)(pCurPMT->common_sect_data.sect_length -
                                       PROG_MAP_SECT_HDR_BYTES -
                                       pCurPMT->usProgDescLen - CRC_BYTES);
        ulIndex += pCurPMT->usProgDescLen;
        pCurPMT->nBytesConsumed = (uint16)(pCurPMT->nBytesConsumed +
                                           pCurPMT->usProgDescLen);

        //! Parse Elementary Stream Descriptors
        pCurPMT->ESDescData = parseProgESDescriptors(pucDataBuf + ulIndex,
                                                     usESInfoSize, pContext);

        pCurPMT->nBytesConsumed = (uint16)(pCurPMT->nBytesConsumed + usESInfoSize);
        ulIndex += usESInfoSize;
        if((pCurPMT->section_no == pCurPMT->last_sect_no) &&
           (pCurPMT->common_sect_data.sect_length == pCurPMT->nBytesConsumed) )
        {
          pCurPMT->bProgParseComplete = true;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                       "parseProgMapTable prog# %u parsing complete",
                        pCurPMT->usProgNum);
        }
        retError = MP2STREAM_SUCCESS;
        if(bnewPMT)
        {
          if (pLastPMT)
          {
            pLastPMT->pNext = pCurPMT;
          }
          else
          {
            pMapSection = pCurPMT;
          }
        }
      } // if(bOK) --> New descriptor, parse it completely
    } //else (Section length is within limits)
  } // if(bOK, memory allocated successfully)

  //! bOK = false means either memory allocation not done or same PMT is found
  if (false == bOK)
  {
    freePMT(pCurPMT);
    pCurPMT = NULL;
  }
  else
  {
    // We cannot free memory allocated to pCurrProgMapSection
    // as it is still being used, KW might complain.
  }
  return retError;
}

/* ==========================================================================
@brief      Function to prepare Descriptors list.

@details    This function will prepare descriptors list from the given buf.

@param[in]      pucDataBuf      Data Buf pointer.
@param[in]      ulBytes         Data Buffer Size.

@return     Pointer of first entry in list will be returned in success case,
            Else, NULL pointer.
@note       None.
========================================================================== */
Desciptor* prepareDescriptorList(uint8* pucDataBuf, const uint32 ulDescLen)
{
  Desciptor *pHead = NULL;
  Desciptor *pDesc = NULL;
  uint32 ulIndex = 0;
  //! At least 6 bytes are required
  while(ulIndex + TS_DESC_HEADER_LEN <= ulDescLen )
  {
    Desciptor *pTemp = (Desciptor *)MM_Malloc(sizeof(Desciptor));
    //! Check whether memory is allocated or not
    if (!pTemp)
    {
      break;
    }
    memset(pTemp, 0, sizeof(Desciptor));
    //! Store current entry pointer in prev entry
    if (pDesc)
    {
      pDesc->pNext = pTemp;
    }
    //! Update pointer to hold current entry
    pDesc = pTemp;
    //! If it is first entry, update Head element in list
    if (!pHead)
    {
      pHead = pDesc;
    }
    pDesc->ucDescriptorTag    = pucDataBuf[ulIndex++];
    pDesc->ucDescriptorLength = pucDataBuf[ulIndex++];
    if (pDesc->ucDescriptorLength)
    {
      pDesc->pucDescrData = (uint8*)MM_Malloc(pDesc->ucDescriptorLength);
    }
    if (pDesc->pucDescrData)
    {
      memcpy(pDesc->pucDescrData, pucDataBuf + ulIndex,
             pDesc->ucDescriptorLength);
    }
    ulIndex += pDesc->ucDescriptorLength;
  }
  return pHead;
}

/* ==========================================================================
@brief      Function to parse Elementary Stream in PMT.

@details    This function will parse the ES descriptors list in PMT.

@param[in]      pucBuf        Data Buf pointer.
@param[in]      usBytes       Data Buf size.
@param[in]      pContext      Context pointer.

@return     Pointer of first entry in list will be returned in success case,
            Else, NULL pointer.
@note       None.
========================================================================== */
ESDescriptor* parseProgESDescriptors(uint8* pucDataBuf, uint16 usDescLen,
                                     MP2ParserContext* pContext)
{
  uint8  ucStreamType   = 0;
  uint16 usElemPID      = 0;
  uint16 ESInfoLen      = 0;
  uint16 usNumStreams   = 0;
  uint16 usIndex        = 0;
  ESDescriptor* pESInfo = NULL;
  ESDescriptor* pHead   = NULL;
  ESDescriptor* pTemp   = NULL;

  //! Loop to count number of streams in the program
  while(usIndex + TS_ELEMSTREAM_DESC_HEADER_LEN <= usDescLen)
  {
    ucStreamType = pucDataBuf[usIndex++];
    usElemPID    = pucDataBuf[usIndex++] & 0x1F;
    usElemPID    = (uint16)(usElemPID<<8);
    usElemPID    = (uint16)(usElemPID |pucDataBuf[usIndex++]);

    ESInfoLen = pucDataBuf[usIndex++] & 0x0F;
    ESInfoLen = (uint16)(ESInfoLen << 8);
    ESInfoLen = (uint16)(ESInfoLen | pucDataBuf[usIndex++]);

    usIndex   = (uint16)(usIndex + ESInfoLen);

    usNumStreams++;
  }
  //! Update total number of streams available in current program
  pContext->usNumStreams = usNumStreams;

  usIndex = 0;
  //! Update Elem-Stream and Stream-Info Structures
  while(usIndex + TS_ELEMSTREAM_DESC_HEADER_LEN <= usDescLen )
  {
    bool bIsValidStream = false;
    bool bIsAudio       = false;
    ucStreamType = pucDataBuf[usIndex++];
    usElemPID    = pucDataBuf[usIndex++] & 0x1F;
    usElemPID    = (uint16)(usElemPID <<8);
    usElemPID    = (uint16)(usElemPID | pucDataBuf[usIndex++]);

    ESInfoLen =  (uint16)(pucDataBuf[usIndex++] & 0x0F);
    ESInfoLen =  (uint16)(ESInfoLen << 8);
    ESInfoLen = (uint16)(ESInfoLen | pucDataBuf[usIndex++]);
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "parseProgESDescriptors type %d, PID %u ES Len %u",
                 ucStreamType, usElemPID, ESInfoLen);
    pTemp = (ESDescriptor*)MM_Malloc( sizeof(ESDescriptor) );
    if (!pTemp)
    {
      break;
    }
    if (pESInfo)
    {
      pESInfo->pNext = pTemp;
    }
    pESInfo = pTemp;
    if (!pHead)
    {
      pHead = pESInfo;
    }
    pESInfo->stream_type    = ucStreamType;
    pESInfo->elementary_pid = usElemPID;
    pESInfo->pNext          = NULL;

    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "parseProgESDescriptors elementary_pid %d stream_type %d",
                 usElemPID,ucStreamType);

    pESInfo->ES_info_length = ESInfoLen;
    pESInfo->pESDescList =
      prepareDescriptorList(pucDataBuf + usIndex, ESInfoLen);

    //! Check whether HDMV registration descriptor is available or not
    if (!pContext->bHDMVFlag)
    {
      pContext->bHDMVFlag = SearchRegDescriptor(pESInfo->pESDescList,
                                                  HDMV_REG_DESCRIPTOR);
    }
    media_codec_type eVidStrType = isVideoStreamType(ucStreamType);
    media_codec_type eAudStrType = isAudioStreamType(ucStreamType, pContext);

    if(  (UNKNOWN_AUDIO_VIDEO_CODEC != eVidStrType) &&
         (pContext->usSelectedProgNum) )
    {
      bIsAudio       = false;
      bIsValidStream = true;
    }
    else if( (UNKNOWN_AUDIO_VIDEO_CODEC != eAudStrType) &&
             (pContext->usSelectedProgNum) )
    {
      bIsAudio       = true;
      bIsValidStream = true;
    }
    else if((((ucStreamType >= TS_PSI_PVT_START_ID) &&
              (ucStreamType <= TS_PSI_PVT_END_ID ) ) ||
             (PES_PVT_STREAM_TYPE == ucStreamType)) &&
            (pContext->usSelectedProgNum))
    {
      if ( ( SearchRegDescriptor(pESInfo->pESDescList, AC3_REG_DESCRIPTOR)) ||
           ( FindDescriptor(pESInfo->pESDescList, TS_AC3_AUDIO_DESC_TAG))   ||
           ( FindDescriptor(pESInfo->pESDescList, AC3_SYS_B_AUDIO_DESC_TAG)))
      {
        bIsAudio       = true;
        bIsValidStream = true;
        ucStreamType   = AC3_AUDIO_STREAM_TYPE;
      }
      else if((SearchRegDescriptor(pESInfo->pESDescList,EAC3_REG_DESCRIPTOR))||
              (SearchRegDescriptor(pESInfo->pESDescList, EC3_REG_DESCRIPTOR)) ||
              (FindDescriptor(pESInfo->pESDescList, EC3_SYS_B_AUDIO_DESC_TAG)))
      {
        bIsAudio       = true;
        bIsValidStream = true;
        ucStreamType   = EAC3_AUDIO_STREAM_TYPE;
      }
      else if ((SearchRegDescriptor(pESInfo->pESDescList, VC1_REG_DESCRIPTOR)))
      {
        bIsAudio       = false;
        bIsValidStream = true;
        ucStreamType   = VC1_VIDEO_STREAM_TYPE;
      }
    }
    //! If it is a valid bitstream add it to the stream-info list
    if (bIsValidStream)
    {
      if((!pContext->usVideoPIDSelected) && (!bIsAudio))
      {
        pContext->usVideoPIDSelected = usElemPID;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"m_nVideoPIDSelected %u",
                     pContext->usVideoPIDSelected);
      }
      else if((!pContext->usAudioPIDSelected) && (bIsAudio))
      {
        pContext->usAudioPIDSelected = usElemPID;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"m_nAudioPIDSelected %u",
                     pContext->usAudioPIDSelected);
      }
    }
    //! Even if stream is unsupported, keep stream info in list
    pContext->usNumStreamsSelected++;
    (void)UpdateStreamInfo(ucStreamType, usElemPID, pContext);
    usIndex = (uint16)(usIndex + ESInfoLen);
  }//!while(usIndex + TS_ELEMSTREAM_DESC_HEADER_LEN <= usDescLen )
  return pHead;
}

/* ==========================================================================
@brief      Function to update stream info.

@details    This function will update stream info in the given context.

@param[in]  ucStreamType      Stream Type.
@param[in]  usPID             PID value.
@param[in]  pContext          Context ptr.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus UpdateStreamInfo(uint8 ucStreamType, uint16 usPID,
                                 MP2ParserContext* pContext)
{
  MP2StreamStatus retStatus = MP2STREAM_OUT_OF_MEMORY;
  uint16 usNumStreams = pContext->usNumStreams;
  stream_info* pStreamInfo = pContext->pStreamInfo;
  media_codec_type eStrType    = UNKNOWN_AUDIO_VIDEO_CODEC;
  media_codec_type eVidStrType = isVideoStreamType(ucStreamType);
  media_codec_type eAudStrType = isAudioStreamType(ucStreamType, pContext);

  //! Check if PID is already stored in list
  for(uint16 usIndex = 0; usIndex < usNumStreams; usIndex++)
  {
    if (!pStreamInfo)
    {
      break;
    }
    //check if we already stored this stream-id in our table
    if( (usPID == pStreamInfo[usIndex].stream_id) &&
        (TRACK_TYPE_UNKNOWN != pStreamInfo[usIndex].stream_media_type) )
    {
      return MP2STREAM_SUCCESS;
    }
  }

  if(UNKNOWN_AUDIO_VIDEO_CODEC != eAudStrType)
  {
    eStrType = eAudStrType;
    pContext->usNumAudioStreams++;
    //! To avoid multiple allocations, assume all streams as aud tracks only
    if(pContext->pAudioStreamIds == NULL)
    {
      pContext->pAudioStreamIds = (uint16*)
                                    MM_Malloc(usNumStreams * sizeof(uint16));
    }
    if(pContext->pAudioStreamIds)
    {
      pContext->pAudioStreamIds[pContext->usNumAudioStreams-1] = usPID;
    }
    if(!pContext->usAudioPIDSelected)
    {
      pContext->usAudioPIDSelected = usPID;
    }
  }//! if(UNKNOWN_AUDIO_VIDEO_CODEC != eAudStrType)
  else if(UNKNOWN_AUDIO_VIDEO_CODEC != eVidStrType)
  {
    eStrType = eVidStrType;
    pContext->usNumVideoStreams++;
    //! To avoid multiple allocations, assume all streams as vid tracks only
    if(pContext->pVideoStreamIds == NULL)
    {
      pContext->pVideoStreamIds = (uint16*)
                                    MM_Malloc(usNumStreams * sizeof(uint16));
    }
    if(pContext->pVideoStreamIds)
    {
      pContext->pVideoStreamIds[pContext->usNumVideoStreams-1] = usPID;
    }
    if(!pContext->usVideoPIDSelected)
    {
      pContext->usVideoPIDSelected = usPID;
    }
  }//! else if(UNKNOWN_AUDIO_VIDEO_CODEC != eVidStrType)
  //! Allocate memory to stream info structure if not already allocated
  if (NULL == pContext->pStreamInfo)
  {
    pContext->pStreamInfo = (stream_info*)
                            MM_Malloc(usNumStreams * sizeof(stream_info));
    if (pContext->pStreamInfo)
    {
      memset(pContext->pStreamInfo, 0, usNumStreams * sizeof(stream_info));
    }
  }

  if (pContext->pStreamInfo)
  {
    retStatus = UpdateCodecInfo(usPID, eStrType, pContext);
  }
  return retStatus;
}

/* ==========================================================================
@brief      Function to update codec info.

@details    This function will update stream info in the given context.

@param[in]  usPID             PID value.
@param[in]  eStreamType       Stream Type Enum.
@param[in]  pContext          Context ptr.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus UpdateCodecInfo(uint16 usPID, media_codec_type  eStreamType,
                                MP2ParserContext* pContext)
{
  MP2StreamStatus eStatus     = MP2STREAM_SUCCESS;
  stream_info*    pStreamInfo = pContext->pStreamInfo;

  bool   bUpdate      = false;
  bool   bPlayAudio   = pContext->bIsAudioPBInstance;
  bool   bPlayVideo   = pContext->bIsVideoPBInstance;
  //! Check whether user requested for codec config data or not.
  //! Also check whether it is video or audio instance.
  //! Do not parse audio metadata in video case and vice-versa.
  //! If the following flag is true, means metadata parsing is not required.
  bool   bAudioMeta   = !(pContext->bLocateCodecHdr && bPlayAudio);
  bool   bVideoMeta   = !(pContext->bLocateCodecHdr && bPlayVideo);
  uint16 usNumStreams = pContext->usNumStreams;
  uint16 usIndex      = 0;

  for(; (usIndex < usNumStreams) && (!bUpdate); usIndex++)
  {
    //find an unused entry and update the information
    if((TRACK_TYPE_UNKNOWN != pStreamInfo[usIndex].stream_media_type) ||
       (TRACK_TYPE_UNKNOWN == pStreamInfo[usIndex].stream_media_type &&
        pStreamInfo[usIndex].bParsed))
    {
      continue;
    }//!if(TRACK_TYPE_UNKNOWN != pStreamInfo[usIndex].stream_media_type)

    //Update the stream info with given pid
    pStreamInfo[usIndex].stream_id = usPID;

    //Now check if streamType belongs to audio/video
    switch(eStreamType)
    {
    case VIDEO_CODEC_MPEG4:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_VIDEO;
      pStreamInfo[usIndex].video_stream_info.Video_Codec = VIDEO_CODEC_MPEG4;
      pStreamInfo[usIndex].bParsed = bVideoMeta;
      bUpdate = true;
      break;
    case VIDEO_CODEC_VC1:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_VIDEO;
      pStreamInfo[usIndex].video_stream_info.Video_Codec = VIDEO_CODEC_VC1;
      pStreamInfo[usIndex].bParsed = bVideoMeta;
      bUpdate = true;
      break;
    case VIDEO_CODEC_MPEG2:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_VIDEO;
      pStreamInfo[usIndex].video_stream_info.Video_Codec = VIDEO_CODEC_MPEG2;
      pStreamInfo[usIndex].bParsed = bVideoMeta;
      bUpdate = true;
      break;
    case VIDEO_CODEC_H264:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_VIDEO;
      pStreamInfo[usIndex].video_stream_info.Video_Codec = VIDEO_CODEC_H264;
      pStreamInfo[usIndex].bParsed = bVideoMeta;
      bUpdate = true;
      break;
    case AUDIO_CODEC_AAC:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_AUDIO;
      pStreamInfo[usIndex].audio_stream_info.Audio_Codec = AUDIO_CODEC_AAC;
      pStreamInfo[usIndex].bParsed = bAudioMeta;
      bUpdate = true;
      break;
    case AUDIO_CODEC_LPCM:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_AUDIO;
      pStreamInfo[usIndex].audio_stream_info.Audio_Codec = AUDIO_CODEC_LPCM;
      pStreamInfo[usIndex].bParsed = bAudioMeta;
      bUpdate = true;
      break;
    case AUDIO_CODEC_HDMV_LPCM:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_AUDIO;
      pStreamInfo[usIndex].audio_stream_info.Audio_Codec = AUDIO_CODEC_HDMV_LPCM;
      pStreamInfo[usIndex].bParsed = bAudioMeta;
      bUpdate = true;
      break;
    case AUDIO_CODEC_AC3:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_AUDIO;
      pStreamInfo[usIndex].audio_stream_info.Audio_Codec = AUDIO_CODEC_AC3;
      pStreamInfo[usIndex].bParsed = bAudioMeta;
      bUpdate = true;
      break;
    case AUDIO_CODEC_EAC3:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_AUDIO;
      pStreamInfo[usIndex].audio_stream_info.Audio_Codec = AUDIO_CODEC_EAC3;
      pStreamInfo[usIndex].bParsed = bAudioMeta;
      bUpdate = true;
      break;
    case AUDIO_CODEC_DTS:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_AUDIO;
      pStreamInfo[usIndex].audio_stream_info.Audio_Codec = AUDIO_CODEC_DTS;
      pStreamInfo[usIndex].bParsed = bAudioMeta;
      bUpdate = true;
      break;
    case AUDIO_CODEC_MP3:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_AUDIO;
      pStreamInfo[usIndex].audio_stream_info.Audio_Codec = AUDIO_CODEC_MP3;
      pStreamInfo[usIndex].bParsed = bAudioMeta;
      bUpdate = true;
      break;
    case AUDIO_CODEC_MPEG2:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_AUDIO;
      pStreamInfo[usIndex].audio_stream_info.Audio_Codec = AUDIO_CODEC_MPEG2;
      pStreamInfo[usIndex].bParsed = bAudioMeta;
      bUpdate = true;
      break;
    default:
      pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_UNKNOWN;
      pStreamInfo[usIndex].audio_stream_info.Audio_Codec = UNKNOWN_AUDIO_VIDEO_CODEC;
      pStreamInfo[usIndex].bParsed = true;
      bUpdate = true;
      break;
    }//!switch(eStreamType)
  }//!for(usIndex < pContext->m_nstreams..)

  return eStatus;
}

/* ==========================================================================
@brief      Function to copy AVC codec config data into o/p buffer.

@details    Function to copy AVC Codec config data from Context.

@param[in]  pucBuf    I/p Buffer pointer.
@param[in]  pucSize   I/p Buffer size.
@param[in]  pContext  Context pointer.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
bool makeAVCVideoConfig(uint8* pBuf, uint32* pSize, MP2ParserContext* pContext)
{
  bool bRet = false;
  avc_codec_info* pAvcCodecInfo = pContext->pAVCCodecBuf;

  if(pAvcCodecInfo && pAvcCodecInfo->isValid && pSize)
  {
    *pSize = pAvcCodecInfo->size;

    if(pBuf)
    {
      memcpy(pBuf,pAvcCodecInfo->codecInfoBuf,pAvcCodecInfo->size);
    }
    bRet = true;
  }
  return bRet;
}

/*! ======================================================================
@brief  Checks id store to see if given id exists in the store

@detail    Checks id store to see if given id exists in the store

@param[in]  ulTrackId   Track Id
@param[in]  pContext    Context pointer

@return    true if id exists in id store else returns false
@note      None.
========================================================================== */
bool isTrackIdInIdStore(uint32 ulTrackId, MP2ParserContext* pContext)
{
  bool    bRet     = false;
  uint8   ucCount  = 0;
  uint16* pStreams = pContext->pAudioStreamIds;

  for( ; (pStreams) && ucCount < pContext->usNumAudioStreams; ucCount++)
  {
    if(pStreams[ucCount] == ulTrackId)
    {
      bRet = true;
      break;
    }
  }
  ucCount  = 0;
  pStreams = pContext->pVideoStreamIds;

  for(; (pStreams) && (ucCount< pContext->usNumVideoStreams); ucCount++)
  {
    if (bRet)
    {
      break;
    }
    if(pStreams[ucCount] == ulTrackId)
    {
      bRet = true;
      break;
    }
  }
  return bRet;
}

/* ==========================================================================
@brief      Function to parse CA Descriptor.

@details    This function will parse the given Descriptor and update
            the CA Descriptor structure.

@param[in]  pDescList       Descriptor Ptr list.
@param[in]  pCADesc         CA Descriptor Ptr.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseCADescriptor(Desciptor* pDesc, CADescriptor* pCADesc)
{
  uint32 ulIndex      = 0;
  uint32 ulDescrIndex = 0;
  uint8  ucBytes      = 0;
  uint8* pucBuf       = NULL;
  MP2StreamStatus retStatus = MP2STREAM_SUCCESS;
  if ((!pDesc) || (!pCADesc))
  {
    return MP2STREAM_INVALID_PARAM;
  }

  ucBytes = pDesc->ucDescriptorLength;
  pucBuf  = pDesc->pucDescrData;
  memset(pCADesc,0,sizeof(CADescriptor));

  uint16 usCASystemID = pucBuf[ulIndex++];
  usCASystemID = (uint16)( usCASystemID << 8);
  usCASystemID = (uint16)( usCASystemID | pucBuf[ulIndex++]);

  uint16 usCAPID  = pucBuf[ulIndex++] & 0x1F;
  usCAPID = (uint16)(usCAPID << 8);
  usCAPID = (uint16)(usCAPID | pucBuf[ulIndex++]);

  pCADesc->ucDescrTagID   = TS_CA_DESC_TAG;
  pCADesc->ucDescrLen     = ucBytes;
  pCADesc->usCASystemID   = usCASystemID;
  pCADesc->usCAPID        = usCAPID;
  pCADesc->usPvtDataBytes = uint16(ucBytes - ulIndex);
  pCADesc->pucDescrData   = (uint8*)MM_Malloc(ucBytes);

  //! Keep the whole descriptor data. This is provided to Client as it is
  if(pCADesc->pucDescrData)
    memcpy(pCADesc->pucDescrData, pucBuf, ucBytes);

  return retStatus;
}

/* ==========================================================================
@brief      Function to parse DTS Audio Descriptor.

@details    This function will parse given Descriptor and updates audio info
            structure.

  @param[in]  pDescList       Descriptor Ptr list.
  @param[in]  pAudioInfo      Audio info structure ptr.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseDTSAudioDescriptor(Desciptor* pDesc,
                                        audio_info* pAudioInfo)
{
  uint8 ucIndex = 0;
  MP2StreamStatus retStatus = MP2STREAM_DEFAULT_ERROR;

  uint8  ucBytes = pDesc->ucDescriptorLength;
  uint8* pucBuf  = pDesc->pucDescrData;
  if(pucBuf)
  {
    uint8 ucSampleRateCode = (pucBuf[ucIndex] & 0xf0) >> 4;
    uint8 ucBitrateCode = (uint8)((pucBuf[ucIndex] & 0x0f) | (pucBuf[ucIndex+1] & 0xc0));
    ucIndex++;
    uint8 ucNBlks =  (uint8)((pucBuf[ucIndex] & 0x3f) | (pucBuf[ucIndex+1] & 0x80));
    if((ucNBlks < 5) || (ucNBlks > 127))
    {
      retStatus = MP2STREAM_CORRUPT_DATA;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "parseDTSAudioDescriptor nblks not within range!!");
    }
    ucIndex++;
    uint32 ulFSize = (pucBuf[ucIndex] & 0x7f) | (pucBuf[ucIndex+1] & 0xfe);
    if((ulFSize < 95) || (ulFSize > 8192))
    {
      retStatus = MP2STREAM_CORRUPT_DATA;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "parseDTSAudioDescriptor fsize not within range!!");
    }
    ucIndex++;
    uint8 ucSurroundMode = (uint8)((pucBuf[ucIndex] & 0x01) | (pucBuf[ucIndex+1] & 0xf8));
    ucIndex++;
    uint8 ucLFEFlag = (pucBuf[ucIndex] & 0x04);
    uint8 ucExtendedSurroundFlag = (pucBuf[ucIndex] & 0x03);
    ucIndex++;
    uint8 ucComponentType = pucBuf[ucIndex];
    ucIndex++;

    if ((ucSampleRateCode < 15) && (ucBitrateCode < 21) &&
        (ucSurroundMode < 10))
    {
      pAudioInfo->Bitrate           = TS_DTS_BIT_RATE[ucBitrateCode];
      pAudioInfo->NumberOfChannels  = (uint8)TS_DTS_CHANNELS[ucSurroundMode];
      pAudioInfo->SamplingFrequency = TS_DTS_FSCODE_RATE[ucSampleRateCode];
      retStatus = MP2STREAM_SUCCESS;
    }
  }
  return retStatus;
}

/* ==========================================================================
@brief      Function to parse DTSHD Audio Descriptor.

@details    This function will parse given Descriptor and updates audio info
            structure.

@param[in]  pDescList       Descriptor Ptr list.
@param[in]  pAudioInfo      Audio info structure ptr.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseDTSHDAudioDescriptor(Desciptor* pDesc,
                                          audio_info* pAudioInfo)
{
  uint8  ucIndex = 0;
  uint8  ucBytes = 0;
  uint8* pucBuf  = NULL;
  DTSHDAudioDescriptor DTSHDDesc;
  MP2StreamStatus retStatus = MP2STREAM_DEFAULT_ERROR;

  if ((!pDesc) || (!pAudioInfo))
  {
    return MP2STREAM_INVALID_PARAM;
  }
  ucBytes = pDesc->ucDescriptorLength;
  pucBuf  = pDesc->pucDescrData;

  memset(&DTSHDDesc, 0, sizeof(DTSHDAudioDescriptor));

    DTSHDDesc.ucSubstreamCoreFlag = (pucBuf[ucIndex] & 0x80);
    DTSHDDesc.ucSubstream0Flag = (pucBuf[ucIndex] & 0x40);
    DTSHDDesc.ucSubstream1Flag = (pucBuf[ucIndex] & 0x20);
    DTSHDDesc.ucSubstream2Flag = (pucBuf[ucIndex] & 0x10);
    DTSHDDesc.ucSubstream3Flag = (pucBuf[ucIndex] & 0x08);
    ucIndex++;
    if(DTSHDDesc.ucSubstreamCoreFlag)
    {
      DTSHDSubstreamStruct sSubStream;
      memset(&sSubStream, 0, sizeof(DTSHDSubstreamStruct));

      sSubStream.ucSubstreamLength = pucBuf[ucIndex];
      ucIndex++;
      (void)parseDTSHDSubstreamInfo(pucBuf + ucIndex, &sSubStream,
                                    sSubStream.ucSubstreamLength);
      pAudioInfo->SamplingFrequency =
          TS_DTSHD_FSCODE_RATE[sSubStream.ucSamplingFrequency];
      pAudioInfo->NumberOfChannels = sSubStream.ucChannelCount;
      retStatus = MP2STREAM_SUCCESS;
    }

  return retStatus;
}

/* ==========================================================================
@brief      Function to parse DTS HD SubStream Info.

@details    This function will parse inside the given descriptor and updates
            SubStream Structure.

@param[in]  pucBuf                Buffer Ptr.
@param[in]  pSubstreamStruct      SubStream Structure Ptr.
@param[in]  ucLen                 Buffer Size.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseDTSHDSubstreamInfo(uint8* pucBuf,
                                        DTSHDSubstreamStruct* pSubstreamStruct,
                                        uint8 /*ucLen*/)
{
  uint8 ucIndex = 0;
  MP2StreamStatus retStatus = MP2STREAM_SUCCESS;

  pSubstreamStruct->ucNumAssets = (pucBuf[ucIndex] & 0xe0);
  pSubstreamStruct->ucChannelCount = (pucBuf[ucIndex] & 0x1f);
  ucIndex++;
  pSubstreamStruct->ucLFEFlag = (pucBuf[ucIndex] & 0x80);
  pSubstreamStruct->ucSamplingFrequency = (pucBuf[ucIndex] & 0x78);
  pSubstreamStruct->ucSampleResolution = (pucBuf[ucIndex] & 0x04);
  ucIndex++;

  return retStatus;
}

/* ==========================================================================
@brief      Function to parse DVD LPCM Audio Descriptor.

@details    This function will parse given Descriptor and updates audio info
            structure.

@param[in]  pDescList       Descriptor Ptr list.
@param[in]  pAudioInfo      Audio info structure ptr.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseDVDLPCMAudioDescriptor(Desciptor* pDesc,
                                            audio_info* pAudioInfo)
{
  uint8 ucIndex = 0;
  MP2StreamStatus retStatus = MP2STREAM_CORRUPT_DATA;

  if ((!pDesc) || (!pAudioInfo))
  {
    return MP2STREAM_INVALID_PARAM;
  }

  /* Size of descriptor as defined in the spec */
  const uint8 ucSizeDefined = 2;
  uint8  ucBytes = pDesc->ucDescriptorLength;
  //! First two bytes are used to store descriptor type and size info
  uint8* pucBuf  = pDesc->pucDescrData;

  /* If not equal, return corrupt data */
  if(ucSizeDefined == ucBytes)
  {
    uint8 ucSamplingFreq = (pucBuf[ucIndex] & 0xE0)>> 5;
    if( 1 == ucSamplingFreq )
    {
      pAudioInfo->SamplingFrequency = 44100;
    }
    else if(2 == ucSamplingFreq )
    {
      pAudioInfo->SamplingFrequency = 48000;
    }
    uint8 ucBitsPerSample = (uint8)((pucBuf[ucIndex] & 0x18) >> 3);
    if(!ucBitsPerSample)
    {
      pAudioInfo->ucBitsPerSample = 16;
    }

    ucIndex++;
    uint8 ucChannels = (pucBuf[ucIndex] & 0xE0) >> 5;
    if((!ucChannels ) || (1 == ucChannels ))
    {
      pAudioInfo->NumberOfChannels = 2;
    }
    retStatus = MP2STREAM_SUCCESS;
  }
  return retStatus;
}

/* ==========================================================================
@brief      Function to parse HDMV LPCM Audio Descriptor.

@details    This function will parse given Descriptor and updates audio info
            structure.

@param[in]  pDescList       Descriptor Ptr list.
@param[in]  pAudioInfo      Audio info structure ptr.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseHDMVLPCMAudioDescriptor(Desciptor* pDesc,
                                             audio_info* pAudioInfo)
{
  uint8 ucIndex = 0;
  MP2StreamStatus retStatus = MP2STREAM_CORRUPT_DATA;

  if ((!pDesc) || (!pAudioInfo))
  {
    return MP2STREAM_INVALID_PARAM;
  }

  /* Size of descriptor as defined in the spec */
  const uint8 ucSizeDefined = 8;
  uint8  ucBytes = pDesc->ucDescriptorLength;
  //! First two bytes are used to store descriptor type and size info
  uint8* pucBuf  = pDesc->pucDescrData;

  /* If not equal, return corrupt data */
  if((ucSizeDefined == ucBytes) &&
     (!memcmp(pucBuf, "HDMV", 4) ))
  {
    //! stuffing bytes
    ucIndex++;
    ucIndex += 4;
    if (HDMV_LPCM_STREAM_TYPE == pucBuf[ucIndex++])
    {
      pAudioInfo->Audio_Codec = AUDIO_CODEC_HDMV_LPCM;
      (void)parseHDMVLPCMHeader(pAudioInfo, pucBuf + ucIndex,
                                ucBytes - ucIndex);
      retStatus = MP2STREAM_SUCCESS;
    } //! if (HDMV_LPCM_STREAM_TYPE == pucBuf[ucIndex++])
  } //! if (!memcmp(pucBuf, "HDMV", 4))

  return retStatus;
}

/* ==========================================================================
@brief      Function to parse Language Descriptor.

@details    This function will parse given Descriptor and updates language
            info.

@param[in]  pDesc      Descriptor Ptr list.
@param[in]  pLang      Language Descriptor Ptr.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseLanguageDescriptor(Desciptor* pDesc,
                                        LanguageDescriptor *pLang)
{
  uint8 ucIndex = 0;
  MP2StreamStatus retStatus = MP2STREAM_SUCCESS;

  if ((!pDesc) || (!pLang) )
  {
    return MP2STREAM_INVALID_PARAM;
  }

  /* Minimum size of descriptor as defined in the spec */
  const uint8 ucSizeDefined = 4;
  uint8  ucBytes = pDesc->ucDescriptorLength;
  uint8* pucBuf  = pDesc->pucDescrData;

  if(ucBytes >= ucSizeDefined)
  {
    uint8 ucBytesConsumed = ucBytes;
    memcpy(pLang->ucISO639LangCode, pucBuf, 3);
    pLang->ucISO639LangCode[3] = '\0';
    ucIndex =(uint8)(ucIndex + 3*sizeof(uint8));
    pLang->ucAudioType = pucBuf[ucIndex++];
    ucBytesConsumed = (uint8)(ucBytesConsumed + ucIndex);
  }
  return retStatus;
}

/* ==========================================================================
@brief      Function to parse AC3 Audio Descriptor.

@details    This function will parse given Descriptor and updates audio info
            structure.

@param[in]  pDescList       Descriptor Ptr list.
@param[in]  pAudioInfo      Audio info structure ptr.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
MP2StreamStatus parseAC3AudioDescriptor(Desciptor* pDesc,
                                        audio_info* pAudioInfo)
{
  uint8 ucIndex = 0;
  MP2StreamStatus retStatus = MP2STREAM_SUCCESS;
  if ((!pDesc) || (!pAudioInfo))
  {
    return MP2STREAM_INVALID_PARAM;
  }

  /* Minimum size of AC3 descriptor is 4bytes. */
  const uint8 ucSizeDefined = 4;
  uint8       ucBytes       = pDesc->ucDescriptorLength;
  uint8*       pucBuf       = pDesc->pucDescrData;

  /* AC3 descriptor minimum size is 4bytes.  */
  if(ucSizeDefined <= ucBytes)
  {
    uint32 ulTotalBits    = ucBytes * 8;
    uint32 ulCurBitOffset = 0;
    uint8 ucSamplingRateCode = (uint8)
      GetBitsFromBuffer(3, ulCurBitOffset, pucBuf, ulTotalBits);
    ulCurBitOffset += 3;
    uint8 ucBSID = (uint8)
      GetBitsFromBuffer(5, ulCurBitOffset, pucBuf, ulTotalBits);
    ulCurBitOffset += 5;
    uint8 ucBitRateCode = (uint8)
      GetBitsFromBuffer(6, ulCurBitOffset, pucBuf, ulTotalBits);
    ulCurBitOffset += 6;
    uint8 ucSurrondMode = (uint8)
      GetBitsFromBuffer(2, ulCurBitOffset, pucBuf, ulTotalBits);
    ulCurBitOffset += 2;
    uint8 ucBSMod = (uint8)
      GetBitsFromBuffer(3, ulCurBitOffset, pucBuf, ulTotalBits);
    ulCurBitOffset += 3;
    uint8 ucNumChannels = (uint8)
      GetBitsFromBuffer(4, ulCurBitOffset, pucBuf, ulTotalBits);
    ulCurBitOffset += 4;

    /* As per specification, below values are recommended.
       In case of multiple values, we use first mentioned value in
       specifications.
        "000" 48
        "001" 44,1
        "010" 32
        "011" Reserved
        "100" 48 or 44,1
        "101" 48 or 32
        "110" 44,1 or 32
        "111" 48 or 44,1 or 32
    */
    pAudioInfo->SamplingFrequency = AC3_SAMPLE_RATE[ucSamplingRateCode];

    /* Bit rate requires 5bits only. Upper bit is used to indicate
       whether bit rate available is Nominal or Max Bitrate.
       However we use same value as bit rate in both cases. */
    if(ucBitRateCode < 19)
    {
       pAudioInfo->Bitrate = 1000 * AC3_BITRATE_CODE[ucBitRateCode];
    }
    else if((ucBitRateCode >= 32 ) &&
            (ucBitRateCode <= 50))
    {
      pAudioInfo->Bitrate = 1000 * AC3_BITRATE_CODE[ucBitRateCode - 32];
    }
    else
    {
      pAudioInfo->Bitrate = 0;
      retStatus = MP2STREAM_PARSE_ERROR;
    }
    //! If MSB is ZERO, then this field is similar to ACMOD value
    //! Else, fail the descriptor parsing so that Parser can read actual
    //! sample data to get the accurate number of channels.
    if (ucNumChannels < 8)
    {
      pAudioInfo->NumberOfChannels = (uint8)ACMOD_CHANNELS[ucNumChannels];
    }
    else
      retStatus = MP2STREAM_PARSE_ERROR;
  }
  else
  {
    retStatus = MP2STREAM_CORRUPT_DATA;
  }

  return retStatus;
}

/* ==========================================================================
@brief      Function to free PAT structure.

@details    This function will free the memory allocated to PAT Structure.

@param[in]      pPATSection        PAT pointer.

@return     Pointer of first entry in list will be returned in success case,
            Else, NULL pointer.
@note       None.
========================================================================== */
void freePAT(ProgramAssociationSection* pPATSection)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"MP2StreamParser::freePAT");
  if (!pPATSection)
  {
    return;
  }

  if(pPATSection->pusProgNum)
  {
    MM_Free(pPATSection->pusProgNum);
  }
  if(pPATSection->pusTSPID)
  {
    MM_Free(pPATSection->pusTSPID);
  }
  return;
}

/* ==========================================================================
@brief      Function to free PMT list.

@details    This function will free the memory allocated to PMT Structure.

@param[in]      pProgMapSection        PMT pointer.

@return     Pointer of first entry in list will be returned in success case,
            Else, NULL pointer.
@note       None.
========================================================================== */
void freePMT(ProgramMapSection* pCurPMT )
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"MP2StreamParser::freePMT");

  ProgramMapSection* pNextPMT = pCurPMT;
  while(pCurPMT)
  {
    ESDescriptor* pNext = NULL;
    ESDescriptor* pESDesc = pCurPMT->ESDescData;

    pNextPMT = pCurPMT->pNext;
    //! Run the loop to free all ES descriptors
    while(pESDesc)
    {
      pNext = pESDesc->pNext;
      FreeDescriptors(pESDesc->pESDescList);
      MM_Free(pESDesc);
      pESDesc = pNext;
    }
    FreeDescriptors(pCurPMT->pPIDescList);
    MM_Free(pCurPMT);
    pCurPMT = pNextPMT;
  }
  return;
}

/* ==========================================================================
@brief      Function to free Descriptor list.

@details    This function will free the memory allocated to Descriptors.

@param[in]  pDescList       Descriptor Ptr list.

@return     None.
@note       None.
========================================================================== */
void FreeDescriptors(Desciptor* pDescList)
{
  if (!pDescList)
  {
    return;
  }
  Desciptor* pNext = NULL;
  while(pDescList)
  {
    pNext = pDescList->pNext;
    if (pDescList->pucDescrData)
    {
      MM_Free(pDescList->pucDescrData);
    }
    MM_Free(pDescList);
    pDescList = pNext;
  }
  return;
}

/* ==========================================================================
@brief      Function to check whether given PID is PMT or not.

@details    This function will check whether PID is PMT or not.

@param[in]  usPID             PID value.
@param[in]  pusProgIndex      Program Index number.
@param[in]  pContext          Context ptr.

@return     True if PID belongs to PMT otherwise returns false.
@note       None.
========================================================================== */
bool isProgramMapPacket(uint16 usPID, uint16* pusProgramIndex,
                        MP2ParserContext* pContext)
{
  bool bRet = false;
  ProgramAssociationSection* pPATSection = &pContext->sPATSection;
  if(pusProgramIndex && pPATSection->bPATComplete && pPATSection->pusTSPID)
  {
    for (uint16 usIndex = 0; usIndex < pPATSection->nPrograms; usIndex++)
    {
      //0x0 program number is for network info table.
      if( (pPATSection->pusTSPID[usIndex] == usPID)&&
          (pPATSection->pusProgNum[usIndex] != 0x0) )
      {
        bRet = true;
        *pusProgramIndex = usIndex;
        break;
      }
    }
  }
  return bRet;
}

/* ==========================================================================
@brief      Function to check whether TS Packet has payload data or metadata.

@details    This function will return true, if TS Pkt contains PAT/PMT/CAT.

@param[in]      pContext        Context pointer.

@return     MP2STREAM_SUCCESS indicating metadata read successfully.
            Else, it will report corresponding error.
@note       None.
========================================================================== */
bool isPSI(uint32 ulTrackId, MP2ParserContext* pContext)
{
  bool bRet = false;
  uint16 usProgIndex = 0;
  if( (ulTrackId == TS_PROG_ASSOCIATION_TBL_PID) ||
      (ulTrackId == TS_CONDITIONAL_ACCESS_TBL_PID) ||
      (ulTrackId == TS_DESC_TBL_PID) )
  {
    bRet = true;
  }
  else if(isProgramMapPacket((uint16)ulTrackId, &usProgIndex, pContext))
  {
    bRet = true;
  }
  return bRet;
}
/* ==========================================================================
@brief      Function to check whether stream type is audio or not.

@details    Checks if streamType passed is one of the supported ones.

@param[in]  ucStreamType  Stream type value.
@param[in]  pContext      Context Ptr.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
media_codec_type isAudioStreamType(uint8 ucStreamType,
                                   MP2ParserContext* pContext)
{
  bool bRet = false;
  media_codec_type eStrType = UNKNOWN_AUDIO_VIDEO_CODEC;

  if ((pContext) && (pContext->bHDMVFlag) &&
      (ucStreamType >= HDMV_LPCM_STREAM_TYPE))
  {
    switch(ucStreamType)
    {
      /* Following Stream types are used if HDMV registration descriptor is
         defined. */
      case HDMV_LPCM_STREAM_TYPE:
        eStrType = AUDIO_CODEC_HDMV_LPCM;
        break;
      case HDMV_AC3_STREAM_TYPE:
      case HDMV_AC3_LOSSLESSSTREAM_TYPE:
        eStrType = AUDIO_CODEC_AC3;
        break;
      case HDMV_EAC3_STREAM_TYPE:
        eStrType = AUDIO_CODEC_EAC3;
        break;
      case HDMV_DTS_STREAM_TYPE:
      case DTS_HD_EXCEPT_XLLSTREAM_TYPE:
      case DTS_HD_STREAM_TYPE:
        eStrType = AUDIO_CODEC_DTS;
        break;
  //    case HDMV_DRA_STREAM_TYPE:
  //      eStrType = AUDIO_CODEC_DRA;
        break;
      default:
        break;
    } //! switch(ucStreamType)
  } //! if (m_bHDMVFlag && ucStreamType >= HDMV_LPCM_STREAM_TYPE)
  else
  {
    switch(ucStreamType)
    {
      case MPEG1_AUDIO_STREAM_TYPE:
        eStrType = AUDIO_CODEC_MP3;
        break;
      case MPEG2_AUDIO_STREAM_TYPE:
        eStrType = AUDIO_CODEC_MPEG2;
        break;
      case AAC_ADTS_STREAM_TYPE:
        eStrType = AUDIO_CODEC_AAC;
        break;
      case AC3_AUDIO_STREAM_TYPE:
        eStrType = AUDIO_CODEC_AC3;
        break;
      /* Private stream need not always be Audio track,
         This will be updated later. */
 //     case PES_PVT_STREAM_TYPE:
 //     case USER_PVT_STREAM_TYPE:
 //       eStrType = AUDIO_CODEC_DRA;
 //       break;
      case LPCM_AUDIO_STREAM_TYPE:
        eStrType = AUDIO_CODEC_LPCM;
        break;
      case DTS_HD_STREAM_TYPE:
      case HDMV_DTS_STREAM_TYPE:
        eStrType = AUDIO_CODEC_DTS;
        break;
#ifdef ATSC_COMPLIANCE
      case EAC3_AUDIO_STREAM_TYPE:
        eStrType = AUDIO_CODEC_EAC3;
        break;
#endif
      default:
        break;
    } //! switch(ucStreamType)
  } //! else of if (m_bHDMVFlag && ucStreamType >= HDMV_LPCM_STREAM_TYPE)
  return eStrType;
}

/* ==========================================================================
@brief      Function to check whether stream is video or not.

@details    Checks if streamType passed is one of the supported ones.

@param[in]  ucStreamType  Stream type value.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
media_codec_type isVideoStreamType(uint8 ucStreamType)
{
  media_codec_type eStrType = UNKNOWN_AUDIO_VIDEO_CODEC;
  switch(ucStreamType)
  {
    case VC1_VIDEO_STREAM_TYPE:
      eStrType = VIDEO_CODEC_VC1;
      break;
    case MPEG4_VIDEO_STREAM_TYPE:
      eStrType = VIDEO_CODEC_MPEG4;
      break;
    case MPEG2_VIDEO_STREAM_TYPE:
      eStrType = VIDEO_CODEC_MPEG2;
      break;
    case AVC_VIDEO_STREAM_TYPE:
      eStrType = VIDEO_CODEC_H264;
      break;
    default:
      break;
  }
  return eStrType;
}

/* ==========================================================================
@brief      Function to check whether given PID is PMT or not.

@details    Inspects Data content to determine
            if it's a PES packet start code.

@param[in]  pucDataBuf      Data Buf Ptr.
@param[in]  pulValue        PES Packet type.

@return     True if Data represents PES packet otherwise returns false.
@note       None.
========================================================================== */
bool isPESPacket(uint8* pucDataBuf,uint32* pulValue)
{
  bool bRet = false;
  if(pucDataBuf && pulValue)
  {
    //read 4 bytes value including PES packet start code
    *pulValue = getBytesValue(4,pucDataBuf);
    uint32 startCode = getBytesValue(3,pucDataBuf);
    if(startCode == PES_PKT_START_CODE)
    {
      //mask off PES start code (first 24 bits)and get just the stream id
      *pulValue = (*pulValue) & 0x000000FF;

      if( (*pulValue ==  PROG_STREAM_MAP_ID)                                             ||
        (*pulValue == PRIVATE_STREAM1_ID)                                                ||
        (*pulValue == PADDING_STREAM_ID)                                                 ||
        (*pulValue == PRIVATE_STREAM2_ID)                                                ||
        ( (*pulValue >= AUDIO_STREAM_ID_START) && (*pulValue <= AUDIO_STREAM_ID_END) )        ||
        ( (*pulValue >= VIDEO_STREAM_ID_START) && (*pulValue <= VIDEO_STREAM_ID_END) )        ||
        ( (*pulValue >= RES_DATA_STREAM_START_ID) && (*pulValue <= RES_DATA_STREAM_END_ID ) ) ||
        (*pulValue == ECM_STREAM_ID)                                                     ||
        (*pulValue == EMM_STREAM_ID)                                                     ||
        (*pulValue == DSMCC_STREAM_ID)                                                   ||
        (*pulValue == ISO_OEC_13522_STREAM_ID)                                           ||
        (*pulValue == ANCILLARY_STREAM_ID )                                              ||
        (*pulValue == SL_PACKETIZED_STREAM_ID)                                           ||
        (*pulValue == FLEX_MUX_STREAM_ID )                                               ||
        (*pulValue == H222_TYPE_A_STREAM_ID)                                             ||
        (*pulValue == H222_TYPE_B_STREAM_ID)                                             ||
        (*pulValue == H222_TYPE_C_STREAM_ID)                                             ||
        (*pulValue == H222_TYPE_D_STREAM_ID)                                             ||
        (*pulValue == H222_TYPE_E_STREAM_ID)                                             ||
        (*pulValue == PROG_STREAM_DIRECTORY_ID)   )
      {
        bRet = true;
      }
    }
  }
  return bRet;
}

/*! ======================================================================
@brief      Parses video meta data.

@detail     Parses video meta data from currently parsed PES packet
            retrieve resolution, aspect ratio etc.

@param[in]  pContext      Context Pointer

@return     true if meta data is parsed successfully otherwise returns false
@note       None.
========================================================================== */
bool parseVideoMetaData(MP2ParserContext* pContext)
{
  bool bRet = false;

  uint32 ulIndex = 0;
  uint8 ucVal = 0;
  bool bContinue = true;
  uint16 usPID = pContext->sTSPacket.PID;

  uint8* pucBuf     = pContext->pucBuf;
  uint32 ulDataRead = pContext->ulDataRead;

  ESDescriptor* pESDesc     =  NULL;
  video_info*   pVideoInfo  = NULL;
  stream_info*  pStreamInfo = pContext->pStreamInfo;

  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"parseVideoMetaData");

  if (pContext->bProgramStream)
  {
    usPID = pContext->pCurrPESPkt->trackid;
    //! If System header is not available, then do not update trackInfo
    if (NULL == pContext->pPackHdr->sys_header)
    {
      bContinue = false;
    }
    //! If PSM is not available, then update video track Info details
    else if((!pContext->pProgramStreamMap) && (!pContext->usNumVideoStreams))
    {
      uint32 ulCodeVal    = getBytesValue(4, pucBuf);
      uint8  ucStreamType = 0xFF;

      if ((FULL_SEQUENCE_HEADER_CODE == ulCodeVal ) ||
          (FULL_USER_DATA_START_CODE == ulCodeVal ) ||
          (FULL_GROUP_START_CODE == ulCodeVal ) ||
          (FULL_SEQUENCE_ERROR_CODE == ulCodeVal ) ||
          (FULL_EXTENSION_START_CODE == ulCodeVal ) ||
          (FULL_SEQUENCE_END_CODE == ulCodeVal ) )
      {
        ucStreamType = MPEG2_VIDEO_STREAM_TYPE;
      }
      else if ((AVC_4BYTE_START_CODE == ulCodeVal ) ||
               (AVC_3BYTE_START_CODE == ulCodeVal ))
      {
        pContext->bIsH264 = true;
        ucStreamType = AVC_VIDEO_STREAM_TYPE;
      }
      else if (( MPEG4_VO_SEQ == ulCodeVal) ||
               ( MPEG4_VOP_FRAME_START_CODE == ulCodeVal))
      {
        ucStreamType = MPEG4_VIDEO_STREAM_TYPE;
      }
      /* Update track properties. */
      UpdateStreamInfo(ucStreamType, usPID, pContext);
    }//! else if(!pContext->pProgramStreamMap && !pContext->m_nVideoStreams)
  }//! if (pContext->bProgramStream)

  for (uint32 ulCounter = 0; ulCounter < pContext->usNumStreams; ulCounter++)
  {
    if(pStreamInfo[ulCounter].stream_id == usPID)
    {
      pStreamInfo = pStreamInfo + ulCounter;
      pVideoInfo  = &(pStreamInfo[0].video_stream_info);
      break;
    }
  }

  //! If Metadata is already parsed, then skip
  //! If stream info is not allocated or not a valid video stream id, then skip
  if((!pStreamInfo) || (!pVideoInfo) ||(true == pStreamInfo->bParsed))
  {
    return true;
  }

  if(pContext->pMapSection && (pContext->pMapSection->ESDescData))
  {
    pESDesc = pContext->pMapSection->ESDescData;
    while(pESDesc)
    {
      if(AVC_VIDEO_STREAM_TYPE == pESDesc->stream_type)
      {
        pVideoInfo->Video_Codec = VIDEO_CODEC_H264;
        pStreamInfo->bParsed = true;
        pContext->bIsH264 = true;
        bRet = true;
        bContinue = false;
        break;
      }
      else if (MPEG4_VIDEO_STREAM_TYPE == pESDesc->stream_type)
      {
        int32 slHieght = 0, slWidth = 0;
        pVideoInfo->Video_Codec = VIDEO_CODEC_MPEG4;
        bRet = true;
        bContinue = false;
        bool bStatus = GetDimensionsFromVOLHeader(pContext->pucBuf,
                                                  pContext->ulDataRead,
                                                  &slWidth, &slHieght);
        if (bStatus)
        {
          pVideoInfo->Height   = slHieght;
          pVideoInfo->Width    = slWidth;
          pStreamInfo->bParsed = true;
        }
        break;
      }
      pESDesc = pESDesc->pNext;
    }
  }//! if(pContext->pMapSection && (pContext->pMapSection->ESDescData))

  //! H264 codec config is handled in different way
  if (VIDEO_CODEC_H264 == pVideoInfo->Video_Codec)
  {
    pStreamInfo->bParsed = true;
    pContext->bIsH264    = true;
    bContinue = false;
  }
  if(bContinue)
  {
    while((bContinue) && (ulIndex < ulDataRead))
    {
      uint32 ulStartCodeVal = getBytesValue(3, pucBuf + ulIndex);
      //make sure there is video prefix start code
      if(VIDEO_START_CODE_PREFIX == ulStartCodeVal)
      {
        ulIndex += 3;
        if(USER_DATA_START_CODE == pucBuf[ulIndex])
        {
          ulIndex++;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"USER_DATA_START_CODE found");

          ulStartCodeVal = getBytesValue(4,pucBuf + ulIndex);
          if(AFD_START_CODE == ulStartCodeVal)
          {
            ulIndex += 4;
            getByteFromBitStream(&ucVal,&pucBuf[ulIndex],0,1);
            if(ucVal != 0)
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"AFD corrupted");
            }
            else
            {
              getByteFromBitStream(&ucVal,&pucBuf[ulIndex],1,1);
              if(ucVal == 1)
              {
                getByteFromBitStream(&ucVal,&pucBuf[ulIndex],11,4);
              }
            }
          }
        }
        else if(PICTURE_START_CODE == pucBuf[ulIndex])
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"PICTURE_START_CODE found");
          ulIndex += 2;
        }
        else if(GROUP_START_CODE == pucBuf[ulIndex])
        {
          ulIndex++;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"GROUP_START_CODE found");
        }
        else if(SEQUENCE_HEADER_CODE == pucBuf[ulIndex])
        {
          //skip bytes to get the resolution.
          ulIndex++;

          //next 3 bytes contains (12 bits)width and (12 bits)height
          uint16 width1 = (uint16)(((uint16)pucBuf[ulIndex])<<4);
          uint16 width2 = ((uint16)(pucBuf[ulIndex+1] & 0xF0))>>4;
          uint16 usWidth = width1 | width2;
          uint16 height1 = (uint16)(((uint16)(pucBuf[ulIndex+1] & 0x0F))<<8);
          uint16 usHeight  = (uint16)(height1 | pucBuf[ulIndex+2]);
          ulIndex += 3;

          //next byte gives aspect ratio and frame rate
          uint8 ucAspectRatio = (pucBuf[ulIndex]& 0xF0)>>4;
          uint8 ucFrameRate   = (pucBuf[ulIndex]& 0x0F);
          ulIndex++;

          //18 bits nominal bit-rate
          uint32 ulBitRate = (((uint32)pucBuf[ulIndex])<< 8) |
                              (pucBuf[ulIndex + 1]);
          ulBitRate <<= 2;
          ulIndex += 2;

          //get remaining 2 bits for nominal bit-rate
          ulBitRate |= ((pucBuf[ulIndex] & 0xC0) >> 6);

          //10 bits are for VBV buffer size and 6 bits are reserved..
          ulIndex += 2;

          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"Aspect Ratio : %d(REFER to ENUM VALUES)",ucAspectRatio);
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"Bit-Rate     : %lu",ulBitRate);
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"Frame Rate   : %d(REFER to ENUM VALUES)",ucFrameRate);
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"Height       : %u",usHeight);
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"Width        : %u",usWidth);

          pVideoInfo->Aspect_Ratio = (video_aspect_ratio)ucAspectRatio;
          pVideoInfo->Frame_Rate   = (video_frame_rate)ucFrameRate;
          pVideoInfo->Height       = usHeight;
          pVideoInfo->Width        = usWidth;
          pVideoInfo->Video_Codec  = VIDEO_CODEC_MPEG2;
          pStreamInfo->bitRate     = ulBitRate;
          pStreamInfo->bParsed     = true;
          break;
        }//! else if(SEQUENCE_HEADER_CODE == pucBuf[ulIndex])
        else
        {
          ulIndex++;
        }
      }//! if(VIDEO_START_CODE_PREFIX == ulStartCodeVal)
      else
      {
        ulIndex += 1;
      }
    }//! while((bContinue) && (ulIndex < ulDataRead))
  }//! if(bContinue)
  return bRet;
}

/* ==========================================================================
@brief      Function to update audio metadata if required.

@details    Parses audio meta data from currently parsed PES packet
            to retrieve sampling frequency, number of channels etc.

@param[in]  pContext      Context Ptr.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
bool parseAudioMetaData(MP2ParserContext* pContext)
{
  ProgramMapSection*     pMAPSection = pContext->pMapSection;
  MP2TransportStreamPkt* pTSPacket   = &pContext->sTSPacket;
  PESPacket*             pPESPacket  = pContext->pCurrPESPkt;

  uint8* pucBuf     = pContext->pucBuf;
  uint32 ulDataRead = pContext->ulDataRead;

  stream_info*  pStreamInfo = pContext->pStreamInfo;
  ESDescriptor* pESDesc     = NULL;
  audio_info*   pAudioInfo  = NULL;

  bool bRet             = false;
  bool bESStreamTypeSet = true;
  uint16 usPID          = pContext->sTSPacket.PID;
  uint32 ulBufIndex     = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"parseAudioMetaData");

  /* Check whether audio track/codec info is updated properly or not. */
  if (pContext->bProgramStream && pContext->pPackHdr->sys_header)
  {
    usPID = pContext->pCurrPESPkt->trackid;
    UpdateAudioInfoInPS(pContext);
  }
  for (uint32 ulCounter = 0; ulCounter < pContext->usNumStreams; ulCounter++)
  {
    if((pStreamInfo) && (pStreamInfo[ulCounter].stream_id == usPID))
    {
      pStreamInfo = pStreamInfo + ulCounter;
      pAudioInfo  = &pStreamInfo->audio_stream_info;
      break;
    }
  }
  if ((!pStreamInfo) || (!pAudioInfo) || (pStreamInfo->bParsed) )
  {
     return true;
  }
  if(pMAPSection && (pMAPSection->ESDescData))
  {
    pESDesc = pMAPSection->ESDescData;
    while(pESDesc)
    {
      bRet = false;
      if (pESDesc->elementary_pid != usPID)
      {
        pESDesc = pESDesc->pNext;
        continue;
      }
      //! If PES Packet is encrypted, then do not look for metadata
      //! Report Success without any codec properties.
      if (true == pPESPacket->pes_extn_hdr.pes_extn_pvt_data_flag)
      {
        bRet = true;
      }
      else if(AAC_ADTS_STREAM_TYPE == pESDesc->stream_type )
      {
        bRet = parseAACHeader(pAudioInfo, pucBuf, ulDataRead);
      }
      else if((pESDesc->stream_type == MPEG1_AUDIO_STREAM_TYPE)||
              (pESDesc->stream_type == MPEG2_AUDIO_STREAM_TYPE))
      {
        bRet = ParseMPGAudioHeader(pAudioInfo, pucBuf, ulDataRead);
      }
      else if(LPCM_AUDIO_STREAM_TYPE == pESDesc->stream_type)
      {
        bRet = parseLPCMHeader(pAudioInfo, pucBuf, ulDataRead);
      }
      else if (HDMV_LPCM_STREAM_TYPE == pESDesc->stream_type)
      {
        //! First Two bytes contain AU size, ignore those two bytes
        bRet = parseHDMVLPCMHeader(pAudioInfo, pucBuf + 2, ulDataRead);
      }
      else if(AC3_AUDIO_STREAM_TYPE == pESDesc->stream_type)
      {
        bRet = parseAC3Header(pAudioInfo, pucBuf, ulDataRead);
      }
      else if( (pESDesc->stream_type == HDMV_DTS_STREAM_TYPE) ||
               (pESDesc->stream_type == DTS_HD_STREAM_TYPE) )
      {
        bRet = parseDTSHeader(pAudioInfo, pucBuf, ulDataRead);
      }
      else if(PES_PVT_STREAM_TYPE == pESDesc->stream_type )
      {
        bRet = parseAC3Header(pAudioInfo, pucBuf, ulDataRead);
        if (false == bRet)
        {
          bRet = parseDTSHeader(pAudioInfo, pucBuf, ulDataRead);
          if (false == bRet)
          {
            bRet = parseLPCMHeader(pAudioInfo, pucBuf, ulDataRead);
            if (false == bRet)
            {
              bESStreamTypeSet = false;
              break;
            }
          }
        }
      } // else if(pESDesc->stream_type == PES_PVT_STREAM_TYPE)

      if (bRet)
      {
        pStreamInfo->bitRate = pAudioInfo->Bitrate;
        pStreamInfo->bParsed = true;
        break;
      }
      pESDesc = pESDesc->pNext;
    } // while(pESDesc)

    //! What if PES Private stream is not mapped with any proper codec types ?
    if (false == bESStreamTypeSet)
    {
      pContext->usNumStreams--;
      pContext->usNumAudioStreams--;
      pContext->usNumStreamsSelected--;
      pStreamInfo->bParsed = true;
    }
  } // if(m_ProgMapSection && (m_ProgMapSection->ESDescData))

  if(pContext->bProgramStream)
  {
    bool bIsValid  = true;
    bool bContinue = true;
    if (!pAudioInfo)
    {
      return false;
    }

    //! If codec type is not known, then check whether it is MP3/AAC
    //! Both have same Sync marker, so validate two successive frames before
    //! marking codec type as AAC or MP3
    if (UNKNOWN_AUDIO_VIDEO_CODEC == pAudioInfo->Audio_Codec &&
        PRIVATE_STREAM1_ID != pStreamInfo->stream_id)
    {
      bRet = parseAACHeader(pAudioInfo, pucBuf, ulDataRead);
      if (false == bRet)
      {
        bIsValid = false;
        pAudioInfo->Audio_Codec = AUDIO_CODEC_MP3;
      }
      else
      {
        /*Copy first frame properties into class variable and compare them with
          second frame properties, they should match. Else we can treat
          bit-stream as MPG compliant. */
        if(0 == pContext->sAACAudioInfo.SamplingFrequency)
        {
          memcpy(pAudioInfo, &pContext->sAACAudioInfo, sizeof(audio_info));
        }
        else if((pContext->sAACAudioInfo.SamplingFrequency !=
                 pAudioInfo->SamplingFrequency) ||
                (pContext->sAACAudioInfo.AudioObjectType   !=
                 pAudioInfo->AudioObjectType) )
        {
          bIsValid = false;
          pAudioInfo->Audio_Codec = AUDIO_CODEC_MP3;
        }
        else
        {
          pAudioInfo->Audio_Codec = AUDIO_CODEC_AAC;
        }
      }
    }
    if( AUDIO_CODEC_AAC == pAudioInfo->Audio_Codec)
    {
      bRet = parseAACHeader(pAudioInfo, pucBuf, ulDataRead);
      if(true == bRet)
      {
        pStreamInfo->bParsed = true;
      }
    }
    else if(AUDIO_CODEC_MP3   == pAudioInfo->Audio_Codec ||
            (AUDIO_CODEC_MPEG2 == pAudioInfo->Audio_Codec))
    {
      bRet = ParseMPGAudioHeader(pAudioInfo, pucBuf, ulDataRead);
      if(bRet)
      {
        pStreamInfo->bParsed = true;
        bIsValid = true;
      }
    }
    //Audio Sub-stream header for AC3 does not exist for TS
    else if((ulBufIndex = IsBitStreamAC3Complaint(pucBuf, ulDataRead)) &&
           (ulBufIndex))
    {
      bRet = parseAC3Header(pAudioInfo, pucBuf + ulBufIndex,
                            ulDataRead - ulBufIndex);
      if(bRet)
      {
        pAudioInfo->Audio_Codec = AUDIO_CODEC_AC3;
        pStreamInfo->bitRate    = pAudioInfo->Bitrate;
        pStreamInfo->bParsed    = true;
      }
    } //else AC3 section
    if((DVD_LPCM_FRAME_START_CODE == pucBuf[0]) &&
       (UNKNOWN_AUDIO_VIDEO_CODEC == pAudioInfo->Audio_Codec ))
    {
      bool bRet = parseDVDLPCMHeader(pAudioInfo, pucBuf,
                                     ulDataRead);
      if(bRet)
      {
        pAudioInfo->Audio_Codec = AUDIO_CODEC_LPCM;
        pStreamInfo->bParsed = true;
      }
    }
    if((!memcmp(pucBuf, (void*)DTS_SYNCWORD_MPG_PS, FOURCC_SIGNATURE_BYTES)) &&
       (UNKNOWN_AUDIO_VIDEO_CODEC == pAudioInfo->Audio_Codec ))
    {
      // Move ahead by size of FOURCC
      ulBufIndex+=FOURCC_SIGNATURE_BYTES;
      bool bRet = parseDTSHeader(pAudioInfo, pucBuf +ulBufIndex,
                                 ulDataRead);
      if(bRet)
      {
              pAudioInfo->Audio_Codec = AUDIO_CODEC_DTS;
              pStreamInfo->bitRate = pAudioInfo->Bitrate;
              pStreamInfo->bParsed = true;
      }
    }
    //! If bit-stream is not complaint to any standard format,t, then mark
    //! stream as unknown type.
    if(false == bIsValid)
    {
      bRet = false;
      pContext->usNumStreams--;
      pContext->usNumStreamsSelected--;
      pContext->usNumAudioStreams--;
    }
  } // if(m_bProgramStream)

  return bRet;
}

/* ==========================================================================
@brief      Function to update audio stream id info in Program Stream.

@details    This function updates total number of audio streams.

@param[in]  pContext        Context Ptr.

@return     None.
@note       None.
========================================================================== */
void UpdateAudioInfoInPS(MP2ParserContext* pContext)
{
  uint16 usPID = pContext->pCurrPESPkt->trackid;
  pack_header* pPackHdr    = pContext->pPackHdr;
  stream_info* pStreamInfo = pContext->pStreamInfo;
  /* If PSM (Program Stream Map) is not present in Program Streams, then parser
     has not updated total tracks and other useful information. So by using
     system header info, update track properties and other required info. */
  if (pContext->pProgramStreamMap)
  {
    return;
  }
  uint32 ulTotalTracks = pPackHdr->sys_header->audio_bound +
                         pPackHdr->sys_header->video_bound;
  if ((pStreamInfo) &&
      (pContext->usNumAudioStreams < pPackHdr->sys_header->audio_bound)  )
  {
    uint32 ulCount = 0;
    pContext->usNumStreams = (uint8)ulTotalTracks;

    //! Check the structure for uninitialized entry
    while((TRACK_TYPE_UNKNOWN != pStreamInfo[ulCount++].stream_media_type) &&
          (ulCount < ulTotalTracks));

    pStreamInfo[ulCount-1].stream_id = usPID;
    if (!pContext->pAudioStreamIds)
    {
      pContext->pAudioStreamIds = (uint16*)MM_Malloc(ulTotalTracks *
                                                     sizeof(uint16));
    }
    if(pContext->pAudioStreamIds)
    {
      pContext->pAudioStreamIds[pContext->usNumAudioStreams] = usPID;
    }
    pStreamInfo[ulCount-1].stream_media_type = TRACK_TYPE_AUDIO;
    pContext->usNumAudioStreams++;
    if (!pContext->usAudioPIDSelected)
    {
      pContext->usAudioPIDSelected = usPID;
    }
  }
  return;
}

  /* ==========================================================================
  @brief      Function to update MPG meta data in audio info structure.

  @details    Parses MPG meta data from currently parsed PES packet
              to retrieve sampling frequency, number of channels etc.

  @param[in]  pAudioInfo    Audio info structure pointer.
  @param[in]  pucBuf        Data Buf Ptr.
  @param[in]  ulBufSize     Data Buf Size.

  @return     True if successful, otherwise returns false.
  @note       None.
  ========================================================================== */
bool ParseMPGAudioHeader(audio_info* pAudioInfo,
                         uint8* pucBuf, uint32 /*ulBufSize*/)
{
  bool bRet = true;

  if((0xFF != pucBuf[0]) && (0xE0 != (pucBuf[1] & 0xE0)))
  {
    return false;
  }
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
              "parseAudioMetaData MPEG AUDIO SYNC WORD");

  mp3_ver_enum_type eVersion = (mp3_ver_enum_type)
    ((pucBuf[MP3HDR_VERSION_OFS] & MP3HDR_VERSION_M) >> MP3HDR_VERSION_SHIFT);

  if (!(MP3_VER_25 == eVersion ||
        MP3_VER_2  == eVersion ||
        MP3_VER_1  == eVersion))
  {
    bRet = false;
  }
  mp3_layer_enum_type eLayer = (mp3_layer_enum_type)
    ((pucBuf[MP3HDR_LAYER_OFS] & MP3HDR_LAYER_M) >> MP3HDR_LAYER_SHIFT);

  uint8 ucSamplingFreqIndex =  (uint8)((pucBuf[MP3HDR_SAMPLERATE_OFS] &
                              MP3HDR_SAMPLERATE_M) >> MP3HDR_SAMPLERATE_SHIFT);

  if(bRet)
  {
    //! Number of channels
    switch ((pucBuf[3] & 0x30) >> 4)
    {
      case MP3_CHANNEL_STEREO:
      case MP3_CHANNEL_JOINT_STEREO:
      case MP3_CHANNEL_DUAL:
        pAudioInfo->NumberOfChannels = 2;
        break;
      case MP3_CHANNEL_SINGLE:
      default:
        pAudioInfo->NumberOfChannels = 1;
        break;
    }

    pAudioInfo->SamplingFrequency = MP3_SAMPLING_RATE[eVersion]
                                                     [ucSamplingFreqIndex];
    if (MP3_LAYER_3 == eLayer)
    {
      pAudioInfo->Audio_Codec = AUDIO_CODEC_MP3;
    }
    else if(MP3_LAYER_2 == eLayer)
    {
      pAudioInfo->Audio_Codec  = AUDIO_CODEC_MPEG2;
    }
    else if (MP3_LAYER_1 == eLayer)
    {
      pAudioInfo->Audio_Codec  = AUDIO_CODEC_MPEG1;
    }
    else
    {
      bRet = false;
    }
  }
  return bRet;
}

/* ==========================================================================
@brief      Function to check whether bit-stream is AC3 format or not.

@details    This function checks whether sub-stream id is AC3 id or not.

@param[in]  pucBuf        Data Buf Ptr.
@param[in]  ulBufSize     Data Buf Size.

@return     Index from where AC3 bit-stream will start.
            ZERO indicates bit-stream is not AC3 complaint.
@note       None.
========================================================================== */
uint16 IsBitStreamAC3Complaint(uint8* pucBuf, uint32 /*ulSize*/)
{
  uint16 usBufIndex = 0;
  if( (pucBuf[usBufIndex] >= AC3_AUDIO_SUBSTREAM_ID_BEG) &&
      (pucBuf[usBufIndex] <= AC3_AUDIO_SUBSTREAM_ID_END) )
  {
    usBufIndex++;
    //next byte gives number of frame headers
    uint8 ucFrameHeaders = pucBuf[usBufIndex++];
    //next 2 bytes represents first access unit pointer
    uint16 usFirstAUPtr = (uint16)((pucBuf[usBufIndex] <<8)|
                                    pucBuf[usBufIndex+1]);
    usBufIndex = (uint16)(usBufIndex + 2);

    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                 "AC3 ucFrameHeaders %d and usFirstAUPtr %u",
                 ucFrameHeaders, usFirstAUPtr);
    //! "usFirstAUPtr" contains the amount of data that needs to be skipped
    //! to go to frame start
    usBufIndex = (uint16)(usBufIndex + usFirstAUPtr - 1);
    uint16 usAC3Sync  = (uint16)((pucBuf[usBufIndex] <<8) |
                                  pucBuf[usBufIndex+1]);
    if( AC3_SYNC_WORD != usAC3Sync)
    {
      usBufIndex = 0;
    }
  }
  return usBufIndex;
}

/* ==========================================================================
@brief      Function to update LPCM data in audio info structure.

@details    Parses LPCM meta data from currently parsed PES packet
            to retrieve sampling frequency, number of channels etc.

@param[in]  pAudioInfo    Audio info structure pointer.
@param[in]  pucBuf        Data Buf Ptr.
@param[in]  ulBufSize     Data Buf Size.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
bool parseLPCMHeader(audio_info* pAudioInfo,
                     uint8* pucBuf, uint32 /*ulBufSize*/)
{
  bool bRet     = true;
  uint8 ucIndex = 0;
  uint8 ucVal   = 0;

  pAudioInfo->Audio_Codec = AUDIO_CODEC_LPCM;

  /* Check whether sub-stream ID is with LPCM range or not. */
  if((LPCM_AUDIO_SUBSTREAM_ID_BEG > pucBuf[ucIndex]) ||
     (LPCM_AUDIO_SUBSTREAM_ID_END < pucBuf[ucIndex]) )
  {
    return false;
  }

  //skipping 1 byte of substreamID
  ucIndex++;

  pAudioInfo->NumberOfFrameHeaders = pucBuf[ucIndex];
  ucIndex++;

  //skipping one byte
  ucIndex++;

  (void)getByteFromBitStream(&ucVal,&pucBuf[ucIndex],2,3);

  if(ucVal < MAX_PCM_SAMPLING_FREQ_INDEX_VALUES)
  {
    pAudioInfo->SamplingFrequency = (uint32)PCM_SAMPLING_FREQUENCY_TABLE[ucVal];
  }
  else
  {
    bRet = false;
  }

  (void)getByteFromBitStream(&ucVal,&pucBuf[ucIndex],5,3);
  pAudioInfo->NumberOfChannels =(uint8)( ucVal+1);

  return bRet;
}

/* ==========================================================================
@brief      Function to update HDMV LPCM data in audio info structure.

@details    Parses LPCM meta data from currently parsed PES packet
            to retrieve sampling frequency, number of channels etc.

@param[in]  pAudioInfo    Audio info structure pointer.
@param[in]  pucBuf        Data Buf Ptr.
@param[in]  ulBufSize     Data Buf Size.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
bool parseHDMVLPCMHeader(audio_info* pAudioInfo,
                         uint8* pucBuf, uint32 /*ulBufSize*/)
{
  bool bRet     = true;
  uint8 ucIndex = 0;
  uint8 ucVal   = 0;

  pAudioInfo->Audio_Codec = AUDIO_CODEC_HDMV_LPCM;

  uint8 ucAudPresentationType = (uint8)(pucBuf[ucIndex] >> 4);
  uint8 ucSamplingFreq = (pucBuf[ucIndex++] & 0x0F);
  uint8 ucBitsPerSample = (uint8)(pucBuf[ucIndex] >> 6);

  pAudioInfo->SamplingFrequency = HD_LPCM_SAMPLING_FREQ[ucSamplingFreq];
  pAudioInfo->NumberOfChannels  = HD_LPCM_CHANNEL_INFO[ucAudPresentationType];
  pAudioInfo->ucBitsPerSample   = HD_LPCM_BIT_WIDTH[ucBitsPerSample];

  return bRet;
}

/* ==========================================================================
@brief      Function to update AC3 data in audio info structure.

@details    Parses AC3 meta data from currently parsed PES packet
            to retrieve sampling frequency, number of channels etc.

@param[in]  pAudioInfo    Audio info structure pointer.
@param[in]  pucBuf        Data Buf Ptr.
@param[in]  ulBufSize     Data Buf Size.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
bool parseAC3Header(audio_info* pAudioInfo, uint8* pucBuf, uint32 /*ulBufSize*/)
{
  uint32 ulIndex= 0;
  bool bRet = true;
  uint16 usAC3Sync  = (uint16)((pucBuf[ulIndex] << 8) | pucBuf[ulIndex + 1]);
  if( AC3_SYNC_WORD != usAC3Sync)
  {
    return false;
  }
  //Sync word is followed by 16 bits CRC, so increment the offset by 4.
  //Store parsed audio meta-data in stream info
  ulIndex += 4;
  uint32 ulSampFreqCode = (pucBuf[ulIndex] & 0xC0)>>6;
  uint32 ulFrmSizeCode = pucBuf[ulIndex] & 0x3F;
  ulIndex++;

  uint32 ulSamplingFreq = 48000;
  uint32 bit_rate     = 0;
  switch(ulSampFreqCode)
  {
    case SAMPLE_RATE_48_KHZ:
    {
      ulSamplingFreq = 48000;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"AC3 AUDIO 48 KHZ");
      break;
    }
    case SAMPLE_RATE_44_1_KHZ:
    {
      ulSamplingFreq = 44100;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"AC3 AUDIO 44.1 KHZ");
      break;
    }
    case SAMPLE_RATE_32_KHZ:
    {
      ulSamplingFreq = 32000;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"AC3 AUDIO 32 KHZ");
      break;
    }
    default:
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"AC3 USING DEFAULT SAMPLING FREQUENCY 48KHZ");
      break;
    }
  }//switch(fscod)
  pAudioInfo->SamplingFrequency = ulSamplingFreq;
  if(ulFrmSizeCode <= 0x25)
  {
    bit_rate = FRAME_SIZE_CODE_TABLE[ulFrmSizeCode].nominal_bit_rate * 1000;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"AC3 bit-rate %lu",bit_rate);
  }
  pAudioInfo->Bitrate = bit_rate;
  //Get the bit-stream identification, bit-stream Mode and number of channels
  uint8 bsid =  (uint8)((pucBuf[ulIndex] & 0xF1)>>3);
  uint8 bsmod = pucBuf[ulIndex] & 0x07;
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,"AC3 bsid %d bsmod %d",bsid,bsmod);
  ulIndex++;
  uint8 ucACmod = (pucBuf[ulIndex] & 0xE0)>>5;
  uint8 ucNumChannels = 2;
  if( ucACmod <= 0x07 )
  {
    ucNumChannels = CHANNELS_CONFIG[ucACmod].nfchans;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"AC3 #Channels %d",ucNumChannels);
  }
  pAudioInfo->NumberOfChannels = ucNumChannels;
  pAudioInfo->Audio_Codec      = AUDIO_CODEC_AC3;
  return bRet;
}
/* ==========================================================================
@brief      Function to update DVD-LPCM data in audio info structure.

@details    Parses DVD-LPCM meta data from currently parsed PES packet
            to retrieve sampling frequency, number of channels etc.

@param[in]  pAudioInfo    Audio info structure pointer.
@param[in]  pucBuf        Data Buf Ptr.
@param[in]  ulBufSize     Data Buf Size.

@return     True if successful, otherwise returns false.
@note       None.
 * LPCM DVD header :
   - number of frames in this packet (8 bits)
   - first access unit (16 bits) == 0x0003 ?
   - emphasis (1 bit)
   - mute (1 bit)
   - reserved (1 bit)
   - current frame (5 bits)
   - quantisation (2 bits) 0 == 16bps, 1 == 20bps, 2 == 24bps, 3 == illegal
   - frequency (2 bits) 0 == 48 kHz, 1 == 96 kHz, 2 == 44.1 kHz, 3 == 32 kHz
   - reserved (1 bit)
   - number of channels - 1 (3 bits) 1 == 2 channels
   - dynamic range (8 bits) 0x80 == neutral
========================================================================== */
bool parseDVDLPCMHeader(audio_info* pLPCMAudioInfo, uint8* pucBuf,
                        uint32 /*ulBufSize*/)
{
  bool bRet = true;
  uint8 ucReadIndex = 0;
  uint8 ucVal = 0;


  pLPCMAudioInfo->Audio_Codec = AUDIO_CODEC_LPCM;

  //skipping 1 byte of substreamID
  ucReadIndex++;

  pLPCMAudioInfo->NumberOfFrameHeaders =
    getBytesValue(1,&pucBuf[ucReadIndex]);
  ucReadIndex++;

  //skipping three bytes
  ucReadIndex = (uint8)(ucReadIndex + 3);

  (void)getByteFromBitStream(&ucVal,&pucBuf[ucReadIndex],2,2);

  if(ucVal < MAX_PCM_SAMPLING_FREQ_INDEX_VALUES)
  {
    pLPCMAudioInfo->SamplingFrequency = (uint32)
      DVD_PCM_SAMPLING_FREQUENCY_TABLE[ucVal];
  }
  else
  {
    bRet = false;
  }

  (void)getByteFromBitStream(&ucVal,&pucBuf[ucReadIndex],5,3);
  if(ucVal)
  {
    pLPCMAudioInfo->NumberOfChannels = 2;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
               "pLPCMAudioInfo->NumberOfChannels %lu",
               (uint32)pLPCMAudioInfo->NumberOfChannels);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
               "pLPCMAudioInfo->SamplingFrequency %lu",
               (uint32)pLPCMAudioInfo->SamplingFrequency);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
               "pLPCMAudioInfo->NumberOfFrameHeaders %lu",
               (uint32)pLPCMAudioInfo->NumberOfFrameHeaders);

  return bRet;
}
/* ==========================================================================
@brief      Function to update DTS data in audio info structure.

@details    Parses LPCM meta data from currently parsed PES packet
            to retrieve sampling frequency, number of channels etc.

@param[in]  pAudioInfo    Audio info structure pointer.
@param[in]  pucBuf        Data Buf Ptr.
@param[in]  ulBufSize     Data Buf Size.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
bool parseDTSHeader(audio_info* pDTSAudioInfo, uint8* pucBuf, uint32 /*ulBufSize*/)
{
  bool bRet = false;
  uint8 ucReadIndex = 0;
  uint8 ucVal = 0;

  if(pDTSAudioInfo)
  {
    if(!memcmp(pucBuf, (void*)DTS_SYNCWORD_CORE, FOURCC_SIGNATURE_BYTES))
    {
      pDTSAudioInfo->Audio_Codec = AUDIO_CODEC_DTS;
      ucReadIndex = (uint8)(ucReadIndex + 4); //skip 4 bytes of sync word

      ucReadIndex++;
      (void)getByteFromBitStream(&ucVal,&pucBuf[ucReadIndex],6,14);
      //frame size = 1+ val;
      ucReadIndex = (uint8)(ucReadIndex + 2);

      (void)getByteFromBitStream(&ucVal,&pucBuf[ucReadIndex],4,6);
      pDTSAudioInfo->NumberOfChannels = ucVal;
      ucReadIndex++;

      (void)getByteFromBitStream(&ucVal,&pucBuf[ucReadIndex],2,4);
      pDTSAudioInfo->SamplingFrequency = DTS_FSCODE_RATE[ucVal];
      //ucReadIndex++;

      (void)getByteFromBitStream(&ucVal,&pucBuf[ucReadIndex],6,5);
      pDTSAudioInfo->Bitrate = DTS_BIT_RATE[ucVal];
      bRet = true;
    }
  }
  return bRet;
}

/* ==========================================================================
@brief      Function to update AAC metadata in audio info structure.

@details    Parses AAC meta data from currently parsed PES packet
            to retrieve sampling frequency, number of channels etc.

@param[in]  pAudioInfo    Audio info structure pointer.
@param[in]  pucBuf        Data Buf Ptr.
@param[in]  ulBufSize     Data Buf Size.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
bool parseAACHeader(audio_info *pAACInfo, uint8* pucBuf, uint32 /*ulBufSize*/)
{
  bool bIsAAC = false;
  //! Check AAC Sync marker
  if( (0xFF != pucBuf[0] ) || (0xF0 != (pucBuf[1] & 0xF0)) )
  {
    return bIsAAC;
  }
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
              "parseAudioMetaData AAC_ADTS_SYNC_WORD");
  //ADTS Frame starts from here
  uint8 ucProtectionBit = (pucBuf[1] & 0x01);
  uint8 audioObjectType = (uint8)(((pucBuf[2] >> 6) & 0x03)+1);
  uint8 samplingFrequencyIndex = ((pucBuf[2] >> 2) & 0x0F);
  uint8 channelConfiguration   = (uint8)(((pucBuf[2] << 2) & 0x04) |
                                  ((pucBuf[3] >> 6) & 0x03));
  uint16 uData = (uint16)((pucBuf[1] << 8) + pucBuf[0]);

  // Verify sync word and layer field.
  if ((ADTS_HEADER_MASK_RESULT != (uData & ADTS_HEADER_MASK)) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "AAC sync word/layer verification failed...");
  }
  else
  {
    // Extract frame length from the frame header
    uint64 frameLength
      = (static_cast<uint64> (pucBuf [3] & 0x03) << 11)
        | (static_cast<uint64> (pucBuf [4]) << 3)
        | (static_cast<uint64> (pucBuf [5] & 0xE0) >> 5);

    uint32 ulSampleFreq = AAC_SAMPLING_FREQUENCY_TABLE[samplingFrequencyIndex];

    // Frame Length field should non zero
    // Sampling Frequency should always proper value
    if ((0 == frameLength) || (samplingFrequencyIndex >= 12) ||
        (ulSampleFreq == 0) )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "BitStream is not AAC complaint");
    }
    else
    {
      bIsAAC = true;
    }
    if(true == bIsAAC)
    {
      // if Protection Absent: Set to '1' if no CRC & '0' if CRC present.
      pAACInfo->ucProtection      = (ucProtectionBit == 0)? 1: 0;
      pAACInfo->AudioObjectType   = audioObjectType;
      pAACInfo->NumberOfChannels  = channelConfiguration;
      pAACInfo->SamplingFrequency = ulSampleFreq;
      pAACInfo->Audio_Codec       = AUDIO_CODEC_AAC;
    }
  }

  return bIsAAC;
}

/*! ======================================================================
@brief  Prepares Codec Config data and returns success

@detail    Checks whether SPS/PPS NAL types are available or not.
           If they are available, it prepares codec config data in Context.

@param[in] pulBufSize: Buffer Size
           pucDataBuf: buffer in which to look
           pContext  : Context pointer

@return    TRUE or FALSE
@note      None.
========================================================================== */
bool GetAVCCodecInfo(uint32* pulBufSize, uint8* pucDataBuf,
                     MP2ParserContext* pContext)
{
  avc_codec_info* pAVCCodecBuf = NULL;
  uint32 ulNALSize       = 0;
  uint32 ulBufSize       = 0;
  uint32 ulAVCIndex      = 0;
  uint32 ulBufIndex      = 0;
  uint32 ulSPSDataStart  = 0;
  uint32 ulNALStartIndex = 0;
  uint8  ucNALUType      = 0;
  bool bSPSFound = false;
  bool bPPSFound = false;
  bool bRet      = false;

  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"MP2StreamParser::GetAVCCodecInfo");

  //! Allocate memory if not allocated
  if(!pContext->pAVCCodecBuf)
  {
    pContext->pAVCCodecBuf = (avc_codec_info*)MM_Malloc(sizeof(avc_codec_info));
    if(pContext->pAVCCodecBuf)
    {
      memset(pContext->pAVCCodecBuf,0,sizeof(avc_codec_info));
    }
  }

  if(pContext->pAVCCodecBuf)
  {
    pAVCCodecBuf = pContext->pAVCCodecBuf;
    while((ulBufIndex < *pulBufSize))
    {
      //! Get SPS Length and start index
      bRet = GetNextH264NALUnit(ulBufIndex, pucDataBuf, &ucNALUType, &ulNALSize,
                                *pulBufSize, &ulNALStartIndex);
      if(bRet)
      {
        bRet = false;
        if((NAL_UNIT_TYPE_SPS == ucNALUType) ||
           (NAL_UNIT_TYPE_PPS == ucNALUType))
        {
          uint8* pCodecBuf = pAVCCodecBuf->codecInfoBuf;
          if(NAL_UNIT_TYPE_SPS == ucNALUType)
            bSPSFound = true;
          else
            bPPSFound = true;
          /* First time allocation, allocate more than required to avoid
             multiple reallocations. */
          if(NULL == pCodecBuf)
          {
            ulBufSize = ulNALSize * 4;
            pCodecBuf = (uint8*)MM_Malloc(ulBufSize);
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                         "GetAVCCodecInfo allocating %u", (ulBufSize));
          }
          //! If size allocated is less than required
          else if(pAVCCodecBuf->size < (ulAVCIndex + ulNALSize))
          {
            ulBufSize = (ulBufSize + ulNALSize) * 2;
            pCodecBuf = (uint8*)MM_Realloc(pCodecBuf, ulBufSize);
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                         "GetAVCCodecInfo realloc %u", ulBufSize);
          }
          if(pCodecBuf)
          {
            pAVCCodecBuf->codecInfoBuf = pCodecBuf;
            pAVCCodecBuf->size = ulBufSize;
            memcpy(pAVCCodecBuf->codecInfoBuf + ulAVCIndex,
                   pucDataBuf + ulBufIndex + ulNALStartIndex, ulNALSize);
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "GetAVCCodecInfo found AVC codec info, nal type %d",
                         ucNALUType);
            //! SPS is more important for Parser to get Height and Width info
            if (NAL_UNIT_TYPE_SPS == ucNALUType)
              pAVCCodecBuf->isValid = true;
            bRet = true;
          }
          else
          {
            //! Free the memory (if realloc failed)
            if (pAVCCodecBuf->codecInfoBuf)
            {
              MM_Free(pAVCCodecBuf->codecInfoBuf);
            }
            pAVCCodecBuf->codecInfoBuf = NULL;
            pAVCCodecBuf->size = ulAVCIndex = 0;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "GetAVCCodecInfo Memory alloc failed");
            break;
          }
          ulAVCIndex += ulNALSize;
        }
        else
        {
          //! If at least one SPS/PPS is found, break the loop
          if (bSPSFound && bPPSFound)
          {
            break;
          }
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                      "GetNextNALUnit not SPS, SKIP IT");
        }
        ulBufIndex += ulNALStartIndex;
        ulBufIndex += ulNALSize;
      } //! if(bRet)
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"GetNextNALUnit returned false");
        break;
      }
    } // while((ulIndex < *pulBufSize))
    //! If SPS/PPS is found, then return true
    bRet = false;
    if (pAVCCodecBuf->isValid)
    {
      bRet = true;
      //! update size to indicate amount of data filled in buffer
      pAVCCodecBuf->size = ulAVCIndex;
    }
  } // if(m_pAvcCodecInfo)
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"GetAVCCodecInfo returning %d", bRet);

  return bRet;
}

/*! ======================================================================
@brief      Function to get NAL Unit properties

@detail     Searches for two consecutive NAL unit start codes.
            Updates the NAL unit size.

@param[in]  ulBufOffset   Offset from where start code searches
            pucBuf        Buffer pointer
            pucNALUType   NALU type value
            pucNALULen    NALU Length
            nBytesRead    Data read in buffer
            pulIndex      Index to indicate start code pointer

@return     return true if NALU start code is found, else false.
@note       None.
========================================================================== */
bool GetNextH264NALUnit(uint32 ulBufOffset, uint8* pucBuf,
                        uint8* pucNALUType, uint32* pulNALULen,
                        int32  slBytesRead, uint32* pulIndex)
{
  bool bRet = false;
  uint32 ulTrackId = 0xFFFF;
  uint32 ulNALStartIndex = 0;
  start_code_type startCodeType = START_CODE_DEFAULT;

  //! Validate input params
  if ((!pucBuf) || (!pulIndex) || (0 == slBytesRead))
  {
    return false;
  }
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"MP2StreamParser::GetNextH264NalUnit");

  while((int32)ulBufOffset + 4 < slBytesRead)
  {
    //! Store first Offset from which NAL Unit is searched
    uint32 ulStartOffset = ulBufOffset;
    bRet = isFrameStartWithStartCode(&ulBufOffset, ulTrackId, pucBuf,
                                     slBytesRead, &startCodeType);
    if(bRet)
    {
      *pulIndex        = ulBufOffset - ulStartOffset;
      ulBufOffset     += startCodeType;
      *pucNALUType     = (pucBuf[ulBufOffset] & 0x1F);
      ulNALStartIndex  = ulBufOffset;
      bRet = isFrameStartWithStartCode(&ulBufOffset, ulTrackId, pucBuf,
                                       slBytesRead, &startCodeType);
      if(bRet)
      {
        ulBufOffset += startCodeType;
        *pulNALULen  = (ulBufOffset - ulNALStartIndex);
        break;
      }
    }
    else
    {
      break;
    }
  }
  return bRet;
}

/* ==========================================================================
@brief      Function to check whether frame is started or not.

@details    Function to check whether AVC 32bit or 24bit start codes are
            started in this frame or not.

@param[in]  pulBufIndex     I/p Buffer index.
@param[in]  ulTrackId       track Id.
@param[in]  pucBuf          Buffer pointer.
@param[in]  slBytesRead     Number of bytes read.
@param[in]  pStartCodeType  Start Code Enum Pointer.

@return     True if successful, otherwise returns false.
@note       None.
========================================================================== */
bool isFrameStartWithStartCode(uint32* pulBufOffset, uint32 /*ulTrackId*/,
                               uint8* pucBuf, int32 slBytesRead,
                               start_code_type* pStartCode)
{
  track_type       eTrackType = TRACK_TYPE_UNKNOWN;
  media_codec_type eCodecType = UNKNOWN_AUDIO_VIDEO_CODEC;
  uint32 startcodeval_24bit = 0;
  uint32 startcodeval_32bit = 0;
  bool   bRet = false;

  while((int32)*pulBufOffset < slBytesRead)
  {
    startcodeval_24bit = getBytesValue(3,&pucBuf[*pulBufOffset]);
    startcodeval_32bit = getBytesValue(4,&pucBuf[*pulBufOffset]);

    if(AVC_START_CODE_PREFIX_24BIT == startcodeval_24bit)
    {
      *pStartCode = AVC_START_CODE_24BIT;
      bRet = true;
      break;
    }
    else if(AVC_START_CODE_PREFIX_32BIT == startcodeval_32bit)
    {
      *pStartCode = AVC_START_CODE_32BIT;
      bRet = true;
      break;
    }
    else
    {
      (*pulBufOffset)++;
    }
  }//! while((int32)*pulBufOffset < nBytesRead)

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
              "MP2StreamParser::isFrameStartWithStartCode returning %d", bRet);

  return bRet;
}

 /*! ======================================================================
 @brief  Function to provide stream info struct ptr for given track

 @detail    Checks if given track id is stored in context or not and returns
            the stream_info structure pointer if available.

 @param[in]  ulTrackId   Track Id
 @param[in]  pContext    Context pointer

 @return    pointer if id exists in id store else returns NULL.
 @note      None.
 ========================================================================== */
stream_info* GetStreamInfoStructurePtr(uint32 ulTrackId,
                                       MP2ParserContext* pContext)
{
  stream_info* pTmpStreamInfo = NULL;
  stream_info* pStreamInfo    = NULL;
  uint8        ucIndex        = 0;
  if (!pContext)
  {
    return pStreamInfo;
  }
  pTmpStreamInfo = pContext->pStreamInfo;
  for(; ucIndex < pContext->usNumStreams && pTmpStreamInfo; ucIndex++)
  {
    if(pTmpStreamInfo[ucIndex].stream_id == ulTrackId)
    {
      pStreamInfo = pTmpStreamInfo + ucIndex;
      break;
    }
  }
  return pStreamInfo;
}

