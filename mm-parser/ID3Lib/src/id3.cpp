/* =======================================================================
                              ID3.cpp

  DESCRIPTION:

  Definitions for ID3 classes. This includes
  ID3v1 and ID3v2 and all constants and enums
  used by these classes and the callers of these classes

  Copyright (c) 2009-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ID3Lib/main/latest/src/id3.cpp#32 $
$DateTime: 2013/11/06 08:04:15 $
$Change: 4726108 $

========================================================================== */

//============================================================
// INCLUDES
//============================================================
#include "id3.h"
#include "oscl_file_io.h"
//=============================================================
// GLOBALS
//=============================================================

//=============================================================
// MACROS
//=============================================================

#define ID3V2_IS_WFRAME(x) (((byte *)(x))[0] == 'W' )

#define ID3V2_IS_TFRAME(x) (((byte *)(x))[0] == 'T' )

// Condition to check for frame ID strings that should be handled as
// generic frames and are NOT parsed individually
// list more macros under this condition when required
#define ID3V2_IS_FRAME_SUPPORTED(x) ((ID3V2_IS_WFRAME(x))\
                                      ||(ID3V2_IS_TFRAME(x)))

int GET_ID3V2_FRAME_ID_SIZE(int major)
{
  if(major >=3 )
  {
    return ID3V2_3_4_FRAME_ID_SIZE;
  }
  return ID3V2_2_BELOW_FRAME_ID_SIZE;
}
int GET_ID3V2_FRAME_LENGTH_SIZE(int major)
{
  if(major >=3 )
  {
    return ID3V2_3_4_FRAME_LENGTH_SIZE;
  }
  return ID3V2_2_BELOW_FRAME_LENGTH_SIZE;
}
int GET_ID3V2_FRAME_HDR_SIZE(int major)
{
  if(major >=3 )
  {
    return ID3V2_3_4_FRAME_HEADER_SIZE;
  }
  return ID3V2_2_BELOW_FRAME_HEADER_SIZE;
}

/* ============================================================================
  @brief  CalcStringLength

  @details    Calculates string length.
              It varies based on coding type

  @param[in]  pBuf                Input buffer (string pointer)
  @param[in]  bEncodingType       Encoding type
  @param[in]  ulBufSize           Input Buffer Size

  @return  String size.
  @note    None.
=============================================================================*/
uint32 CalcStringLength(const uint8 *pBuf, uint8 bEncodingType,
                        uint32 ulBufSize)
{
  //! Validate input parameters
  if ((NULL == pBuf) || (0 == ulBufSize))
  {
    return 0;
  }
  /* ID3 tags has support for 4 types of Encodings.
     $00: ISO-8859-1 (LATIN-1, Identical to ASCII for values smaller than 0x80)
     $01: UCS-2 (UTF-16 encoded Unicode with BOM), in ID3v2.2 and ID3v2.3.
     $02: UTF-16BE encoded Unicode without BOM, in ID3v2.4.
     $03: UTF-8 encoded Unicode, in ID3v2.4.
  */

  // ISO 8859-1 or UTF-8, calculate string size using normal strlen API
  if ((0x00 == bEncodingType) || (0x03 == bEncodingType))
  {
    return (uint32)(strlen((const char *)pBuf) + 1);
  }

  // UTF-16 with and without BOM
  uint32 ulStrSize = 0;
  while (pBuf[ulStrSize] != '\0' || pBuf[ulStrSize + 1] != '\0')
  {
    //! Check if String length calculated is crossing the buffer Size or not
    if ((ulStrSize + (int)sizeof(uint16)) > ulBufSize)
    {
      ulStrSize -= (int)sizeof(uint16);
      break;
    }
    ulStrSize += (int)sizeof(uint16);
  }

  // Add size of null termination.
  return ulStrSize + (int)sizeof(uint16);
}

//=============================================================
// CONSTANTS
//=============================================================
// Frame ID strings for known list of frame types which are handled
/*
*Caution: Do not change the following order.
*If you have to change following order, make the same change in
*enum ID3V2_FOUR_CHARS_TAG_ENUMS.The order of these two must match exactly.
*/
static const byte frame_identifiers[ID3V2_FOUR_CHARS_TAG_MAX][5] =
{
  {0},   /* unknown frame type*/
  "AENC",/*Audio encryption*/
  "APIC",/*Attached picture*/
  "COMM",/*Comments*/
  "COMR",/*Commercial frame*/
  "ENCR",/*Encryption method registration*/
  "EQUA",/*Equalization*/
  "ETCO",/*Event timing codes*/
  "GEOB",/*General encapsulated object*/
  "GRID",/*Group identification registration*/
  "IPLS",/*Involved people list*/
  "LINK",/*Linked information*/
  "MCDI",/*Music CD identifier*/
  "MLLT",/*MPEG location lookup table*/
  "OWNE",/*Ownership frame*/
  "PRIV",/*Private frame*/
  "PCNT",/*Play counter*/
  "POPM",/*Popularimeter*/
  "POSS",/*Position synchronisation frame*/
  "RBUF",/*Recommended buffer size*/
  "RVAD",/*Relative volume adjustment*/
  "RVRB",/*Reverb*/
  "SYLT",/*Synchronized lyric/text*/
  "SYTC",/*Synchronized tempo codes*/
  "TALB",/*Album/Movie/Show title*/
  "TBPM",/*BPM (beats per minute)*/
  "TCOM",/*Composer*/
  "TCON",/*Content type*/
  "TCOP",/*Copyright message*/
  "TDAT",/*Date*/
  "TDLY",/*Playlist delay*/
  "TENC",/*Encoded by*/
  "TEXT",/*Lyricist/Text writer*/
  "TFLT",/*File type*/
  "TIME",/*Time*/
  "TIT1",/*Content group description*/
  "TIT2",/*Title/songname/content description*/
  "TIT3",/*Subtitle/Description refinement*/
  "TKEY",/*Initial key*/
  "TLAN",/*Language(s)*/
  "TLEN",/*Length*/
  "TMED",/*Media type*/
  "TOAL",/*Original album/movie/show title*/
  "TOFN",/*Original filename*/
  "TOLY",/*Original lyricist(s)/text writer(s)*/
  "TOPE",/*Original artist(s)/performer(s)*/
  "TORY",/*Original release year*/
  "TOWN",/*File owner/licensee*/
  "TPE1",/*Lead performer(s)/Soloist(s)*/
  "TPE2",/*Band/orchestra/accompaniment*/
  "TPE3",/*Conductor/performer refinement*/
  "TPE4",/*Interpretedremixedor otherwise modified by*/
  "TPOS",/*Part of a set*/
  "TPUB",/*Publisher*/
  "TRCK",/*Track number/Position in set*/
  "TRDA",/*Recording dates*/
  "TRSN",/*Internet radio station name*/
  "TRSO",/*Internet radio station owner*/
  "TSIZ",/*Size*/
  "TSRC",/*ISRC (international standard recording code)*/
  "TSSE",/*Software/Hardware and settings used for encoding*/
  "TYER",/*Year*/
  "TXXX",/*User defined text information frame*/
  "UFID",/*Unique file identifier*/
  "USER",/*Terms of use*/
  "USLT",/*Unsychronized lyric/text transcription*/
  "WCOM",/*Commercial information*/
  "WCOP",/*Copyright/Legal information*/
  "WOAF",/*Official audio file webpage*/
  "WOAR",/*Official artist/performer webpage*/
  "WOAS",/*Official audio source webpage*/
  "WORS",/*Official internet radio station homepage*/
  "WPAY",/*Payment*/
  "WPUB",/*Publishers official webpage*/
  "WXXX",/*User defined URL link frame*/
 };

/*
*Caution: Do not change the following order.
*If you have to change following order, make the same change in
*ID3V2_2_BELOW_TAG_ENUMS.The order of these two must match exactly.
*/
static const byte id3v2_2_below_frame_identifiers[ID3V2_THREE_CHARS_TAG_MAX][4] =
{
  {0},   /*unknown frame type*/
  "BUF", /*Recommended buffer size*/
  "CNT", /*Play counter*/
  "COM", /*Comments*/
  "CRA", /*Audio encryption*/
  "CRM", /*Encrypted meta frame*/
  "ETC", /*Event timing codes*/
  "EQU", /*Equalization*/
  "GEO", /*General encapsulated object*/
  "IPL", /*Involved people list*/
  "LNK", /*Linked information*/
  "MCI", /*Music CD Identifier*/
  "MLL", /*MPEG location lookup table*/
  "PIC", /*Attached picture*/
  "POP", /*Popularimeter*/
  "REV", /*Reverb*/
  "RVA", /*Relative volume adjustment*/
  "SLT", /*Synchronized lyric/text*/
  "STC", /*Synced tempo codes*/
  "TAL", /*Album/Movie/Show title*/
  "TBP", /*BPM (Beats Per Minute)*/
  "TCM", /*Composer*/
  "TCO", /*Content type*/
  "TCR", /*Copyright message*/
  "TDA", /*Date*/
  "TDY", /*Playlist delay*/
  "TEN", /*Encoded by*/
  "TFT", /*File type*/
  "TIM", /*Time*/
  "TKE", /*Initial key*/
  "TLA", /*Language(s)*/
  "TLE", /*Length*/
  "TMT", /*Media type*/
  "TOA", /*Original artist(s)/performer(s)*/
  "TOF", /*Original filename*/
  "TOL", /*Original Lyricist(s)/text writer(s)*/
  "TOR", /*Original release year*/
  "TOT", /*Original album/Movie/Show title*/
  "TP1", /*Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group*/
  "TP2", /*Band/Orchestra/Accompaniment*/
  "TP3", /*Conductor/Performer refinement*/
  "TP4", /*Interpreted, remixed, or otherwise modified by*/
  "TPA", /*Part of a set*/
  "TPB", /*Publisher*/
  "TRC", /*ISRC (International Standard Recording Code)*/
  "TRD", /*Recording dates*/
  "TRK", /*Track number/Position in set*/
  "TSI", /*Size*/
  "TSS", /*Software/hardware and settings used for encoding*/
  "TT1", /*Content group description*/
  "TT2", /*Title/Songname/Content description*/
  "TT3", /*Subtitle/Description refinement*/
  "TXT", /*Lyricist/text writer*/
  "TXX", /*User defined text information frame*/
  "TYE", /*Year*/
  "UFI", /*Unique file identifier*/
  "ULT", /*Unsychronized lyric/text transcription*/
  "WAF", /*Official audio file webpage*/
  "WAR", /*Official artist/performer webpage*/
  "WAS", /*Official audio source webpage*/
  "WCM", /*Commercial information*/
  "WCP", /*Copyright/Legal information*/
  "WPB", /*Publishers official webpage*/
  "WXX", /*User defined URL link frame*/
};
//=============================================================
// FUNCTION DEFINITONS
//=============================================================
// Simple file seek and read function
PARSER_ERRORTYPE seekandreadfile(OSCL_FILE* fp_ID3FilePtr, uint32 length,
                                 uint64 position, byte *pbuffer);

