
/* =======================================================================
                              id3.h

  Declarations for ID3 classes. This includes
  ID3v1 and ID3v2 and all constants and enums
  used by these classes and the callers of these classes

  Copyright (c) 2009-2015 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

======================================================================*/

/*======================================================================
                             Edit History
//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ID3Lib/main/latest/inc/id3.h#18 $
//$DateTime: 2013/10/21 02:25:48 $
//$Change: 4628966 $

======================================================================*/

//============================================================
#ifndef ID3_H
#define ID3_H
//============================================================
// INCLUDES
//============================================================
#include "parserdatadef.h"
#include "id3_metadata.h"
#include "oscl_file_io.h"
/* In HTTP Live streaming use case, Parser needs to read timestamp from ID3
   tags. This support is not required in the normal playback mode. */
#undef HLS_PRIVATE_TIMESTAMP

/* ============================================================================
  @brief  This function is used to parse given ID3 v2 Metadata.

  @details This function is used to check whether requested metadata type info
           is present in the given metadata list and updates the data buffer
           and other parameters as required.

  @param[in]     pID3v2Info             ID3 Metadata object.
  @param[in]     ienumData              Metadata enum type.
  @param[in/out] pucDataBuf             Buffer to store o/p data.
  @param[in/out] pulDatabufLen          Buffer size.
  @param[in/out] bMetaDatainUTF8        Flag to set if data is in UTF8 format.

  @return     None.
  @note       None.
============================================================================ */
PARSER_ERRORTYPE ParseID3V2MetaData(void *pID3v2Info,
                                    FileSourceMetaDataType ienumData,
                                    wchar_t*  pucDataBuf,
                                    uint32*   pulDatabufLen,
                                    FS_TEXT_ENCODING_TYPE&     bMetaDatainUTF8);

/* ============================================================================
  @brief  This function is used to parse given ID3 V1 Metadata.

  @details This function is used to check whether requested metadata type info
           is present in the given metadata list and updates the data buffer
           and other parameters as required.

  @param[in]     pID3v1Info             ID3 Metadata object.
  @param[in]     ienumData              Metadata enum type.
  @param[in/out] pucDataBuf             Buffer to store o/p data.
  @param[in/out] pulDatabufLen          Buffer size.

  @return     None.
  @note       None.
============================================================================ */
PARSER_ERRORTYPE ParseID3V1MetaData(void *pID3v1Info,
                                    FileSourceMetaDataType ienumData,
                                    wchar_t*  pucDataBuf,
                                    uint32*   pulDatabufLen);

/* ============================================================================
  @brief  This function is used to free the ID3 v2 Metadata structure.

  @details This function is used to free the memory allocated in ID3 v2
           metadata structure.

  @param[in]     pID3v2Info             ID3 Metadata object.

  @return     None.
  @note       None.
============================================================================ */
void FreeID3V2MetaDataMemory(metadata_id3v2_type* pID3v2);

/* ============================================================================
  @brief  This function is used to parse given ID3 V2 Metadata.

  @details This function is used to check whether requested metadata type info
           is present in the given metadata list and updates the data buffer
           and other parameters as required.

  @param[in]     pID3v2Info             ID3 Metadata object.
  @param[in/out] pucDataBuf             Buffer to store o/p data.
  @param[in/out] pulDatabufLen          Buffer size.

  @return     None.
  @note       None.
============================================================================ */
PARSER_ERRORTYPE ParseAlbumArtFromID3V2(void *pID3v2,
                                        wchar_t*  pucDataBuf,
                                        uint32*   pulDatabufLen);

//============================================================
// DATA TYPES
//============================================================
// list of known frame types which are handled in id3 library.

