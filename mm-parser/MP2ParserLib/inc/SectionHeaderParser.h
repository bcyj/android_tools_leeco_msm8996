/* =======================================================================
                              SectionHeaderParser.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/inc/SectionHeaderParser.h#1 $
========================================================================== */
#include "MP2StreamParserDataDefn.h"
#include "MP2StreamParserStatus.h"

  /* ==========================================================================
    @brief      Create MP2 TS Header parsing context.

    @details    This function is used to initialize the context.

    @param[in/out]    pContext        Context pointer.

    @return   true if memory allocated successfully, else false.

    @note       None.
  ========================================================================== */
  bool              CreateMP2ParserContext(MP2ParserContext* pContext);
  /* ==========================================================================
    @brief      Free MP2 TS Header parsing context.

    @details    This function is used to de-initialize the context.

    @param[in/out]    pContext        Context pointer.

    @return   true if memory allocated successfully, else false.

    @note       None.
  ========================================================================== */
  void              FreeMP2ParserContext(MP2ParserContext* pContext);

  //! Functions to parse TS Packet data (Payload or Tables or Descriptors)
  /* ==========================================================================
  @brief      Function to parse a TS Packet.

  @details    This function is used to read TS Packet Size worth of data and
              parse it.

  @param[in/out]    pContext        Context pointer.

  @return   MP2STREAM_SUCCESS if parsed successfully, else corresponding error.

  @note       None.
  ========================================================================== */
  MP2StreamStatus   parseTransportStreamPacket(MP2ParserContext* pContext);
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
  uint16            CalcSectionLength(uint8* pucBuf, uint32& rulIndex,
                                      bool bPayloadStart);
  /* ==========================================================================
    @brief      Function to parse various Descriptor tables.

    @details    This function is used to parse various metadata tables such as
                PAT, PMT and CAT. It first checks whether sufficient data

    @param[in]      pContext      Context pointer.

    @return     MP2STREAM_SUCCESS indicating metadata read successfully.
                Else, it will report corresponding error.
    @note       BufferSize should be less than metadata size value (188).
  ========================================================================== */
  MP2StreamStatus   parseTablesData(MP2ParserContext* pContext);
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
  MP2StreamStatus   parsePayloadData(MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to parse Program Association Table.

  @details    This function is used to parse PAT read into PAT Section buffer.

  @param[in]      pContext        Context pointer.

  @return     MP2STREAM_SUCCESS indicating metadata read successfully.
              Else, it will report corresponding error.
  @note       None.
========================================================================== */
  MP2StreamStatus   parseProgAssociationTable(MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to parse Program Map Table.

  @details    This function is used to parse PAT read into PMT Section buffer.

  @param[in]      pContext        Context pointer.

  @return     MP2STREAM_SUCCESS indicating metadata read successfully.
              Else, it will report corresponding error.
  @note       None.
========================================================================== */
  MP2StreamStatus   parseProgMapTable(MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to parse Adaptation Field in TS Packet Header.

  @details    This function is used to parse adaptation field in TS Pkt.

  @param[in]      pContext        Context pointer.

  @return     MP2STREAM_SUCCESS indicating metadata read successfully.
              Else, it will report corresponding error.
  @note       None.
========================================================================== */
  MP2StreamStatus   parseAdaptationField(MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to parse Conditional Access Table.

  @details    This function is used to parse PAT read into CAT Section buffer.

  @param[in]      pContext        Context pointer.

  @return     MP2STREAM_SUCCESS indicating metadata read successfully.
              Else, it will report corresponding error.
  @note       None.
  ========================================================================== */
  MP2StreamStatus   parseCondAccessTable(MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to check whether TS Packet has payload data or metadata.

  @details    This function will return true, if TS Pkt contains PAT/PMT/CAT.

  @param[in]      pContext        Context pointer.

  @return     MP2STREAM_SUCCESS indicating metadata read successfully.
              Else, it will report corresponding error.
  @note       None.
  ========================================================================== */
  bool              isPSI(uint32 ulPID, MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to prepare Descriptors list.

  @details    This function will prepare descriptors list from the given buf.

  @param[in]      pucDataBuf      Data Buf pointer.
  @param[in]      ulBytes         Data Buffer Size.

  @return     Pointer of first entry in list will be returned in success case,
              Else, NULL pointer.
  @note       None.
  ========================================================================== */
  Desciptor*        prepareDescriptorList(uint8* pucDataBuf,
                                          const uint32 ulBytes);
  /* ==========================================================================
  @brief      Function to update track info using descriptors.

  @details    This function will update track info with the given descriptors.

  @param[in]      pContext        Context pointer.

  @return     Pointer of first entry in list will be returned in success case,
              Else, NULL pointer.
  @note       None.
  ========================================================================== */
  MP2StreamStatus   UpdateTrackInfoUsingDescriptors(MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to check whether required registration descriptor .

  @details    This function will update track info with the given descriptors.

  @param[in]      pList             Descriptor list.
  @param[in]      ulRegDesctiptor   Registration Desc value.

  @return     true if found, else false.
  @note       None.
  ========================================================================== */
  bool              SearchRegDescriptor(Desciptor* pList,
                                        uint32 ulRegDesctiptor);
  /* ==========================================================================
  @brief    Function to check for whether required descriptor is present or not.

  @details    This function is used to check whether requested descriptor
              is available in given list or not.

  @param[in]      pList              Descriptor list.
  @param[in]      ulRegDesctiptor    Descriptor tag.

  @return     True if found, else false.
  @note       None.
  ========================================================================== */
  bool              FindDescriptor(Desciptor* pList,
                                   uint8 ucDesctiptorTag);
  /* ==========================================================================
    @brief      Function to parse required descriptors.

    @details    This function is used to check whether requested descriptor
                is available in given list or not.

    @param[in]      pList              Descriptor list.
    @param[in]      ucDescrTag         Registration descriptor.
    @param[in/out]  pVoid              Structure to keep descriptor data.

    @return     True if found and parsed successfully, else false.

    @note       None.
  ========================================================================== */
  bool              ParseDescriptor(Desciptor* pDescList,
                                    uint8 ucDescrTag,
                                    void* pVoid);
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
  ESDescriptor*     parseProgESDescriptors(uint8* pucBuf,
                                           uint16 usBytes,
                                           MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to free PAT structure.

  @details    This function will free the memory allocated to PAT Structure.

  @param[in]  pPATSection        PAT pointer.

  @return     Pointer of first entry in list will be returned in success case,
              Else, NULL pointer.
  @note       None.
  ========================================================================== */
  void              freePAT(ProgramAssociationSection* pPATSection);
  /* ==========================================================================
  @brief      Function to free PMT list.

  @details    This function will free the memory allocated to PMT Structure.

  @param[in]  pProgMapSection        PMT pointer.

  @return     None.
  @note       None.
  ========================================================================== */
  void              freePMT(ProgramMapSection* pProgMapSection);
  /* ==========================================================================
  @brief      Function to free Descriptor list.

  @details    This function will free the memory allocated to Descriptors.

  @param[in]  pDescList       Descriptor Ptr list.

  @return     None.
  @note       None.
  ========================================================================== */
  void              FreeDescriptors(Desciptor* pDescList);
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
  MP2StreamStatus   parseCADescriptor(Desciptor* pDescList,
                                      CADescriptor* pCADesc);
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
  MP2StreamStatus   parseDTSAudioDescriptor(Desciptor* pDescList,
                                            audio_info* pAudioInfo);
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
  MP2StreamStatus   parseDTSHDAudioDescriptor(Desciptor* pDesc,
                                              audio_info* pAudioInfo);
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
  MP2StreamStatus   parseDTSHDSubstreamInfo(uint8* pucBuf,
                                       DTSHDSubstreamStruct* pSubstreamStruct,
                                       uint8 ucLen);
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
  MP2StreamStatus   parseDVDLPCMAudioDescriptor(Desciptor* pDesc,
                                                audio_info* pAudioInfo);

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
                                               audio_info* pAudioInfo);
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
  MP2StreamStatus   parseAC3AudioDescriptor(Desciptor* pDescList,
                                            audio_info* pAudioInfo);
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
  MP2StreamStatus   parseLanguageDescriptor(Desciptor* pDesc,
                                            LanguageDescriptor *pLang);
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
  MP2StreamStatus   UpdateStreamInfo(uint8  ucStreamType,
                                     uint16 usPID,
                                     MP2ParserContext* pContext);
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
  MP2StreamStatus   UpdateCodecInfo(uint16 usPID,
                                    media_codec_type eStreamType,
                                    MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to check whether given PID is PMT or not.

  @details    This function will check whether PID is PMT or not.

  @param[in]  usPID             PID value.
  @param[in]  pusProgIndex      Program Index number.
  @param[in]  pContext          Context ptr.

  @return     True if PID belongs to PMT otherwise returns false.
  @note       None.
  ========================================================================== */
  bool              isProgramMapPacket(uint16 usPID,
                                       uint16* pusProgIndex,
                                       MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to check whether given PID is PMT or not.

  @details    Inspects Data content to determine
              if it's a PES packet start code.

  @param[in]  pucDataBuf      Data Buf Ptr.
  @param[in]  pulValue        PES Packet type.

  @return     True if Data represents PES packet otherwise returns false.
  @note       None.
  ========================================================================== */
  bool              isPESPacket(uint8* pucDataBuf, uint32* pulValue);

  //! Functions to parse Audio Metadata
  /* ==========================================================================
  @brief      Function to update audio metadata if required.

  @details    Parses audio meta data from currently parsed PES packet
              to retrieve sampling frequency, number of channels etc.

  @param[in]  pContext      Context Ptr.

  @return     True if successful, otherwise returns false.
  @note       None.
  ========================================================================== */
  bool              parseAudioMetaData(MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to update audio metadata if required.

  @details    Checks if streamType passed is one of the supported ones.

  @param[in]  ucStreamType  Stream type value.
  @param[in]  pContext      Context Ptr.

  @return     True if successful, otherwise returns false.
  @note       None.
  ========================================================================== */
  media_codec_type  isAudioStreamType(uint8 ucStreamType,
                                      MP2ParserContext* pContext);
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
  bool              parseLPCMHeader(audio_info* pAudioInfo,
                                    uint8* pucBuf, uint32 ulBufSize);
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
  bool              parseHDMVLPCMHeader(audio_info* pAudioInfo,
                                        uint8* pucBuf, uint32 ulBufSize);
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
  bool              parseAC3Header(audio_info* pAudioInfo,
                                   uint8* pucBuf, uint32 ulBufSize);
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
  bool              parseAACHeader(audio_info* pAACInfo,
                                   uint8* pucBuf, uint32 ulBufSize);
  /* ==========================================================================
  @brief      Function to update DTS metadata in audio info structure.

  @details    Parses DTS meta data from currently parsed PES packet
              to retrieve sampling frequency, number of channels etc.

  @param[in]  pAudioInfo    Audio info structure pointer.
  @param[in]  pucBuf        Data Buf Ptr.
  @param[in]  ulBufSize     Data Buf Size.

  @return     True if successful, otherwise returns false.
  @note       None.
  ========================================================================== */
  bool              parseDTSHeader(audio_info* pDTSInfo, uint8* pucBuf,
                                   uint32 ulBufSize);
  /* ==========================================================================
  @brief      Function to update DVDLPCM metadata in audio info structure.

  @details    Parses DVDLPCM meta data from currently parsed PES packet
              to retrieve sampling frequency, number of channels etc.

  @param[in]  pAudioInfo    Audio info structure pointer.
  @param[in]  pucBuf        Data Buf Ptr.
  @param[in]  ulBufSize     Data Buf Size.

  @return     True if successful, otherwise returns false.
  @note       None.
  ========================================================================== */
  bool              parseDVDLPCMHeader(audio_info* pDTSInfo, uint8* pucBuf,
                                   uint32 ulBufSize);
  /* ==========================================================================
  @brief      Function to update MPG header in audio info structure.

  @details    Parses MPG meta data from currently parsed PES packet
              to retrieve sampling frequency, number of channels etc.

  @param[in]  pAudioInfo    Audio info structure pointer.
  @param[in]  pucBuf        Data Buf Ptr.
  @param[in]  ulBufSize     Data Buf Size.

  @return     True if successful, otherwise returns false.
  @note       None.
  ========================================================================== */
  bool              ParseMPGAudioHeader(audio_info* pAudioInfo,
                                        uint8* pucBuf, uint32 ulBufSize);
  /* ==========================================================================
  @brief      Function to check whether bit-stream is AC3 format or not.

  @details    This function checks whether sub-stream id is AC3 id or not.

  @param[in]  pucBuf        Data Buf Ptr.
  @param[in]  ulBufSize     Data Buf Size.

  @return     Index from where AC3 bit-stream will start.
              ZERO indicates bit-stream is not AC3 complaint.
  @note       None.
  ========================================================================== */
  uint16            IsBitStreamAC3Complaint(uint8* pucBuf, uint32 ulBufSize);
  /* ==========================================================================
  @brief      Function to update audio stream id info in Program Stream.

  @details    This function updates total number of audio streams.

  @param[in]  pContext        Context Ptr.

  @return     None.
  @note       None.
  ========================================================================== */
  void              UpdateAudioInfoInPS(MP2ParserContext* pContext);

  //! Functions to parse Video Metadata
  /*! ====================================================================
  @brief      Parses video meta data.

  @detail     Parses video meta data from currently parsed PES packet
              retrieve resolution, aspect ratio etc.

  @param[in]  pContext      Context Pointer

  @return     true if meta data is parsed successfully else returns false
  @note       None.
  ======================================================================== */
  bool              parseVideoMetaData(MP2ParserContext* pContext);
  /* ==========================================================================
  @brief      Function to check whether stream is video or not.

  @details    Checks if streamType passed is one of the supported ones.

  @param[in]  ucStreamType  Stream type value.

  @return     True if successful, otherwise returns false.
  @note       None.
  ========================================================================== */
  media_codec_type  isVideoStreamType(uint8 ucStreamType);
  /* ==========================================================================
  @brief      Function to copy AVC codec config data into o/p buffer.

  @details    Function to copy AVC Codec config data from Context.

  @param[in]  pucBuf    I/p Buffer pointer.
  @param[in]  pucSize   I/p Buffer size.
  @param[in]  pContext  Context pointer.

  @return     True if successful, otherwise returns false.
  @note       None.
  ========================================================================== */
  bool              makeAVCVideoConfig(uint8* pucBuf,uint32* pucSize,
                                       MP2ParserContext* pContext);
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
  bool              isFrameStartWithStartCode(uint32* pulBufIndex,
                                              uint32  ulTrackId,
                                              uint8*  pucBuf,
                                              int32   slBytesRead,
                                              start_code_type* pStartCodeType);
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
  bool             GetAVCCodecInfo(uint32* pulBytesRead, uint8* pucBuf,
                                    MP2ParserContext* pContext);
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
   bool             GetNextH264NALUnit(uint32 ulOffset, uint8* pucBuf,
                                      uint8* nalUType, uint32* nalLen,
                                      int32 slBytesRead, uint32* pulOffset);
   /*! ======================================================================
   @brief  Checks id store to see if given id exists in the store

   @detail    Checks id store to see if given id exists in the store

   @param[in]  ulTrackId   Track Id
   @param[in]  pContext    Context pointer

   @return    true if id exists in id store else returns false
   @note      None.
   ========================================================================== */
  bool              isTrackIdInIdStore(uint32 ulTrackId,
                                       MP2ParserContext* pContext);

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
                                         MP2ParserContext* pContext);
  uint32            getBytesValue(int nBytes,uint8* pucData);
  uint16            make15BitValue(uint8 ucTopBits, uint16 usLowerBits);
  uint32            make22BitValue(uint16 part1,uint8 part2);
  uint16            make9BitValue(uint8 ucTopBits, uint8 ucLowerBits);
  uint64            make42BitValue(uint64 ullPart1, uint16 usPart2);
  uint64            make33BitValue(uint8 ms3bits, uint16 middle15bits,
                                   uint16 ls15bits);
  void              getByteFromBitStream(uint8 *pByte, uint8 *pSrc,
                                         int nFromBit, int nBits);