//=============================================================
//   Class ID3v1
//=============================================================
//=============================================================
//FUNCTION : Constructor
//
//DESCRIPTION
//  Constructor for ID3v1
//
//PARAMETERS
//  None
//
//RETURN VALUE
//  void
//
//SIDE EFFECTS
//  None
//=============================================================
ID3v1::ID3v1( PARSER_ERRORTYPE &rnResult)
{
  m_fileoffset =0;
  rnResult = PARSER_ErrorNone;
}
//=============================================================
//FUNCTION : Destructor
//
//DESCRIPTION
//  Destructor for ID3v1
//
//PARAMETERS
//  None
//
//RETURN VALUE
//  void
//
//SIDE EFFECTS
//  None
//=============================================================
ID3v1::~ID3v1()
{
   m_fileoffset =0;
}
//=============================================================
// FUNCTION : check_ID3v1_present
//
// DESCRIPTION
//  Static function that can be used to check the file to see if an ID3v1
//  tag is present. Once the caller knows that and ID3v1 tag is present an
//  object of this class can be created to extract the tag information.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  TRUE : ID3v1 tag is present in this file
//  FALSE : ID3v1 tag not present in this file
//
//SIDE EFFECTS
//  None
//=============================================================
bool ID3v1::check_ID3v1_present(OSCL_FILE* fp_ID3FilePtr, uint64 nLength)
{
  byte tagheader[4] = {0};
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  if(NULL == fp_ID3FilePtr || (MAX_FILE_SIZE == nLength))
  {
    return FALSE;
  }
  if(nLength < 128)
  {
    return FALSE;
  }
  result = seekandreadfile(fp_ID3FilePtr, 3, (int64)(nLength-ID3v1_SIZE),
                           tagheader);
  if(result)
  {
    return FALSE;
  }
  if(!memcmp("TAG",tagheader,3))
  {
    return TRUE;
  }
  return FALSE;
}
//=============================================================
// FUNCTION : parse_ID3v1_tag
//
// DESCRIPTION
//  Parses and extracts all the info present in the ID3v1 tag
//  and populates this info into the metadata_id3v1_type struct
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  nLength : total length of the input file in bytes
//  pstid3v1 : output param to be filled by this method
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v1::parse_ID3v1_tag(OSCL_FILE* fp_ID3FilePtr,
                                        metadata_id3v1_type *pstid3v1,
                                        uint64 nLength)
{
  if((NULL == fp_ID3FilePtr)||(NULL == pstid3v1))
  {
    return PARSER_ErrorInvalidParam;
  }

  memset(pstid3v1, 0, sizeof(metadata_id3v1_type));
  if(nLength < 10)
  {
    return PARSER_ErrorInvalidParam;
  }
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  byte buffer[128] ={0};
  byte *bufferreadpos = buffer;
  m_fileoffset = nLength-ID3v1_SIZE;
  pstid3v1->file_position = m_fileoffset;
  result = seekandreadfile(fp_ID3FilePtr, ID3v1_SIZE, m_fileoffset, buffer);
  if(result)
  {
    return result;
  }
  // Skip 'TAG' id
  pstid3v1->size = ID3v1_SIZE;
  bufferreadpos += 3;
  // Copy Title
  (void) memmove(pstid3v1->title, bufferreadpos, ID3V1_TITLE_LENGTH);
  bufferreadpos += ID3V1_TITLE_LENGTH;
  // Copy artist
  (void) memmove(pstid3v1->artist, bufferreadpos, ID3V1_ARTIST_LENGTH);
  bufferreadpos += ID3V1_ARTIST_LENGTH;
  // Copy album
  (void) memmove(pstid3v1->album, bufferreadpos, ID3V1_ALBUM_LENGTH);
  bufferreadpos += ID3V1_ALBUM_LENGTH;
  // Copy year
  (void) memmove(pstid3v1->year, bufferreadpos, ID3V1_YEAR_LENGTH);
  bufferreadpos += ID3V1_YEAR_LENGTH;
  // Copy comment
  (void) memmove(pstid3v1->comment, bufferreadpos, ID3V1_COMMENT_LENGTH);
  bufferreadpos += ID3V1_COMMENT_LENGTH;
  // Copy track
  pstid3v1->track = *bufferreadpos;
  bufferreadpos++;
  // Copy genre
  pstid3v1->genre = *bufferreadpos;
  bufferreadpos++;
  return result;
}
//=============================================================
//   Class ID3v2
//=============================================================
//=============================================================
//FUNCTION : Constructor
//
//DESCRIPTION
//  Constructor for ID3v2
//
//PARAMETERS
//  None
//
//RETURN VALUE
//  void
//
//SIDE EFFECTS
//  None
//=============================================================
ID3v2::ID3v2( PARSER_ERRORTYPE &rnResult)
{
  init_ID3v2_params();
  rnResult = PARSER_ErrorNone;
}
//=============================================================
//FUNCTION : Destructor
//
//DESCRIPTION
//  Destructor for ID3v2
//
//PARAMETERS
//  None
//
//RETURN VALUE
//  void
//
//SIDE EFFECTS
//  None
//=============================================================
ID3v2::~ID3v2()
{
  m_pstid3v2 = NULL;
}
//=============================================================
// FUNCTION : init_ID3v2_params
//
// DESCRIPTION
//  Initializes the member variables of this class used for parsing the tag
//
// PARAMETERS
//  None
//
//RETURN VALUE
//  void
//
//SIDE EFFECTS
//  None
//=============================================================
void ID3v2::init_ID3v2_params()
{
  m_filereadoffset = 0;
  m_ID3v2startpos =0;
  m_size = 0;
  m_umajorversion = 0;
  m_uminorversion = 0;

  m_bunsncyhronisation = FALSE;
  m_bextendedheader = FALSE;
  m_bexperimentalheader = FALSE;
  m_bfooterpresent = FALSE;
  m_pstid3v2 = NULL;
}
//=============================================================
// FUNCTION : check_ID3v2_present
//
// DESCRIPTION
//   static function that can be used to check the file to see if an ID3v2 tag
//   is present. Also sets a flag to inform the caller as to whether the tag
//   is appended at theof the file after the audio data or before the start of
//   audio data in the beginning of the file. Once the caller knows that and
//   ID3v2 tag is present an object of this class can be created to extract the
//   tag information.
//
// PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  nLength : total length of the input file in bytes
//  pbID3v2postend : output param to be filled by this method
//                             flag to show if ID3v2 tag is appended after
//                             the end of audio data in the file.
//
//RETURN VALUE
//  TRUE : ID3v2 tag is present in this file
//  FALSE : ID3v2 tag not present in this file
//
//SIDE EFFECTS
//  None
//=============================================================
bool ID3v2::check_ID3v2_present(OSCL_FILE* fp_ID3FilePtr, uint64 nLength,
                                uint64 nOffset, bool *pbID3v2postend)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  byte tagheader[4] = {0};
  if((NULL == fp_ID3FilePtr)||(NULL == pbID3v2postend))
  {
    return FALSE;
  }
  if(nLength < 10)
  {
    return FALSE;
  }
  // Check for Id3v2 tag at the given offset
  result = seekandreadfile(fp_ID3FilePtr,3,nOffset,tagheader);
  if(result)
  {
   return FALSE;
  }
  if(!memcmp("ID3",tagheader, 3))
  {
    *pbID3v2postend = FALSE;
    return TRUE;
  }
  // Check for Id3v2 tag at the end of the file if file-size known
  // & there is no ID3 tag detected at beginning.

  if((nLength != MAX_FILE_SIZE) && (false == *pbID3v2postend))
  {
    result = seekandreadfile(fp_ID3FilePtr,3,nLength-10,tagheader);
    if(result)
    {
      return FALSE;
    }
    if(!memcmp("3DI",tagheader, 3))
    {
      *pbID3v2postend = TRUE;
      return TRUE;
    }
  }
  return FALSE;
}
//=============================================================
// FUNCTION : parse_ID3v2_tag
//
// DESCRIPTION
//  Parses and extracts all the info present in the ID3v2 tag
//  and populates this info into the metadata_id3v2_type struct
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  bID3v2postend : flag to show if ID3v2 tag is appended after
//                           the end of audio data in the file
//  pstid3v2 : output param to be filled by this method
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::parse_ID3v2_tag(OSCL_FILE* fp_ID3FilePtr,
                                        uint64 nOffset,
                                        metadata_id3v2_type *pstid3v2,
                                        bool bID3v2postend)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   if((NULL == fp_ID3FilePtr)||(NULL == pstid3v2))
   {
     return PARSER_ErrorInvalidParam;
   }
   init_ID3v2_params();
   (void) memset((void*)pstid3v2, 0, sizeof(metadata_id3v2_type));
   if(bID3v2postend)
   {
     result = parse_ID3v2_tag_postend(fp_ID3FilePtr,nOffset,pstid3v2,
                                      /*TBD*/ 50);
   }
   else
   {
     result = parse_ID3v2_tag_prepend(fp_ID3FilePtr,nOffset, pstid3v2,
                                      /*TBD*/ 50);
   }
   return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_tag_postend
//
// DESCRIPTION
//  Parses and extracts all the info present in the ID3v2 tag
//  and populates this info into the metadata_id3v2_type struct
//  when the tag is present after the end of audio data in the file
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  pstid3v2 : output param to be filled by this method
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::parse_ID3v2_tag_postend(OSCL_FILE* fp_ID3FilePtr,
                                                uint64 nOffset,
                                                metadata_id3v2_type *pstid3v2,
                                                uint64 nLength)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   // Move m_filereadoffset to the beginning of the tag
   // TBD
   m_filereadoffset = nLength -10;
   result = parse_ID3v2_tag_header(fp_ID3FilePtr,nOffset, pstid3v2,nLength);
   return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_tag_prepend
//
// DESCRIPTION
//  Parses and extracts all the info present in the ID3v2 tag
//  and populates this info into the metadata_id3v2_type struct
//  when the tag is present before the start of audio data in the file
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  pstid3v2 : output param to be filled by this method
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//void
//
//SIDE EFFECTS
//  None
//=============================================================
//
PARSER_ERRORTYPE ID3v2::parse_ID3v2_tag_prepend(OSCL_FILE* fp_ID3FilePtr,
                                                uint64 nOffset,
                                                metadata_id3v2_type *pstid3v2,
                                                uint64 nLength)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   m_filereadoffset = 0;
   result = parse_ID3v2_tag_header(fp_ID3FilePtr,nOffset, pstid3v2,nLength);
   if(result)
   {
     return result;
   }
   result = parse_ID3v2_frames(fp_ID3FilePtr,pstid3v2,nLength);
   return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_tag_header