/*
*Caution: Do not change the following order.
*If you have to change following order, make the same change in
*static const byte frame_identifiers[][5].
*The order of these two must match exactly.
*/
typedef enum ID3V2_FOUR_CHARS_TAG_ENUMS
{
  ID3V2_FOUR_CHARS_TAG_UNKN,
  ID3V2_FOUR_CHARS_TAG_AENC,
  ID3V2_FOUR_CHARS_TAG_APIC,
  ID3V2_FOUR_CHARS_TAG_COMM,
  ID3V2_FOUR_CHARS_TAG_COMR,
  ID3V2_FOUR_CHARS_TAG_ENCR,
  ID3V2_FOUR_CHARS_TAG_EQUA,
  ID3V2_FOUR_CHARS_TAG_ETCO,
  ID3V2_FOUR_CHARS_TAG_GEOB,
  ID3V2_FOUR_CHARS_TAG_GRID,
  ID3V2_FOUR_CHARS_TAG_IPLS,
  ID3V2_FOUR_CHARS_TAG_LINK,
  ID3V2_FOUR_CHARS_TAG_MCDI,
  ID3V2_FOUR_CHARS_TAG_MLLT,
  ID3V2_FOUR_CHARS_TAG_OWNE,
  ID3V2_FOUR_CHARS_TAG_PRIV,
  ID3V2_FOUR_CHARS_TAG_PCNT,
  ID3V2_FOUR_CHARS_TAG_POPM,
  ID3V2_FOUR_CHARS_TAG_POSS,
  ID3V2_FOUR_CHARS_TAG_RBUF,
  ID3V2_FOUR_CHARS_TAG_RVAD,
  ID3V2_FOUR_CHARS_TAG_RVRB,
  ID3V2_FOUR_CHARS_TAG_SYLT,
  ID3V2_FOUR_CHARS_TAG_SYTC,
  ID3V2_FOUR_CHARS_TAG_TALB,
  ID3V2_FOUR_CHARS_TAG_TBPM,
  ID3V2_FOUR_CHARS_TAG_TCOM,
  ID3V2_FOUR_CHARS_TAG_TCON,
  ID3V2_FOUR_CHARS_TAG_TCOP,
  ID3V2_FOUR_CHARS_TAG_TDAT,
  ID3V2_FOUR_CHARS_TAG_TDLY,
  ID3V2_FOUR_CHARS_TAG_TENC,
  ID3V2_FOUR_CHARS_TAG_TEXT,
  ID3V2_FOUR_CHARS_TAG_TFLT,
  ID3V2_FOUR_CHARS_TAG_TIME,
  ID3V2_FOUR_CHARS_TAG_TIT1,
  ID3V2_FOUR_CHARS_TAG_TIT2,
  ID3V2_FOUR_CHARS_TAG_TIT3,
  ID3V2_FOUR_CHARS_TAG_TKEY,
  ID3V2_FOUR_CHARS_TAG_TLAN,
  ID3V2_FOUR_CHARS_TAG_TLEN,
  ID3V2_FOUR_CHARS_TAG_TMED,
  ID3V2_FOUR_CHARS_TAG_TOAL,
  ID3V2_FOUR_CHARS_TAG_TOFN,
  ID3V2_FOUR_CHARS_TAG_TOLY,
  ID3V2_FOUR_CHARS_TAG_TOPE,
  ID3V2_FOUR_CHARS_TAG_TORY,
  ID3V2_FOUR_CHARS_TAG_TOWN,
  ID3V2_FOUR_CHARS_TAG_TPE1,
  ID3V2_FOUR_CHARS_TAG_TPE2,
  ID3V2_FOUR_CHARS_TAG_TPE3,
  ID3V2_FOUR_CHARS_TAG_TPE4,
  ID3V2_FOUR_CHARS_TAG_TPOS,
  ID3V2_FOUR_CHARS_TAG_TPUB,
  ID3V2_FOUR_CHARS_TAG_TRCK,
  ID3V2_FOUR_CHARS_TAG_TRDA,
  ID3V2_FOUR_CHARS_TAG_TRSN,
  ID3V2_FOUR_CHARS_TAG_TRSO,
  ID3V2_FOUR_CHARS_TAG_TSIZ,
  ID3V2_FOUR_CHARS_TAG_TSRC,
  ID3V2_FOUR_CHARS_TAG_TSSE,
  ID3V2_FOUR_CHARS_TAG_TYER,
  ID3V2_FOUR_CHARS_TAG_TXXX,
  ID3V2_FOUR_CHARS_TAG_UFID,
  ID3V2_FOUR_CHARS_TAG_USER,
  ID3V2_FOUR_CHARS_TAG_USLT,
  ID3V2_FOUR_CHARS_TAG_WCOM,
  ID3V2_FOUR_CHARS_TAG_WCOP,
  ID3V2_FOUR_CHARS_TAG_WOAF,
  ID3V2_FOUR_CHARS_TAG_WOAR,
  ID3V2_FOUR_CHARS_TAG_WOAS,
  ID3V2_FOUR_CHARS_TAG_WORS,
  ID3V2_FOUR_CHARS_TAG_WPAY,
  ID3V2_FOUR_CHARS_TAG_WPUB,
  ID3V2_FOUR_CHARS_TAG_WXXX,
  ID3V2_FOUR_CHARS_TAG_MAX
}ID3V2_FOUR_CHARS_FRAME_TYPES;