//
// DESCRIPTION
//  Parses and extracts all the info present in the ID3v2 tag header
//  and populates this info into the metadata_id3v2_type struct
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  pstid3v2 : output param to be filled by this method
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::parse_ID3v2_tag_header(OSCL_FILE* fp_ID3FilePtr,
                                               uint64 nOffset,
                                               metadata_id3v2_type *pstid3v2,
                                               uint64 nLength)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   uint64 offset = nOffset;
   byte buffer[4] ={0};
   (void) nLength;
   pstid3v2->is_present = TRUE;
   m_ID3v2startpos = m_filereadoffset;
   pstid3v2->file_position = nOffset;
   // Skip the tag name
   offset += 3;
   result = seekandreadfile(fp_ID3FilePtr,2, offset,buffer);
   if(result)
   {
     //DBG(HIGH,"ID3v2::parse_ID3v2_tag_header : seekandreadfile failed");
     return result;
   }
   offset += 2;
   m_umajorversion = buffer[0];
   m_uminorversion = buffer[1];
   uint8 flags = 0;
   result = seekandreadfile(fp_ID3FilePtr,1, offset,&flags);
   if(result)
   {
     //DBG(HIGH,"ID3v2::parse_ID3v2_tag_header : seekandreadfile failed");
     return result;
   }
   offset += 1;
   m_bunsncyhronisation = (flags & 0x80)?TRUE:FALSE;
   m_bextendedheader = (flags & 0x40)?TRUE:FALSE;
   m_bexperimentalheader = (flags & 0x20)?TRUE:FALSE;
   m_bfooterpresent = (flags & 0x10)?TRUE:FALSE;
   result = seekandreadfile(fp_ID3FilePtr,4, offset,buffer);
   if(result)
   {
     //DBG(HIGH,"ID3v2::parse_ID3v2_tag_header : seekandreadfile failed");
     return result;
   }
   offset += 4;
   // Convert the size value read from the tag to int
   m_size = getsyncsafeinteger(buffer,4) + ID3v2_HEADER_SIZE;
   pstid3v2->size = m_size;
   m_filereadoffset += ID3v2_HEADER_SIZE;
   m_filereadoffset+= nOffset;
   // If an extended header is present, skip it
   if(m_bextendedheader)
   {
     result = seekandreadfile(fp_ID3FilePtr,4, m_filereadoffset,buffer);
     if(result)
     {
       //DBG(HIGH,"ID3v2::parse_ID3v2_tag_header : seekandreadfile failed");
       return result;
     }
     m_filereadoffset += 4;
     // Increment offset by size of the extended header
     uint32 exthdrsize = parse_ID3v2_uint(buffer, 4);
     result = seekandreadfile(fp_ID3FilePtr,1, m_filereadoffset,buffer);
     if(result)
     {
       //DBG(HIGH,"ID3v2::parse_ID3v2_tag_header : seekandreadfile failed");
       return result;
     }
     //Increment offset by size of flags field(1 byte) + flags field(buffer[0])
     m_filereadoffset += 1 + buffer[0];
     // Skip the ext header field
     m_filereadoffset += exthdrsize;
   }
   return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_frames
//
// DESCRIPTION
//  Parses all frames in the ID3v2 tag one by one till the end of the tag
//  or till it encounters padding.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  pstid3v2 : output param to be filled by this method
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::parse_ID3v2_frames(OSCL_FILE* fp_ID3FilePtr,
                                           metadata_id3v2_type *pstid3v2,
                                           uint64 /*nLength*/)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   uint64 frame_length;
   ID3V2_FOUR_CHARS_FRAME_TYPES frame_type = ID3V2_FOUR_CHARS_TAG_UNKN;
   ID3V2_THREE_CHARS_FRAME_TYPES id3v2_below_frame_type = ID3V2_THREE_CHARS_TAG_UNK;
   uint64 tagsize = m_size;
   int hdrsize = GET_ID3V2_FRAME_HDR_SIZE(m_umajorversion) ;
   if(m_bfooterpresent)
   {
     tagsize -= hdrsize;
   }
   m_pstid3v2 = pstid3v2;
   m_pstid3v2->generic_frames = NULL;
   uint64 nendoffset = m_filereadoffset;
   if((m_filereadoffset + tagsize)> ID3v2_HEADER_SIZE)
   {
     nendoffset = m_filereadoffset + tagsize - ID3v2_HEADER_SIZE;
   }
   while(m_filereadoffset < nendoffset)
   {
     frame_length = 0;
     if(m_umajorversion >= 3)
     {
       frame_type = ID3V2_FOUR_CHARS_TAG_UNKN;
       result = get_next_frame(fp_ID3FilePtr,&frame_type,&frame_length);
     }
     else
     {
       id3v2_below_frame_type = ID3V2_THREE_CHARS_TAG_UNK;
       result = get_next_frame(fp_ID3FilePtr,&id3v2_below_frame_type,&frame_length);
     }
     if((result) || (frame_length == 0))
     {
       m_filereadoffset += 10;
       continue;;
     }
     // As long as this frame completely exists, parse it even if its type
     // is unknown parse_ID3v2_frame will take care of hanlding the frame type
     if(m_filereadoffset + hdrsize + frame_length <= nendoffset)
     {
       if(m_umajorversion >= 3)
       {
         result = parse_ID3v2_frame(fp_ID3FilePtr,frame_type,frame_length);
       }
       else
       {
         result = parse_ID3v2_frame(fp_ID3FilePtr,id3v2_below_frame_type,
                                    frame_length);
       }
       if(result)
       {
         break;
       }
     }
     else
     {
       // Always make sure m_filereadoffset is updated with frame size to
       // avoid infinite loop
        m_filereadoffset+= (frame_length+hdrsize);
     }
     // When we encounter padding, we can stop parsing
     if(check_padding(fp_ID3FilePtr))
     {
       break;
     }
   }
   return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_uint
//
// DESCRIPTION
//   Parses a single unit of an ID3v2 tag. This process is dependent on the
//   major version of the spec being used. A max of 4 bytes can be handled
//   at a time by this method.
//
//PARAMETERS
//  pInput : Input data (byte stream)
//  nbytes : number of bytes in the input data
//
//RETURN VALUE
//  uint32 : output data after parsing
//
//SIDE EFFECTS
//  None
//=============================================================
uint32 ID3v2::parse_ID3v2_uint(const uint8 *pInput,
                               unsigned int nbytes) const
{
  uint32 uoutputint = 0;
  if(4 == m_umajorversion)
  {
    return getsyncsafeinteger(pInput, nbytes);
  }
  for(;nbytes > 0;nbytes--,pInput++)
  {
    uoutputint = (uoutputint << 8) |*pInput;
  }
  return uoutputint;
}
//=============================================================
// FUNCTION : parse_ID3v2_frame
//
// DESCRIPTION
//  This method handles the parsing of a single ID3v2 frame.
//  Respective handlers for frame types are called from this method
//  to handle the parsing of the frame.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  frame_type : type of the frame that needs to be parsed
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::parse_ID3v2_frame(OSCL_FILE* fp_ID3FilePtr,
                                    ID3V2_FOUR_CHARS_FRAME_TYPES frame_type,
                                    uint64 frame_length)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  if(frame_length ==0)
  {
    return PARSER_ErrorInvalidParam;
  }
  // If this is an unknown type, check if this frame type needs to be supported
  // if yes, then parse it as a generic frame
  if(frame_type == ID3V2_FOUR_CHARS_TAG_UNKN)
  {
    byte frame_id[ID3V2_3_4_FRAME_ID_SIZE] = {0};
    result = seekandreadfile(fp_ID3FilePtr, ID3V2_3_4_FRAME_ID_SIZE,
                             m_filereadoffset, frame_id);
    if(result)
    {
      //DBG(HIGH,"ID3v2::parse_ID3v2_frame: seekandreadfile failed");
      return result;
    }
    if(ID3V2_IS_FRAME_SUPPORTED(frame_id))
    {
      result = parse_ID3v2_frame_generic(fp_ID3FilePtr,frame_id,frame_length);
    }
    else
    {
      // Skip this frame's data
      // Move the file read offset to mark that this data is consumed
      m_filereadoffset += ID3V2_3_4_FRAME_HEADER_SIZE + frame_length;
    }
    return result;
  }
  text_frame* dataptr = NULL;
  private_tag_info* privTag = NULL;
  id3v2_pic_info* pict_tag  = NULL;
  encoder_delay_tag_info* pEncTag = NULL;
  // Parse a known frame type
  switch(frame_type)
  {
    case ID3V2_FOUR_CHARS_TAG_IPLS:
      dataptr = &m_pstid3v2->involved_people_list;
      break;
    case ID3V2_FOUR_CHARS_TAG_TALB:
      dataptr = &m_pstid3v2->album;
      break;
    case ID3V2_FOUR_CHARS_TAG_TBPM:
      dataptr = &m_pstid3v2->beats_per_minute;
      break;
    case ID3V2_FOUR_CHARS_TAG_TCOM:
      dataptr = &m_pstid3v2->composer;
      break;
    case ID3V2_FOUR_CHARS_TAG_TCON:
      dataptr = &m_pstid3v2->genre;
      break;
    case ID3V2_FOUR_CHARS_TAG_TCOP:
      dataptr = &m_pstid3v2->copyright;
      break;
    case ID3V2_FOUR_CHARS_TAG_TDAT:
      dataptr = &m_pstid3v2->date;
      break;
    case ID3V2_FOUR_CHARS_TAG_TDLY:
      dataptr = &m_pstid3v2->playlist_delay;
      break;
    case ID3V2_FOUR_CHARS_TAG_TENC:
      dataptr = &m_pstid3v2->encoder;
      break;
    case ID3V2_FOUR_CHARS_TAG_TEXT:
      dataptr = &m_pstid3v2->lyricist;
      break;
    case ID3V2_FOUR_CHARS_TAG_TFLT:
      dataptr = &m_pstid3v2->file_type;
      break;
    case ID3V2_FOUR_CHARS_TAG_TIME:
      dataptr = &m_pstid3v2->time;
      break;
    case ID3V2_FOUR_CHARS_TAG_TIT1:
      dataptr = &m_pstid3v2->content_group_desc;
      break;
    case ID3V2_FOUR_CHARS_TAG_TIT2:
      dataptr = &m_pstid3v2->title;
      break;
    case ID3V2_FOUR_CHARS_TAG_TIT3:
      dataptr = &m_pstid3v2->subtitle;
      break;
    case ID3V2_FOUR_CHARS_TAG_TKEY:
      dataptr = &m_pstid3v2->init_key;
      break;
    case ID3V2_FOUR_CHARS_TAG_TLAN:
      dataptr = &m_pstid3v2->languages;
      break;
    case ID3V2_FOUR_CHARS_TAG_TLEN:
      dataptr = &m_pstid3v2->length;
      break;
    case ID3V2_FOUR_CHARS_TAG_TMED:
      dataptr = &m_pstid3v2->media_type;
      break;
    case ID3V2_FOUR_CHARS_TAG_TOAL:
      dataptr = &m_pstid3v2->original_title;
      break;
    case ID3V2_FOUR_CHARS_TAG_TOFN:
      dataptr = &m_pstid3v2->original_filename;
      break;
    case ID3V2_FOUR_CHARS_TAG_TOLY:
      dataptr = &m_pstid3v2->original_lyricists;
      break;
    case ID3V2_FOUR_CHARS_TAG_TOPE:
      dataptr = &m_pstid3v2->original_artist;
      break;
    case ID3V2_FOUR_CHARS_TAG_TORY:
      dataptr = &m_pstid3v2->original_release_year;
      break;
    case ID3V2_FOUR_CHARS_TAG_TOWN:
      dataptr = &m_pstid3v2->file_own_licensee;
      break;
    case ID3V2_FOUR_CHARS_TAG_TPE1:
      dataptr = &m_pstid3v2->album_artist;
      break;
    case ID3V2_FOUR_CHARS_TAG_TPE2:
      dataptr = &m_pstid3v2->orchestra;
      break;
    case ID3V2_FOUR_CHARS_TAG_TPE3:
      dataptr = &m_pstid3v2->conductor;
      break;
    case ID3V2_FOUR_CHARS_TAG_TPE4:
      dataptr = &m_pstid3v2->remix_mod_by;
      break;
    case ID3V2_FOUR_CHARS_TAG_TPOS:
      dataptr = &m_pstid3v2->part_of_set;
      break;
    case ID3V2_FOUR_CHARS_TAG_TPUB:
      dataptr = &m_pstid3v2->publisher;
      break;
    case ID3V2_FOUR_CHARS_TAG_TRCK:
      dataptr = &m_pstid3v2->track;
      break;
    case ID3V2_FOUR_CHARS_TAG_TRDA:
      dataptr = &m_pstid3v2->recording_dates;
      break;
    case ID3V2_FOUR_CHARS_TAG_TRSN:
      dataptr = &m_pstid3v2->internet_radio_stn_name;
      break;
    case ID3V2_FOUR_CHARS_TAG_TRSO:
      dataptr = &m_pstid3v2->internet_radio_stn_owner;
      break;
    case ID3V2_FOUR_CHARS_TAG_TSIZ:
      dataptr = &m_pstid3v2->size_in_bytes;
      break;
    case ID3V2_FOUR_CHARS_TAG_TSRC:
      dataptr = &m_pstid3v2->isrc_code;
      break;
    case ID3V2_FOUR_CHARS_TAG_TSSE:
      dataptr = &m_pstid3v2->sw_hw_enc_settings;
      break;
    case ID3V2_FOUR_CHARS_TAG_TYER:
      dataptr = &m_pstid3v2->year;
      break;
    case ID3V2_FOUR_CHARS_TAG_PRIV:
      privTag = &m_pstid3v2->private_tag;
      break;
    case ID3V2_FOUR_CHARS_TAG_APIC:/*custom frame*/
      pict_tag = &m_pstid3v2->pic_info;
      break;
    case ID3V2_FOUR_CHARS_TAG_COMM:/*comment frame*/
      pEncTag = &m_pstid3v2->encoder_delay_tag;
      break;
#if 0
    //skip all custom and unknown frame types
    case ID3V2_FOUR_CHARS_TAG_AENC:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_COMR:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_ENCR:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_EQUA:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_ETCO:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_GEOB:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_GRID:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_LINK:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_MCDI:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_MLLT:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_OWNE:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_PCNT:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_POPM:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_POSS:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_RBUF:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_RVAD:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_RVRB:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_SYLT:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_SYTC:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_TXXX:/*user defined test frame*/
    case ID3V2_FOUR_CHARS_TAG_UFID:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_USER:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_USLT:/*custom frame*/
    case ID3V2_FOUR_CHARS_TAG_WCOM:/*user link frame*/
    case ID3V2_FOUR_CHARS_TAG_WCOP:/*user link frame*/
    case ID3V2_FOUR_CHARS_TAG_WOAF:/*user link frame*/
    case ID3V2_FOUR_CHARS_TAG_WOAR:/*user link frame*/
    case ID3V2_FOUR_CHARS_TAG_WOAS:/*user link frame*/
    case ID3V2_FOUR_CHARS_TAG_WORS:/*user link frame*/
    case ID3V2_FOUR_CHARS_TAG_WPAY:/*user link frame*/
    case ID3V2_FOUR_CHARS_TAG_WPUB:/*user link frame*/
    case ID3V2_FOUR_CHARS_TAG_WXXX:/*user defined url link frame*/
#endif
    default:
    {
      result = PARSER_ErrorNone;
    }
  }
  if(dataptr)
  {
    result = parse_ID3v2_text_frame(fp_ID3FilePtr,dataptr,frame_length);
  }
  if(privTag)
  {
     result = parse_ID3v2_priv_frame(fp_ID3FilePtr,privTag,frame_length);
  }
  if(pict_tag)
  {
    result = parse_ID3v2_frame_APIC(fp_ID3FilePtr,pict_tag,frame_length);
  }
  else if (pEncTag)
  {
    result = parse_ID3v2_comment_frame(fp_ID3FilePtr, pEncTag, frame_length);
  }
  // Move the file read offset to mark that this data is consumed
  m_filereadoffset += ID3V2_3_4_FRAME_HEADER_SIZE + frame_length;
  return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_frame
//
// DESCRIPTION
//  This method handles the parsing of a single ID3v2 frame.
//  Respective handlers for frame types are called from this method
//  to handle the parsing of the frame.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  frame_type : type of the frame that needs to be parsed
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::parse_ID3v2_frame(OSCL_FILE* fp_ID3FilePtr,
                                    ID3V2_THREE_CHARS_FRAME_TYPES frame_type,
                                    uint64 frame_length)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  if(frame_length ==0)
  {
    return PARSER_ErrorInvalidParam;
  }
  // If this is an unknown type, skip the frame
  if(frame_type == ID3V2_THREE_CHARS_TAG_UNK)
  {
    m_filereadoffset += ID3V2_2_BELOW_FRAME_HEADER_SIZE + frame_length;
    return result;
  }
  text_frame* dataptr = NULL;
  encoder_delay_tag_info* pEncTag = NULL;
  // Parse a known frame type
  switch(frame_type)
  {
    case ID3V2_THREE_CHARS_TAG_IPL:
      dataptr = &m_pstid3v2->involved_people_list;
      break;
    case ID3V2_THREE_CHARS_TAG_TAL:
      dataptr = &m_pstid3v2->album;
      break;
    case ID3V2_THREE_CHARS_TAG_TBP:
      dataptr = &m_pstid3v2->beats_per_minute;
      break;
    case ID3V2_THREE_CHARS_TAG_TCM:
      dataptr = &m_pstid3v2->composer;
      break;
    case ID3V2_THREE_CHARS_TAG_TCO:
      dataptr = &m_pstid3v2->genre;
      break;
    case ID3V2_THREE_CHARS_TAG_TCR:
      dataptr = &m_pstid3v2->copyright;
      break;
    case ID3V2_THREE_CHARS_TAG_TDA:
      dataptr = &m_pstid3v2->date;
      break;
    case ID3V2_THREE_CHARS_TAG_TDY:
      dataptr = &m_pstid3v2->playlist_delay;
      break;
    case ID3V2_THREE_CHARS_TAG_TEN:
      dataptr = &m_pstid3v2->encoder;
      break;
    case ID3V2_THREE_CHARS_TAG_TFT:
      dataptr = &m_pstid3v2->file_type;
      break;
    case ID3V2_THREE_CHARS_TAG_TIM:
      dataptr = &m_pstid3v2->time;
      break;
    case ID3V2_THREE_CHARS_TAG_TKE:
      dataptr = &m_pstid3v2->init_key;
      break;
    case ID3V2_THREE_CHARS_TAG_TLA:
      dataptr = &m_pstid3v2->languages;
      break;
    case ID3V2_THREE_CHARS_TAG_TLE:
      dataptr = &m_pstid3v2->length;
      break;
    case ID3V2_THREE_CHARS_TAG_TMT:
      dataptr = &m_pstid3v2->media_type;
      break;
    case ID3V2_THREE_CHARS_TAG_TOA:
      dataptr = &m_pstid3v2->original_artist;
      break;
    case ID3V2_THREE_CHARS_TAG_TOF:
      dataptr = &m_pstid3v2->original_filename;
      break;
    case ID3V2_THREE_CHARS_TAG_TOL:
      dataptr = &m_pstid3v2->recomm_buff_size;
      break;
    case ID3V2_THREE_CHARS_TAG_TOR:
      dataptr = &m_pstid3v2->original_release_year;
      break;
    case ID3V2_THREE_CHARS_TAG_TOT:
      dataptr = &m_pstid3v2->original_title;
      break;
    case ID3V2_THREE_CHARS_TAG_TP1:
      dataptr = &m_pstid3v2->album_artist;
      break;
    case ID3V2_THREE_CHARS_TAG_TP2:
      dataptr = &m_pstid3v2->orchestra;
      break;
    case ID3V2_THREE_CHARS_TAG_TP3:
      dataptr = &m_pstid3v2->conductor;
      break;
    case ID3V2_THREE_CHARS_TAG_TP4:
      dataptr = &m_pstid3v2->remix_mod_by;
      break;
    case ID3V2_THREE_CHARS_TAG_TPA:
      dataptr = &m_pstid3v2->part_of_set;
      break;
    case ID3V2_THREE_CHARS_TAG_TPB:
      dataptr = &m_pstid3v2->publisher;
      break;
    case ID3V2_THREE_CHARS_TAG_TRC:
      dataptr = &m_pstid3v2->isrc_code;
      break;
    case ID3V2_THREE_CHARS_TAG_TRD:
      dataptr = &m_pstid3v2->recording_dates;
      break;
    case ID3V2_THREE_CHARS_TAG_TRK:
      dataptr = &m_pstid3v2->track;
      break;
    case ID3V2_THREE_CHARS_TAG_TSI:
      dataptr = &m_pstid3v2->size_in_bytes;
      break;
    case ID3V2_THREE_CHARS_TAG_TSS:
      dataptr = &m_pstid3v2->sw_hw_enc_settings;
      break;
    case ID3V2_THREE_CHARS_TAG_TT1:
      dataptr = &m_pstid3v2->content_group_desc;
      break;
    case ID3V2_THREE_CHARS_TAG_TT2:
      dataptr = &m_pstid3v2->title;
      break;
    case ID3V2_THREE_CHARS_TAG_TT3:
      dataptr = &m_pstid3v2->subtitle;
      break;
    case ID3V2_THREE_CHARS_TAG_TXT:
      dataptr = &m_pstid3v2->lyricist;
      break;
    case ID3V2_THREE_CHARS_TAG_TXX:
      dataptr = &m_pstid3v2->user_def_text_info;
      break;
    case ID3V2_THREE_CHARS_TAG_TYE:
      dataptr = &m_pstid3v2->year;
      break;
    case ID3V2_THREE_CHARS_TAG_COM://custom frame
      pEncTag = &m_pstid3v2->encoder_delay_tag;
      break;
    //skip all custom,unknown frames
#if 0
    case ID3V2_THREE_CHARS_TAG_BUF://custom frame
    case ID3V2_THREE_CHARS_TAG_CNT://custom frame
    case ID3V2_THREE_CHARS_TAG_CRA://custom frame
    case ID3V2_THREE_CHARS_TAG_CRM://custom frame
    case ID3V2_THREE_CHARS_TAG_ETC://custom frame
    case ID3V2_THREE_CHARS_TAG_EQU://custom frame
    case ID3V2_THREE_CHARS_TAG_GEO://custom frame
    case ID3V2_THREE_CHARS_TAG_LNK://custom frame
    case ID3V2_THREE_CHARS_TAG_MCI://custom frame
    case ID3V2_THREE_CHARS_TAG_MLL://custom frame
    case ID3V2_THREE_CHARS_TAG_PIC://custom frame
    case ID3V2_THREE_CHARS_TAG_POP://custom frame
    case ID3V2_THREE_CHARS_TAG_REV://custom frame
    case ID3V2_THREE_CHARS_TAG_RVA://custom frame
    case ID3V2_THREE_CHARS_TAG_SLT://custom frame
    case ID3V2_THREE_CHARS_TAG_STC://custom frame
    case ID3V2_THREE_CHARS_TAG_UFI://custom frame
    case ID3V2_THREE_CHARS_TAG_ULT://custom frame
    case ID3V2_THREE_CHARS_TAG_WAF://user link frame
    case ID3V2_THREE_CHARS_TAG_WAR://user link frame
    case ID3V2_THREE_CHARS_TAG_WAS://user link frame
    case ID3V2_THREE_CHARS_TAG_WCM://user link frame
    case ID3V2_THREE_CHARS_TAG_WCP://user link frame
    case ID3V2_THREE_CHARS_TAG_WPB://user link frame
    case ID3V2_THREE_CHARS_TAG_WXX://user defined url link frame
#endif
    default:
       result = PARSER_ErrorNone;
    break;
  }
  if(dataptr)
  {
    result = parse_ID3v2_text_frame(fp_ID3FilePtr,dataptr,frame_length);
  }
  else if (pEncTag)
  {
    result = parse_ID3v2_comment_frame(fp_ID3FilePtr, pEncTag, frame_length);
  }
  // Move the file read offset to mark that this data is consumed
  m_filereadoffset += GET_ID3V2_FRAME_HDR_SIZE(m_umajorversion) + frame_length;
  return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_frame_generic