/*
*Caution: Do not change the following order.
*If you have to change following order, make the same change in
*static const byte id3v2_2_below_frame_identifiers[][5].
*The order of these two must match exactly.
*/
typedef enum ID3V2_2_BELOW_TAG_ENUMS
{
  ID3V2_THREE_CHARS_TAG_UNK, /*unknown frame type*/
  ID3V2_THREE_CHARS_TAG_BUF, /*Recommended buffer size*/
  ID3V2_THREE_CHARS_TAG_CNT, /*Play counter*/
  ID3V2_THREE_CHARS_TAG_COM, /*Comments*/
  ID3V2_THREE_CHARS_TAG_CRA, /*Audio encryption*/
  ID3V2_THREE_CHARS_TAG_CRM, /*Encrypted meta frame*/
  ID3V2_THREE_CHARS_TAG_ETC, /*Event timing codes*/
  ID3V2_THREE_CHARS_TAG_EQU, /*Equalization*/
  ID3V2_THREE_CHARS_TAG_GEO, /*General encapsulated object*/
  ID3V2_THREE_CHARS_TAG_IPL, /*Involved people list*/
  ID3V2_THREE_CHARS_TAG_LNK, /*Linked information*/
  ID3V2_THREE_CHARS_TAG_MCI, /*Music CD Identifier*/
  ID3V2_THREE_CHARS_TAG_MLL, /*MPEG location lookup table*/
  ID3V2_THREE_CHARS_TAG_PIC, /*Attached picture*/
  ID3V2_THREE_CHARS_TAG_POP, /*Popularimeter*/
  ID3V2_THREE_CHARS_TAG_REV, /*Reverb*/
  ID3V2_THREE_CHARS_TAG_RVA, /*Relative volume adjustment*/
  ID3V2_THREE_CHARS_TAG_SLT, /*Synchronized lyric/text*/
  ID3V2_THREE_CHARS_TAG_STC, /*Synced tempo codes*/
  ID3V2_THREE_CHARS_TAG_TAL, /*Album/Movie/Show title*/
  ID3V2_THREE_CHARS_TAG_TBP, /*BPM (Beats Per Minute)*/
  ID3V2_THREE_CHARS_TAG_TCM, /*Composer*/
  ID3V2_THREE_CHARS_TAG_TCO, /*Content type*/
  ID3V2_THREE_CHARS_TAG_TCR, /*Copyright message*/
  ID3V2_THREE_CHARS_TAG_TDA, /*Date*/
  ID3V2_THREE_CHARS_TAG_TDY, /*Playlist delay*/
  ID3V2_THREE_CHARS_TAG_TEN, /*Encoded by*/
  ID3V2_THREE_CHARS_TAG_TFT, /*File type*/
  ID3V2_THREE_CHARS_TAG_TIM, /*Time*/
  ID3V2_THREE_CHARS_TAG_TKE, /*Initial key*/
  ID3V2_THREE_CHARS_TAG_TLA, /*Language(s)*/
  ID3V2_THREE_CHARS_TAG_TLE, /*Length*/
  ID3V2_THREE_CHARS_TAG_TMT, /*Media type*/
  ID3V2_THREE_CHARS_TAG_TOA, /*Original artist(s)/performer(s)*/
  ID3V2_THREE_CHARS_TAG_TOF, /*Original filename*/
  ID3V2_THREE_CHARS_TAG_TOL, /*Original Lyricist(s)/text writer(s)*/
  ID3V2_THREE_CHARS_TAG_TOR, /*Original release year*/
  ID3V2_THREE_CHARS_TAG_TOT, /*Original album/Movie/Show title*/
  ID3V2_THREE_CHARS_TAG_TP1, /*Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group*/
  ID3V2_THREE_CHARS_TAG_TP2, /*Band/Orchestra/Accompaniment*/
  ID3V2_THREE_CHARS_TAG_TP3, /*Conductor/Performer refinement*/
  ID3V2_THREE_CHARS_TAG_TP4, /*Interpreted, remixed, or otherwise modified by*/
  ID3V2_THREE_CHARS_TAG_TPA, /*Part of a set*/
  ID3V2_THREE_CHARS_TAG_TPB, /*Publisher*/
  ID3V2_THREE_CHARS_TAG_TRC, /*ISRC (International Standard Recording Code)*/
  ID3V2_THREE_CHARS_TAG_TRD, /*Recording dates*/
  ID3V2_THREE_CHARS_TAG_TRK, /*Track number/Position in set*/
  ID3V2_THREE_CHARS_TAG_TSI, /*Size*/
  ID3V2_THREE_CHARS_TAG_TSS, /*Software/hardware and settings used for encoding*/
  ID3V2_THREE_CHARS_TAG_TT1, /*Content group description*/
  ID3V2_THREE_CHARS_TAG_TT2, /*Title/Songname/Content description*/
  ID3V2_THREE_CHARS_TAG_TT3, /*Subtitle/Description refinement*/
  ID3V2_THREE_CHARS_TAG_TXT, /*Lyricist/text writer*/
  ID3V2_THREE_CHARS_TAG_TXX, /*User defined text information frame*/
  ID3V2_THREE_CHARS_TAG_TYE, /*Year*/
  ID3V2_THREE_CHARS_TAG_UFI, /*Unique file identifier*/
  ID3V2_THREE_CHARS_TAG_ULT, /*Unsychronized lyric/text transcription*/
  ID3V2_THREE_CHARS_TAG_WAF, /*Official audio file webpage*/
  ID3V2_THREE_CHARS_TAG_WAR, /*Official artist/performer webpage*/
  ID3V2_THREE_CHARS_TAG_WAS, /*Official audio source webpage*/
  ID3V2_THREE_CHARS_TAG_WCM, /*Commercial information*/
  ID3V2_THREE_CHARS_TAG_WCP, /*Copyright/Legal information*/
  ID3V2_THREE_CHARS_TAG_WPB, /*Publishers official webpage*/
  ID3V2_THREE_CHARS_TAG_WXX, /*User defined URL link frame*/
  ID3V2_THREE_CHARS_TAG_MAX
}ID3V2_THREE_CHARS_FRAME_TYPES;