//
// DESCRIPTION
//   This method handles the parsing of generic frames.
//   These are frames that are classified as supported frame types but
//   do not have any specific handling for that frame type.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  pframe_id : 4 byte frame id for this frame
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  Allocates memory for a complete generic frame.
//  This memory has to be freed by the caller of the ID3 methods.
//=============================================================
PARSER_ERRORTYPE ID3v2::parse_ID3v2_frame_generic(OSCL_FILE* fp_ID3FilePtr,
                                                  const byte *pframe_id,
                                                  uint64 frame_length)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  id3v2_generic_frame_type *pgenframe = NULL;
  if(NULL == m_pstid3v2->generic_frames)
  {
    m_pstid3v2->generic_framesLen = 1;
    // This is the first generic frame
    // Allocate memory for reading data
    m_pstid3v2->generic_frames = (id3v2_generic_frame_type *)
                                 MM_Malloc(sizeof(id3v2_generic_frame_type));
    if(m_pstid3v2->generic_frames)
    {
      memset(m_pstid3v2->generic_frames,0,sizeof(id3v2_generic_frame_type));
    }
  }
  else
  {
    id3v2_generic_frame_type* pTemp =
      (id3v2_generic_frame_type *)
       MM_Realloc((void*)(m_pstid3v2->generic_frames),
                          sizeof(id3v2_generic_frame_type)*
                          (m_pstid3v2->generic_framesLen+1) );
     if(pTemp)
     {
       m_pstid3v2->generic_framesLen++;
       memset(pTemp + (m_pstid3v2->generic_framesLen-1), 0,
              sizeof(id3v2_generic_frame_type));
       m_pstid3v2->generic_frames = pTemp;
     }
     else
     {
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                    "parse_ID3v2_frame_generic ID3_INSUFFICIENT_BUFFER");
       return PARSER_ErrorInsufficientBufSize;
     }
   }
   if(NULL == m_pstid3v2->generic_frames)
   {
     //DBG(HIGH,"ID3v2::parse_ID3v2_frame_generic: No memory");
     return PARSER_ErrorInsufficientBufSize;
   }
   pgenframe = &(m_pstid3v2->generic_frames[m_pstid3v2->generic_framesLen -1]);
   // Set the frame ID
  (void) memmove(pgenframe->id,pframe_id,ID3V2_3_4_FRAME_ID_SIZE);
   byte ucflags[2] = {0};
   m_filereadoffset += ID3V2_3_4_FRAME_ID_SIZE + ID3V2_3_4_FRAME_LENGTH_SIZE;
   // Get the 2 flag bytes
   result = seekandreadfile(fp_ID3FilePtr,2, m_filereadoffset,ucflags);
   if(result)
   {
      //DBG(HIGH,"ID3v2::parse_ID3v2_frame_generic: seekandreadfile failed");
      return result;
   }
   m_filereadoffset += 2;
   pgenframe->flags = uint16((ucflags[0]<<8)|ucflags[1]);
   pgenframe->in_memory = TRUE;
   pgenframe->contentLen = (int)frame_length;
   // Allocate memory for content
   pgenframe->content = MM_New_Array(byte,((int)frame_length));
   if(NULL == pgenframe->content)
   {
     //DBG(HIGH,"ID3v2::parse_ID3v2_frame_generic: No memory");
     return result;
   }
   // Read the content into this allocated memory
   result = seekandreadfile(fp_ID3FilePtr, (uint32)frame_length,
                            m_filereadoffset, pgenframe->content);

   if(result)
   {
     //DBG(HIGH,"ID3v2::parse_ID3v2_frame_generic: seekandreadfile failed");
     return result;
   }
   // Move the file read offset to mark that this data is consumed
   m_filereadoffset += frame_length;
   return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_frame_APIC
//
// DESCRIPTION
//   This method handles the parsing of an APIC frame.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::parse_ID3v2_frame_APIC(OSCL_FILE* fp_ID3FilePtr,
                                               id3v2_pic_info* pict_tag,
                                               uint64 frame_length)
{
   uint8 *temp = NULL;
   uint32 buf_offset = 0;
   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   if(frame_length)
   {
     temp = MM_New_Array(uint8,uint32(FIXED_APIC_FIELDS_LENGTH));
     if(NULL == temp)
     {
       return PARSER_ErrorMemAllocFail;
     }
     uint64 offset;
     offset = m_filereadoffset + GET_ID3V2_FRAME_HDR_SIZE(m_umajorversion);
     // Read the fixed bytes of frame data into temp array
     result = seekandreadfile(fp_ID3FilePtr, FIXED_APIC_FIELDS_LENGTH,
                              offset, temp);
     if(result)
     {
       //DBG(HIGH,"ID3v2::parse_ID3v2_APIC_frame: seekandreadfile failed");
       return result;
     }

     pict_tag->text_enc = temp[buf_offset++];

     if(!memcmp(temp + buf_offset, "image/jpeg", strlen("image/jpeg")) )
     {
       pict_tag->img_format = IMG_JPG;
       memcpy(pict_tag->img_format_string, temp + buf_offset,
              strlen("image/jpeg") + 1);
       buf_offset += (uint32)strlen("image/jpeg") + 1;
     }
     else if(!std_memcmp(temp + buf_offset, "image/jpg", strlen("image/jpg")) )
     {
       pict_tag->img_format = IMG_JPG;
       memcpy(pict_tag->img_format_string, temp + buf_offset,
         strlen("image/jpg") + 1);
       buf_offset += strlen("image/jpg") + 1;
     }
     else if(!memcmp(temp + buf_offset, "image/png", strlen("image/png")) )
     {
       pict_tag->img_format = IMG_PNG;
       memcpy(pict_tag->img_format_string, temp + buf_offset,
              strlen("image/png") + 1);
       buf_offset += (uint32)strlen("image/png") + 1;
     }
     else if(!memcmp(temp + buf_offset, "image/gif", strlen("image/gif")) )
     {
       pict_tag->img_format = IMG_GIF;
       memcpy(pict_tag->img_format_string, temp + buf_offset,
         strlen("image/png") + 1);
       buf_offset += (uint32)strlen("image/gif") + 1;
     }
     pict_tag->pict_type = (FS_PICTURE_TYPE)temp[buf_offset++];

     //! Calculate Description string length
     uint32 ulDescStrLength = 0;
     ulDescStrLength = CalcStringLength(temp + buf_offset, pict_tag->text_enc,
                                        FIXED_APIC_FIELDS_LENGTH - buf_offset);

     //! Copy Description string data in structure
     //! MAX_DESC_LEN is the Description string size
     memcpy(pict_tag->desc, temp + buf_offset,
            FILESOURCE_MIN (ulDescStrLength, MAX_DESC_LEN));
     buf_offset += ulDescStrLength;

     pict_tag->pic_data_len = uint32(frame_length - buf_offset);
     pict_tag->pic_data = (uint8*)MM_Malloc(pict_tag->pic_data_len);
     if(NULL == pict_tag->pic_data)
     {
       return PARSER_ErrorMemAllocFail;
     }
     // Read the main picture data bytes into output array
     result = seekandreadfile(fp_ID3FilePtr, pict_tag->pic_data_len,
                              offset+buf_offset, pict_tag->pic_data);
     if(result)
     {
       //DBG(HIGH,"ID3v2::parse_ID3v2_APIC_frame: seekandreadfile failed");
       return result;
     }
     /* Free the memory allocated to temp buffer */
     MM_Delete_Array(temp);
   }
   return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_text_frame
//
// DESCRIPTION
//   This method handles the parsing of a text frame. The text data in the tag
//   is extracted and stored in the text frame structure.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  ptextframe : text frame location where this frame should be
//                     parsed and stored
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  Memory for this is allocated here and has to be freed by the
//  caller of the ID3 methods.
//=============================================================
//
PARSER_ERRORTYPE ID3v2::parse_ID3v2_text_frame(OSCL_FILE* fp_ID3FilePtr,
                                               text_frame *ptextframe,
                                               uint64 frame_length)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   ptextframe->text = NULL;
   // Since 1 byte is used to describe the encoding, the actual text len is
   //1 less than the frame's length.
   int textlen = (int)frame_length-1;
   if(textlen)
   {
     //As per ID3 specification, each text info should be NULL terminated,
     //however, there are few clips which are not following it.
     //Put explicit '\0' all the time.
     ptextframe->text = MM_New_Array(char,(textlen+2));
     if(NULL == ptextframe->text)
     {
       return PARSER_ErrorMemAllocFail;
     }
     uint64 offset;
     offset = m_filereadoffset + GET_ID3V2_FRAME_HDR_SIZE(m_umajorversion);
     // Set encoding type
     result = seekandreadfile(fp_ID3FilePtr,1, offset,&ptextframe->encoding);
     if(result)
     {
       //DBG(HIGH,"ID3v2::parse_ID3v2_text_frame: seekandreadfile failed");
       return result;
     }
     offset++;

     memset(ptextframe->text, 0, textlen + 2);
     // Read the content into this allocated memory
     result = seekandreadfile(fp_ID3FilePtr,textlen, offset,
                              (byte*)ptextframe->text);
     if(result)
     {
       //DBG(HIGH,"ID3v2::parse_ID3v2_text_frame: seekandreadfile failed");
       return result;
     }
     ptextframe->text[textlen] = '\0';
     ptextframe->textLen = textlen+2;
   }
   return result;
}
//=============================================================
// FUNCTION : parse_ID3v2_priv_frame
//
// DESCRIPTION
//   This method handles the parsing of a private tag. This tag is
//   used to store first frame's timestamp in case of live streaming.
//   This is extracted and stored in the private frame structure.
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  ptextframe : text frame location where this frame should be
//                     parsed and stored
//  nLength : total length of the input file in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  Memory for this is allocated here and has to be freed by the
//  caller of the ID3 methods.
//=============================================================
//
PARSER_ERRORTYPE ID3v2::parse_ID3v2_priv_frame(OSCL_FILE* fp_ID3FilePtr,
                                               private_tag_info *priv_tag,
                                               uint64 frame_length)
{
   uint8 *temp = 0;
   /* This is the string available as part of Owner identifier */
   uint8 string[] = "com.apple.streaming.transportStreamTimestamp";
   uint32 strLen = (uint32)strlen((const char *)string);
   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   if(frame_length )
   {
     temp = MM_New_Array(uint8,uint32(frame_length));
     if(NULL == temp)
     {
       return PARSER_ErrorMemAllocFail;
     }
     uint64 offset;
     offset = m_filereadoffset + GET_ID3V2_FRAME_HDR_SIZE(m_umajorversion);
     // Read whole frame data into text array
     result = seekandreadfile(fp_ID3FilePtr,uint32(frame_length),
                              offset,temp);
     if(result)
     {
       //DBG(HIGH,"ID3v2::parse_ID3v2_text_frame: seekandreadfile failed");
       return result;
     }

     priv_tag->timeStamp = 0;
     priv_tag->isTsValid = false;
     /* Check whether this is matching with apple standard private tag or not*/
     if(!memcmp(string, temp, strLen))
     {
        priv_tag->isTsValid = true;
        uint32 TSFieldLen = 0;

        /* There is one NULL character after string before start of timestamp*/
        strLen++;

        /* Convert big-endian eight-octet number number into little endian format*/
        while(TSFieldLen < 8 && strLen < frame_length)
        {
          /* Right shift the timestamp field by 8 bits before copying the data*/
          priv_tag->timeStamp <<= 8;
          priv_tag->timeStamp|= temp[strLen++];
          TSFieldLen++;
        }

        /* Timestamp is stored as 33-bit MPEG-2 Program Elementary Stream.
           MPEG-2 stream timestamps are stored as units of 90khz. So, we need
           to divide with 90 to get accurate timestamp. */
        priv_tag->timeStamp = priv_tag->timeStamp / 90;
     }
     /* Free the memory allocated to temp buffer */
     if(temp)
       MM_Delete_Array(temp);
   }
   return result;
}
//=============================================================
// FUNCTION : get_ID3v2_size
//
// DESCRIPTION
//   Called to get the complete size of the ID3v2 tag
//   the caller can call this API even before calling "parse_ID3v2_tag"
//   Calling this API after calling "parse_ID3v2_tag" is more efficient though.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  bID3v2postend : flag to show if ID3v2 tag is appended after
//                           the end of audio data in the file
//  nLength : total length of the input file in bytes
//  size : o/p param: size of the complete ID3v2 tag in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::get_ID3v2_size(OSCL_FILE* fp_ID3FilePtr,uint64 nLength,
                                       uint64 nOffset, bool bID3v2postend,
                                       uint64 *size)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   if((NULL == fp_ID3FilePtr)||(NULL == size))
   {
     return PARSER_ErrorInvalidParam;
   }
   // check to see if we already have a valid size available
   if(m_size != 0)
   {
     // the tag has already been parsed
     // so, just return the size we have counted
     *size = m_size;
     return PARSER_ErrorNone;
   }
   // the tag has not been parsed yet.
   // get the tage info and parse the header
   // and then return the size value calculated
   if(bID3v2postend)
   {
     m_filereadoffset = nLength -10;
   }
   metadata_id3v2_type stid3v2;
   stid3v2.size =0;
   result = parse_ID3v2_tag_header(fp_ID3FilePtr,nOffset, &stid3v2,nLength);
   *size = (uint64)stid3v2.size;
   return result;
}
//=============================================================
// FUNCTION : getsyncsafeinteger
//
// DESCRIPTION
//   Given an input byte stream of sync safe data, this method returns
//   a normal int. A max of 4 bytes can be handled at a time by this method.
//
// PARAMETERS
//   pInput : input data
//   nbytes : number of bytes in the input data
//
// RETURN VALUE
//   uint32 (integer)
//
// SIDE EFFECTS
//   None
//=============================================================
uint32 ID3v2::getsyncsafeinteger(const byte *pInput, int nbytes) const
{
  uint32 uoutputint = 0;
  int n;
  for (n=0; n < nbytes; n++)
  {
    uoutputint = (uoutputint << 7) | (*(pInput)++ & 0x7F);
  }
  return uoutputint;
}
//=============================================================
// FUNCTION : get_frame_id_type
//
// DESCRIPTION
//  Returns the ID3v2_frame_type enum given the 4 bytes frame ID if this
//  is a known frame type according to the list in enum ID3v2_frame_type
//
//PARAMETERS
//  frame_id: 4 byte frame ID
//
//RETURN VALUE
//  ID3v2_frame_type
//
//SIDE EFFECTS
//  None
//=============================================================
ID3V2_FOUR_CHARS_FRAME_TYPES ID3v2::get_frame_id_type(const byte* frame_id) const
{
  int i;
  for(i=0; i< (int)ID3V2_FOUR_CHARS_TAG_MAX;i++)
  {
    if(!memcmp(frame_identifiers[i],frame_id,ID3V2_3_4_FRAME_ID_SIZE))
    {
      return (ID3V2_FOUR_CHARS_FRAME_TYPES)i;
    }
  }
  return ID3V2_FOUR_CHARS_TAG_UNKN;
}
//=============================================================
// FUNCTION : get_frame_id_type
//
// DESCRIPTION
//  Returns the ID3v2_frame_type enum given the 4 bytes frame ID if this
//  is a known frame type according to the list in enum ID3v2_frame_type
//
//PARAMETERS
//  frame_id: 4 byte frame ID
//
//RETURN VALUE
//  ID3v2_frame_type
//
//SIDE EFFECTS
//  None
//=============================================================
ID3V2_THREE_CHARS_FRAME_TYPES ID3v2::get_id3v2_2_below_frame_id_type
                                                  (const byte* frame_id) const
{
  for(int i=0; i< (int)ID3V2_THREE_CHARS_TAG_MAX;i++)
  {
    if(!memcmp(id3v2_2_below_frame_identifiers[i],frame_id,
               ID3V2_2_BELOW_FRAME_ID_SIZE))
    {
      return (ID3V2_THREE_CHARS_FRAME_TYPES)i;
    }
  }
  return ID3V2_THREE_CHARS_TAG_UNK;
}
//=============================================================
// FUNCTION : get_next_frame
//
// DESCRIPTION
//   reads the next frame header and returns the frame type and
//   frame length for the frame.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  pframe_type : output param to be filled by this method
//                        gives the type of this frame
//  pframe_length : length of the frame in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::get_next_frame(OSCL_FILE* fp_ID3FilePtr,
                                     ID3V2_FOUR_CHARS_FRAME_TYPES *pframe_type,
                                     uint64 *pframe_length) const
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   uint64 offset = m_filereadoffset;
   byte frame_id[ID3V2_3_4_FRAME_ID_SIZE] = {0};
   byte frame_length_buffer[ID3V2_3_4_FRAME_LENGTH_SIZE] = {0};
   *pframe_type = ID3V2_FOUR_CHARS_TAG_UNKN;
   result = seekandreadfile(fp_ID3FilePtr,ID3V2_3_4_FRAME_ID_SIZE,
                            offset,frame_id);
   if(result)
   {
     //DBG(HIGH,"ID3v2::get_next_frame: seekandreadfile failed");
     return result;
   }
   offset+= ID3V2_3_4_FRAME_ID_SIZE;
   result = seekandreadfile(fp_ID3FilePtr, ID3V2_3_4_FRAME_ID_SIZE,
                            offset, frame_length_buffer);
   if(result)
   {
     //DBG(HIGH,"ID3v2::get_next_frame: seekandreadfile failed");
     return result;
   }
   offset += ID3V2_3_4_FRAME_LENGTH_SIZE;
   *pframe_type = get_frame_id_type(frame_id);
   // Check if the frame length is valid
   *pframe_length = parse_ID3v2_uint(frame_length_buffer,
                                     ID3V2_3_4_FRAME_LENGTH_SIZE);
   return result;
}
//=============================================================
// FUNCTION : get_next_frame
//
// DESCRIPTION
//   reads the next frame header and returns the frame type and
//   frame length for the frame.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//  pframe_type : output param to be filled by this method
//                        gives the type of this frame
//  pframe_length : length of the frame in bytes
//
//RETURN VALUE
//  AEE_SUCCESS : Everything was done successfully
//  AEE_EFAILED : Could not execute this method successfully
//
//SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE ID3v2::get_next_frame(OSCL_FILE* fp_ID3FilePtr,
                                   ID3V2_THREE_CHARS_FRAME_TYPES *pframe_type,
                                   uint64 *pframe_length) const
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   uint64 offset = m_filereadoffset;
   byte frame_id[ID3V2_2_BELOW_FRAME_ID_SIZE] = {0};
   byte frame_length_buffer[ID3V2_2_BELOW_FRAME_LENGTH_SIZE] = {0};
   *pframe_type = ID3V2_THREE_CHARS_TAG_UNK;
   result = seekandreadfile(fp_ID3FilePtr,ID3V2_2_BELOW_FRAME_ID_SIZE,
                            offset,frame_id);
   if(result)
   {
     return result;
   }
   offset+= ID3V2_2_BELOW_FRAME_ID_SIZE;
   result = seekandreadfile(fp_ID3FilePtr,ID3V2_2_BELOW_FRAME_LENGTH_SIZE,
                            offset,frame_length_buffer);
   if(result)
   {
     return result;
   }
   offset += ID3V2_2_BELOW_FRAME_LENGTH_SIZE;
   *pframe_type = get_id3v2_2_below_frame_id_type(frame_id);
   *pframe_length = parse_ID3v2_uint(frame_length_buffer,
                                     ID3V2_2_BELOW_FRAME_LENGTH_SIZE);
   return result;
}
//=============================================================
// FUNCTION : check_padding
//
// DESCRIPTION
//   This method checks the current read position to see if we have hit padding yet.
//   If we have hit padding, it moves the read pointer to the end of the padding.
//
//PARAMETERS
//  file: IFilePort1 interface pointer to the audio file
//
//RETURN VALUE
//void
//
//SIDE EFFECTS
//  None
//=============================================================
bool ID3v2::check_padding(OSCL_FILE* fp_ID3FilePtr)
{
  uint8 i;
  uint32 result = PARSER_ErrorNone;
  bool bfoundpadding = FALSE;
  // Check if 4 bytes from current position are 0
  // If yes, then we have found padding.
  byte buffer[ID3V2_3_4_FRAME_ID_SIZE] = {0};
  byte null_buffer[ID3V2_3_4_FRAME_ID_SIZE] = {0};
  result = seekandreadfile(fp_ID3FilePtr, ID3V2_3_4_FRAME_ID_SIZE,
                           m_filereadoffset, buffer);
  if(result)
  {
    //DBG(HIGH,"ID3v2::check_padding: seekandreadfile failed");
    return FALSE;
  }
  if(!memcmp(buffer, null_buffer, ID3V2_3_4_FRAME_ID_SIZE))
  {
    // found padding
    bfoundpadding = TRUE;
    m_filereadoffset += ID3V2_3_4_FRAME_ID_SIZE;
  }
  else
  {
    return FALSE;
  }
  // Continue checking to find the end of padding
  while(bfoundpadding)
  {
    result = seekandreadfile(fp_ID3FilePtr ,ID3V2_3_4_FRAME_ID_SIZE,
                             m_filereadoffset,buffer);
    /* If data read is failed because of some problem, then return False. */
    if(result)
    {
      //DBG(HIGH,"ID3v2::check_padding: seekandreadfile failed");
      return FALSE;
    }
    if(!memcmp(buffer, null_buffer, ID3V2_3_4_FRAME_ID_SIZE))
    {
      // found padding
      m_filereadoffset += ID3V2_3_4_FRAME_ID_SIZE;
    }
    else
    {
      bfoundpadding = FALSE;
      for(i=0;i<ID3V2_3_4_FRAME_ID_SIZE;i++)
      {
        if(0 != buffer[i]) { break;}
        m_filereadoffset++;
      }
    }
  }
  return TRUE;
}
//=============================================================
// FUNCTION : seekandreadfile
//
// DESCRIPTION
//  Seeks to the given position and then reads the desired number of bytes into the
//  buffer provided by the caller. A check is done to make sure that the bytes read
//  are as many as requested in the read.
//
// PARAMETERS
//  file :  IFilePort1 interface pointer to the file
//  length : the no. of bytes required to be read
//  position : position to start reading in the file
//  pbuffer : pointer to the start of buffer for the data to be read into
//
// RETURN VALUE
//  AEE_SUCCESS : All worked well
//  AEE_EFAILED : Could not perform the operation as required
//
// SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE seekandreadfile (OSCL_FILE* fp_ID3FilePtr, uint32 length,
                                  uint64 position, uint8 *pbuffer)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   uint32 bytes_read=0;
   MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "seek position %llu",position);
   if(PARSER_ErrorNone != OSCL_FileSeek(fp_ID3FilePtr, position, SEEK_SET))
   {
     //DBG(HIGH,"seekandreadfile: file seek failed");
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "seek position fail");
     result = PARSER_ErrorReadFail;
     return result;
   }
   // Verify enough data is read to parse the header
   bytes_read = OSCL_FileRead(pbuffer,length,1,fp_ID3FilePtr);
   // Verify enough data is available to contain the header
   if (bytes_read != length)
   {
     result = PARSER_ErrorReadFail;
     return result;
   }
   return result;
}