//============================================================
// CONSTANTS
//============================================================
// Size of an ID3v1 tag
#define ID3v1_SIZE 128
#define ID3V1_TITLE_LENGTH   30
#define ID3V1_ARTIST_LENGTH  30
#define ID3V1_ALBUM_LENGTH   30
#define ID3V1_YEAR_LENGTH    4
#define ID3V1_COMMENT_LENGTH 29

// Size of an ID3v2 tag header
//This size is identical to all majors of id3v2
#define ID3v2_HEADER_SIZE 10

// Size of tag frame header for ID3v2.3 and above
#define ID3V2_3_4_FRAME_HEADER_SIZE 10
// Size of frame id for ID3v2.3 and above
#define ID3V2_3_4_FRAME_ID_SIZE 4
// Size of frame length ID3v2.3 and above
#define ID3V2_3_4_FRAME_LENGTH_SIZE 4

// Size of tag frame header for ID3v2.2 and below
#define ID3V2_2_BELOW_FRAME_HEADER_SIZE 6
// Size of frame id for ID3v2.2 and below
#define ID3V2_2_BELOW_FRAME_ID_SIZE 3
// Size of frame length for ID3v2.2 and below
#define ID3V2_2_BELOW_FRAME_LENGTH_SIZE 3


//============================================================
// CLASS: ID3v1
//
// DESCRIPTION:
//  Interface that extracts the ID3v1 tag
//
//  This interface defines methods for extracting the ID3v1 tag that is found
//  in the audio data.
//
//============================================================
class ID3v1
{
public:
   // Constructor
   ID3v1 (PARSER_ERRORTYPE &rnResult);
   // Destructor
   ~ID3v1 ();