/*=============================================================
 FUNCTION : parse_ID3v2_comment_frame

 DESCRIPTION
   This method handles the parsing of comment section in ID3 tags

PARAMETERS
  fp_ID3FilePtr : IFilePort1 interface pointer to the file
  pEncTag       : encoder tag pointer
  ullFrameLen   : Comment frame size

RETURN VALUE
  PARSER_ErrorNone if parsing is done successfully,
  else corresponding error will be reproted

SIDE EFFECTS
  None
=============================================================*/
PARSER_ERRORTYPE ID3v2::parse_ID3v2_comment_frame(OSCL_FILE* fp_ID3FilePtr,
                                              encoder_delay_tag_info* pEncTag,
                                              uint64 ullFrameLen)
{
  uint32 ulBufOffset = 0;
  uint64 ullOffset   = 0;
  uint8 *pucTempBuf  = NULL;
  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  if(ullFrameLen)
  {
    pucTempBuf = MM_New_Array(uint8,uint32(ullFrameLen));
    if(NULL == pucTempBuf)
    {
      return PARSER_ErrorMemAllocFail;
    }
    ullOffset = m_filereadoffset + GET_ID3V2_FRAME_HDR_SIZE(m_umajorversion);
    // Read complete frame data into temp array
    result = seekandreadfile(fp_ID3FilePtr, (uint32)ullFrameLen,
                             ullOffset, pucTempBuf);
    if(result)
    {
      //DBG(HIGH,"ID3v2::parse_ID3v2_comment_frame: seekandreadfile failed");
      return result;
    }
    //First 4bytes are used to indicate data type and language code
    ulBufOffset = 4;

    if(!memcmp(pucTempBuf + ulBufOffset, "iTunSMPB", strlen("iTunSMPB")) )
    {
      /* Metadata will be stored in following format:
         iTunSMPB<0><space><8bytes ZERO Padding><space><8bytes Encoder delay>
         <space><8Bytes Padding delay><space>
      */
      //Metadata string length with NULL character
      ulBufOffset += (uint32)strlen("iTunSMPB") + 1;

      //Every string starts with Space character and each field is of size
      //8bytes. 1st field is ZERO Padding field, Parser does not need this
      ulBufOffset += 9;

      //Skip additional Space character at the start of encoder delay field
      ulBufOffset++;

      memcpy(pEncTag->ucEncoderDelay, pucTempBuf + ulBufOffset,
             sizeof(uint64));
      //Update Buffer offset to start of Padding delay filed.
      //1 Extra byte is required to skip space character at the start
      ulBufOffset += (uint8)sizeof(uint64) + 1;

      memcpy(pEncTag->ucPaddingDelay, pucTempBuf + ulBufOffset,
             sizeof(uint64));
      pEncTag->bDelayPresent = true;
    }
    /* Free the memory allocated to temp buffer */
    MM_Delete_Array(pucTempBuf);
  }
  return result;
}

/*! =========================================================================
@brief  Function to reverse byte for UTF16 string

@detail This is used to provide data in system expected Endian format

@param[in]
  pStr      : Input string which needs to be swapped
  ulStrLen  : String length

@return    None
@note      None.
=============================================================================*/
void ID3v2::ByteSwapString(char* pStr, uint32 ulStrLen)
{
  uint32 ulIndex = 0;
  //! Validate input params
  if ((!pStr) || (0 == ulStrLen))
  {
    return;
  }
  for (;(ulIndex + 1) < ulStrLen; ulIndex += 2)
  {
    char temp         = pStr[ulIndex];
    pStr[ulIndex]     = pStr[ulIndex + 1];
    pStr[ulIndex + 1] = temp;
  }
  return;
}

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
                                        uint32*   pulDatabufLen)
{
  encoder_delay_tag_info* pEncDel = NULL;
  text_frame*          pTextFrame = NULL;
  metadata_id3v2_type* pID3v2Info = (metadata_id3v2_type*)pID3v2;
  PARSER_ERRORTYPE     eRet       = PARSER_ErrorInvalidParam;

  if ((NULL == pID3v2Info) || (NULL == pID3v2Info->pic_info.pic_data))
  {
    return eRet;
  }

  if (pucDataBuf)
  {
    FS_ALBUM_ART_METADATA * pAlbumArt = (FS_ALBUM_ART_METADATA*)pucDataBuf;
    eRet = PARSER_ErrorNone;
    if(*pulDatabufLen < pID3v2Info->pic_info.pic_data_len +
                        FIXED_APIC_FIELDS_LENGTH)
    {
      return PARSER_ErrorInsufficientBufSize;
    }
    memset(pucDataBuf, 0, FIXED_APIC_FIELDS_LENGTH);
    pAlbumArt->ucTextEncodeType = pID3v2Info->pic_info.text_enc;
    pAlbumArt->imgFormat        = pID3v2Info->pic_info.img_format;
    pAlbumArt->picType          = pID3v2Info->pic_info.pict_type;
    pAlbumArt->ulPicDataLen     = pID3v2Info->pic_info.pic_data_len;

    memcpy(pAlbumArt->ucDesc,pID3v2Info->pic_info.desc,MAX_DESC_LEN);
    memcpy(pAlbumArt->pucPicData,pID3v2Info->pic_info.pic_data,
           pID3v2Info->pic_info.pic_data_len);
    memcpy(pAlbumArt->ucImgFormatStr,pID3v2Info->pic_info.img_format_string,
           MAX_IMG_FORMAT_LEN);
  }//! if (pucDataBuf)
  if (pID3v2Info->pic_info.pic_data_len)
  {
    eRet = PARSER_ErrorNone;
    *pulDatabufLen = pID3v2Info->pic_info.pic_data_len +
                     FIXED_APIC_FIELDS_LENGTH;
  }
  else
  {
    *pulDatabufLen = 0;
  }
  return eRet;
}

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
PARSER_ERRORTYPE ParseID3V2MetaData(void *pID3v2,
                                    FileSourceMetaDataType ienumData,
                                    wchar_t*  pucDataBuf,
                                    uint32*   pulDatabufLen,
                                    FS_TEXT_ENCODING_TYPE& eEncodeType)
{
  encoder_delay_tag_info* pEncDelay = NULL;
  text_frame*          pTextFrame   = NULL;
  metadata_id3v2_type* pID3v2Info   = (metadata_id3v2_type*)pID3v2;
  PARSER_ERRORTYPE     eRet         = PARSER_ErrorInvalidParam;

  //! Validate the input parameter
  if (NULL == pID3v2)
  {
    return eRet;
  }

  switch (ienumData)
  {
  case FILE_SOURCE_MD_ALBUM:
    pTextFrame = &pID3v2Info->album;
    break;
  case FILE_SOURCE_MD_ARTIST:
    pTextFrame = &pID3v2Info->album_artist;
    break;
  case FILE_SOURCE_MD_ALBUM_ARTIST:
    pTextFrame = &pID3v2Info->orchestra;
    break;
  case FILE_SOURCE_MD_REC_YEAR:
    pTextFrame = &pID3v2Info->year;
    break;
  case FILE_SOURCE_MD_TITLE:
    pTextFrame = &pID3v2Info->title;
    break;
  case FILE_SOURCE_MD_TRACK_NUM:
    pTextFrame = &pID3v2Info->track;
    break;
  case FILE_SOURCE_MD_GENRE:
    pTextFrame = &pID3v2Info->genre;
    break;
  case FILE_SOURCE_MD_AUTHOR:
    pTextFrame = &pID3v2Info->publisher;
    break;
  case FILE_SOURCE_MD_COMPOSER:
    pTextFrame = &pID3v2Info->composer;
    break;
  case FILE_SOURCE_MD_ENC_DELAY:
  case FILE_SOURCE_MD_PADDING_DELAY:
    pEncDelay  = &pID3v2Info->encoder_delay_tag;
    break;
  default:
    break;
  }//!switch (ienumData)if(pTextFrame && pTextFrame->text)
  if(pTextFrame && pTextFrame->text)
  {
    eRet = PARSER_ErrorNone;
    if (pucDataBuf)
    {
      if(*pulDatabufLen < pTextFrame->textLen * sizeof(wchar_t))
      {
        *pulDatabufLen = pTextFrame->textLen * sizeof(wchar_t);
        eRet = PARSER_ErrorInsufficientBufSize;
      }
      else
      {
        memset(pucDataBuf, 0, *pulDatabufLen);
        memcpy(pucDataBuf, pTextFrame->text, pTextFrame->textLen);
        switch(pTextFrame->encoding)
        {
          case ID3V2_FRAME_TEXT_ENCODING_UTF_8:
            eEncodeType = FS_TEXT_ENC_UTF8;
            break;
          case ID3V2_FRAME_TEXT_ENCODING_ISO_8859_1:
            eEncodeType = FS_TEXT_ENC_ISO8859_1;
            break;
          case ID3V2_FRAME_TEXT_ENCODING_UTF_16:
            eEncodeType = FS_TEXT_ENC_UTF16;
            break;
          case ID3V2_FRAME_TEXT_ENCODING_UTF_16BE:
            eEncodeType = FS_TEXT_ENC_UTF16_BE;
            break;
          default:
            eEncodeType = FS_ENCODING_TYPE_UNKNOWN;
            break;
        }
      } //! switch(pTextFrame->encoding)
    } //! if (pucDataBuf)
    else
      *pulDatabufLen = pTextFrame->textLen * sizeof(wchar_t);
  } //! if(pTextFrame && pTextFrame->text)
  else if (pEncDelay && pEncDelay->bDelayPresent)
  {
    eRet = PARSER_ErrorNone;
    if (pucDataBuf)
    {
      eEncodeType = FS_TEXT_ENC_UTF8;
      if(FILE_SOURCE_MD_ENC_DELAY == ienumData)
      {
        memcpy(pucDataBuf, pEncDelay->ucEncoderDelay, sizeof(uint64));
      }
      else
      {
        memcpy(pucDataBuf, pEncDelay->ucPaddingDelay, sizeof(uint64));
      }
    }
    *pulDatabufLen = (uint32)sizeof(uint64);
  }
  else
  {
    *pulDatabufLen = 0;
  }
  return eRet;
}