   // static function that can be used to check the file to see if an ID3v1 tag is present.
   // Once the caller knows that and ID3v1 tag is present an object of this class
   // can be created to extract the tag information.
  static bool check_ID3v1_present(OSCL_FILE* fp_ID3FilePtr, uint64 nLength);
   // called to parse and extract all the info present in the ID3v1 tag
  PARSER_ERRORTYPE parse_ID3v1_tag(OSCL_FILE* fp_ID3FilePtr,
                                   metadata_id3v1_type *pstid3v1,
                                   uint64 nLength);
   private:
   // File read offset while parsing the file to extract ID3v1 info
   uint64 m_fileoffset;
};
//============================================================
// CLASS: ID3v2
//
// DESCRIPTION:
//  Interface that extracts the ID3v2 tag
//
//  This interface defines methods for extracting the ID3v2 tag that is found
//  in the audio data. The methods include seraching the file for an ID3v2 tag,
//  parsing an ID3v2 tag and getting the size of an ID3v2 tag present in the file.
//
//============================================================
class ID3v2
{
public:
   // Constructor
   ID3v2 (PARSER_ERRORTYPE &rnResult);

   // Destructor
   ~ID3v2 ();
   /* static function that can be used to check the file to see if an ID3v2 tag
      is present. Also sets a flag to inform the caller as to whether the tag
      is appended at the end of the file after the audio data or before the
      start of audio data in the beginning of the file. Once the caller knows
      that and ID3v2 tag is present an object of this class can be created to
      extract the tag information. */
  static bool check_ID3v2_present(OSCL_FILE* fp_ID3FilePtr,
                                  uint64 nLength,
                                  uint64 nOffset,
                                  bool *pbID3v2postend);

   // called to parse and extract all the info present in the ID3v2 tag
  PARSER_ERRORTYPE parse_ID3v2_tag(OSCL_FILE* fp_ID3FilePtr, uint64 nOffset,
                                   metadata_id3v2_type *pstid3v2,
                                   bool bID3v2postend);

   // called to get the complete size of the ID3v2 tag, the caller can call
   // this API even before calling "parse_ID3v2_tag". But calling this API
   // after calling "parse_ID3v2_tag" is more efficient though.
  PARSER_ERRORTYPE get_ID3v2_size(OSCL_FILE* fp_ID3FilePtr, uint64 nLength,
                                  uint64 nOffset, bool bID3v2postend,
                                  uint64 *size);

private:


   // Total size of the ID3v2 tag including the header
   uint64 m_size;

   // File read offset while parsing the file to extract ID3v1 info
   uint64 m_filereadoffset;

   // starting position of the ID3 tag in the file
   uint64 m_ID3v2startpos;

   // ID3v2 tag major version
   uint8 m_umajorversion;

   // ID3v2 tag minor version
   uint8 m_uminorversion;

   // flag to indicate a footer is present at the end of the tag
   bool m_bfooterpresent;

   //flag to indicate whether or not unsynchronisation is applied on all frames
   bool m_bunsncyhronisation;

   // flag to indicate whether the header is followed by an extended header
   bool m_bextendedheader;

   // flag to indicate whether the tag is in experimental stage
   bool m_bexperimentalheader;

   // structure pointer to proidve the parsed ID3v2 info to the caller
   metadata_id3v2_type *m_pstid3v2;

   // handles parsing the ID3v2 tag when present after audio data at
   // the end of the file
  PARSER_ERRORTYPE parse_ID3v2_tag_postend(OSCL_FILE* fp_ID3FilePtr,
                                           uint64 nOffset,
                                           metadata_id3v2_type *pstid3v2,
                                           uint64 nLength);

   // handles parsing the ID3v2 tag when present before audio data at
   // the beginning of the file
  PARSER_ERRORTYPE parse_ID3v2_tag_prepend(OSCL_FILE* fp_ID3FilePtr,
                                           uint64 nOffset,
                                           metadata_id3v2_type *pstid3v2,
                                           uint64 nLength);

   // handles parsing the ID3v2 tag header
  PARSER_ERRORTYPE parse_ID3v2_tag_header(OSCL_FILE* fp_ID3FilePtr,
                                          uint64 nOffset,
                                          metadata_id3v2_type *pstid3v2,
                                          uint64 nLength);