/* ============================================================================
  @brief  This function is used to parse given ID3 v1 Metadata.

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
PARSER_ERRORTYPE ParseID3V1MetaData(void *pID3v1,
                                    FileSourceMetaDataType ienumData,
                                    wchar_t*  pucDataBuf,
                                    uint32*   pulDatabufLen)
{
  metadata_id3v1_type* pid3v1info = (metadata_id3v1_type*)pID3v1;
  uint8* pucID3v1Str    = NULL;
  uint32 ulID3v1StrLen  = 0;
  PARSER_ERRORTYPE eRet = PARSER_ErrorDefault;
  char  ucTempString[33];
  memset(ucTempString, 0, 33);

  //! Validate input param
  if ((NULL == pID3v1) || (NULL == pulDatabufLen))
  {
    return eRet;
  }

  switch (ienumData)
  {
  case FILE_SOURCE_MD_ALBUM:
    ulID3v1StrLen = MAX_ID3V1_TEXT_SIZE;
    pucID3v1Str = pid3v1info->album;
    break;
  case FILE_SOURCE_MD_ARTIST:
    ulID3v1StrLen = MAX_ID3V1_TEXT_SIZE;
    pucID3v1Str = pid3v1info->artist;
    break;
  case FILE_SOURCE_MD_TITLE:
    ulID3v1StrLen = MAX_ID3V1_TEXT_SIZE;
    pucID3v1Str = pid3v1info->title;
    break;
  case FILE_SOURCE_MD_REC_YEAR:
    ulID3v1StrLen = MAX_ID3V1_YEAR_TEXT_SIZE;
    pucID3v1Str = pid3v1info->year;
    break;
  case FILE_SOURCE_MD_TRACK_NUM:
    {
#ifdef _ANDROID_
      snprintf(ucTempString, 4, (char*)"%d", (int)pid3v1info->track);
#else
      std_strlprintf(ucTempString, 4, (char*)"%d", (int)pid3v1info->track);
#endif
      ulID3v1StrLen = 4;
      pucID3v1Str = (uint8 *)ucTempString;
    }
    break;
  default:
    break;
  }
  //! All 31 bits need not be used to store metadata
  //! Get actual metadata length
  if (pucID3v1Str)
  {
    ulID3v1StrLen = FILESOURCE_MIN(ulID3v1StrLen, strlen((char*)pucID3v1Str));
  }
  if (pucDataBuf && pucID3v1Str)
  {
    if(*pulDatabufLen < ulID3v1StrLen * (uint32)sizeof(wchar_t))
    {
      return PARSER_ErrorInsufficientBufSize;
    }

    //! Provide data as it is to client, let client figure out encoding type
    memset(pucDataBuf, 0, *pulDatabufLen);
    memcpy(pucDataBuf, (char*)pucID3v1Str, (int)ulID3v1StrLen);
  }

  if (ulID3v1StrLen)
  {
    // Update output buf length value
    *pulDatabufLen = (ulID3v1StrLen + 1) * (uint32)sizeof(wchar_t);
    eRet = PARSER_ErrorNone;
  }

  return eRet;
}

/* ============================================================================
  @brief  This function is used to free the given text entry.

  @details This function is used to free the memory allocated to text frame.

  @param[in]     pTextFrame             text frame pointer.

  @return     None.
  @note       None.
============================================================================ */
void FreeTextEntry(text_frame *pTextFrame)
{
  if (pTextFrame->text)
  {
    MM_Delete_Array(pTextFrame->text);
    pTextFrame->text = NULL;
  }
  return;
}

/* ============================================================================
  @brief  This function is used to free the ID3 v2 Metadata structure.

  @details This function is used to free the memory allocated in ID3 v2
           metadata structure.

  @param[in]     pID3v2Info             ID3 Metadata object.

  @return     None.
  @note       None.
============================================================================ */
void FreeID3V2MetaDataMemory(metadata_id3v2_type* pID3V2Ptr)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "free_ID3v2_metadata_memory");
  int i = 0;

  // MM_Delete all the generic content in generic frame
  if (pID3V2Ptr)
  {
    for(i=0;pID3V2Ptr->generic_frames && i < pID3V2Ptr->generic_framesLen;i++)
    {
      if(pID3V2Ptr->generic_frames[i].content)
      {
        MM_Delete_Array( pID3V2Ptr->generic_frames[i].content);
      }
      pID3V2Ptr->generic_frames[i].content = NULL;
    }

    // MM_Delete all generic frame structures
    if(pID3V2Ptr->generic_frames)
    {
      MM_Free(pID3V2Ptr->generic_frames);
      pID3V2Ptr->generic_frames = NULL;
    }
    // MM_Delete all the text content of text frames
    FreeTextEntry(&pID3V2Ptr->involved_people_list);
    FreeTextEntry(&pID3V2Ptr->album);
    FreeTextEntry(&pID3V2Ptr->beats_per_minute);
    FreeTextEntry(&pID3V2Ptr->composer);
    FreeTextEntry(&pID3V2Ptr->genre);
    FreeTextEntry(&pID3V2Ptr->copyright);
    FreeTextEntry(&pID3V2Ptr->date);
    FreeTextEntry(&pID3V2Ptr->playlist_delay);
    FreeTextEntry(&pID3V2Ptr->encoder);
    FreeTextEntry(&pID3V2Ptr->lyricist);
    FreeTextEntry(&pID3V2Ptr->file_type);
    FreeTextEntry(&pID3V2Ptr->time);
    FreeTextEntry(&pID3V2Ptr->content_group_desc);
    FreeTextEntry(&pID3V2Ptr->title);
    FreeTextEntry(&pID3V2Ptr->subtitle);
    FreeTextEntry(&pID3V2Ptr->init_key);
    FreeTextEntry(&pID3V2Ptr->languages);
    FreeTextEntry(&pID3V2Ptr->length);
    FreeTextEntry(&pID3V2Ptr->media_type);
    FreeTextEntry(&pID3V2Ptr->original_title);
    FreeTextEntry(&pID3V2Ptr->original_filename);
    FreeTextEntry(&pID3V2Ptr->original_lyricists);
    FreeTextEntry(&pID3V2Ptr->original_artist);
    FreeTextEntry(&pID3V2Ptr->original_release_year);
    FreeTextEntry(&pID3V2Ptr->file_own_licensee);
    FreeTextEntry(&pID3V2Ptr->album_artist);
    FreeTextEntry(&pID3V2Ptr->orchestra);
    FreeTextEntry(&pID3V2Ptr->conductor);
    FreeTextEntry(&pID3V2Ptr->remix_mod_by);
    FreeTextEntry(&pID3V2Ptr->part_of_set);
    FreeTextEntry(&pID3V2Ptr->publisher);
    FreeTextEntry(&pID3V2Ptr->track);
    FreeTextEntry(&pID3V2Ptr->recording_dates);
    FreeTextEntry(&pID3V2Ptr->internet_radio_stn_name);
    FreeTextEntry(&pID3V2Ptr->internet_radio_stn_owner);
    FreeTextEntry(&pID3V2Ptr->internet_radio_stn_homepage);
    FreeTextEntry(&pID3V2Ptr->size_in_bytes);
    FreeTextEntry(&pID3V2Ptr->isrc_code);
    FreeTextEntry(&pID3V2Ptr->sw_hw_enc_settings);
    FreeTextEntry(&pID3V2Ptr->year);
    FreeTextEntry(&pID3V2Ptr->recomm_buff_size);
    FreeTextEntry(&pID3V2Ptr->user_def_text_info);
    if(pID3V2Ptr->pic_info.pic_data)
    {
      MM_Free(pID3V2Ptr->pic_info.pic_data);
      pID3V2Ptr->pic_info.pic_data = NULL;
    }
  }//if (m_pid3v2)
}