   // handles idenitfying and parsing the ID3v2 tag frames
  PARSER_ERRORTYPE parse_ID3v2_frames(OSCL_FILE* fp_ID3FilePtr,
                                      metadata_id3v2_type *pstid3v2,
                                      uint64 nLength);

   // handles the parsing of an individual ID3v2 frame
   PARSER_ERRORTYPE parse_ID3v2_frame(OSCL_FILE* fp_ID3FilePtr,
                                      ID3V2_FOUR_CHARS_FRAME_TYPES frame_type,
                                      uint64 frame_length);

   PARSER_ERRORTYPE parse_ID3v2_frame(OSCL_FILE* fp_ID3FilePtr,
                                      ID3V2_THREE_CHARS_FRAME_TYPES frame_type,
                                      uint64 frame_length);

   // handles the parsing of an generic ID3v2 frames
   // this includes frames that are not included under ID3v2_frame_type enum
  // but are supported as defined by ID3V2_IS_FRAME_SUPPORTED macro
  PARSER_ERRORTYPE parse_ID3v2_frame_generic(OSCL_FILE* fp_ID3FilePtr,
                                             const uint8 *pframe_id,
                                             uint64 frame_length);

   // handles the parsing of an individual ID3v2 text frame
  PARSER_ERRORTYPE parse_ID3v2_text_frame(OSCL_FILE* fp_ID3FilePtr,
                                          text_frame *ptextframe,
                                          uint64 frame_length);
  // handles the parsing of an individual ID3v2 private frame
  PARSER_ERRORTYPE parse_ID3v2_priv_frame(OSCL_FILE* fp_ID3FilePtr,
                                          private_tag_info *priv_tag,
                                          uint64 frame_length);

   // handles the parsing of an ID3v2 unit as defined by the spec
   // for the version that the tag complies to
   uint32 parse_ID3v2_uint(const uint8 *pInput, unsigned int nbytes) const;

   // initializes the member variables of this class used for parsing the tag
   void init_ID3v2_params();

   // returns an integer for a sync safe integer used in ID3v2 tags.
   uint32 getsyncsafeinteger(const byte *pInput, int nbytes) const;

   // identifies the frame type from the list of known frame types
   // under ID3v2_frame_type enum given a frame ID
   ID3V2_FOUR_CHARS_FRAME_TYPES get_frame_id_type(const byte *frame_id) const;

   ID3V2_THREE_CHARS_FRAME_TYPES get_id3v2_2_below_frame_id_type
                                            (const byte *frame_id) const;

   // reads the next frame and provides its frame type and length
  PARSER_ERRORTYPE get_next_frame(OSCL_FILE* fp_ID3FilePtr,
                                  ID3V2_FOUR_CHARS_FRAME_TYPES *pframe_type,
                                  uint64 *pframe_length) const;

  PARSER_ERRORTYPE get_next_frame(OSCL_FILE* fp_ID3FilePtr,
                                  ID3V2_THREE_CHARS_FRAME_TYPES *pframe_type,
                                  uint64 *pframe_length) const;

  // checks if we have reached the padding which may be present
  // at the end of an ID3v2 tag
  bool check_padding(OSCL_FILE* fp_ID3FilePtr);

  // method to parse and APIC frame type
  PARSER_ERRORTYPE parse_ID3v2_frame_APIC(OSCL_FILE* fp_ID3FilePtr,
                                          id3v2_pic_info* picTag,
                                          uint64 frame_length);

   // handles the parsing of an individual ID3v2 text frame
  PARSER_ERRORTYPE parse_ID3v2_comment_frame(OSCL_FILE* fp_ID3FilePtr,
                                            encoder_delay_tag_info *ptextframe,
                                            uint64 ullFrameLen);

  /*! =======================================================================
  @brief  Function to reverse byte for UTF16 string

  @detail This is used to provide data in system expected Endian format

  @param[in]
    pStr      : Input string which needs to be swapped
    ulStrLen  : String length

  @return    None
  @note      None.
  ===========================================================================*/
  void             ByteSwapString(char* pStr, uint32 ulStrLen);
};

#endif

