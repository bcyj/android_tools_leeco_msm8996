/* =======================================================================
                              atomutils.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.  Include any initialization and synchronizing
  requirements.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Portions copyrighted by PacketVideo Corporation;
Copyright 1998, 2002, 2003 PacketVideo Corporation, All Rights Reserved;
and Portions copyrighted by Qualcomm Technologies, Inc.;
Copyright 2003-2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/atomutils.cpp#13 $
$DateTime: 2012/01/06 01:37:56 $
$Change: 2128420 $


========================================================================== */
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
//DEBUGGING
#include <stdio.h>
#include <stdlib.h>


/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "atomutils.h"
#include "atomdefs.h"
#include "utf8conv.h"

/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
bool
AtomUtils::READ_OLD_DESCRIPTOR = false;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */

/* ======================================================================
FUNCTION
  AtomUtils::read64

DESCRIPTION
  Read in the 64 bits byte by byte and take most significant byte first

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read64(OSCL_FILE *fp, uint64 &data)
{
  const int32 N=8;
  uint8 bytes[N];
  data=0;

  int32 retVal = (int32)(OSCL_FileRead((void*)bytes,1,N,fp));

  if ( retVal < N )
    return false;


  for ( int32 i=0;i<N;i++ )
    data = (data<<8) | bytes[i];

  return true;

}

/* ======================================================================
FUNCTION
  AtomUtils::read32

DESCRIPTION
  Read in the 32 bits byte by byte and take most significant byte first

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read32(OSCL_FILE *fp, uint32 &data)
{
  const int32 N=4;
  uint8 bytes[N];
  data=0;

  int32 retVal = (int32)(OSCL_FileRead((void*)bytes,1,N,fp));

  if ( retVal < N )
    return false;

  for ( int32 i=0;i<N;i++ )
    data = (data<<8) | bytes[i];

  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::read32read32

DESCRIPTION
  Read in the 32 bits byte by byte and take most significant byte first.
  This is equivalent to two read32 calls.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read32read32(OSCL_FILE *fp, uint32 &data1, uint32 &data2)
{
  const int32 N=8;
  uint8 bytes[N];
  data1=0;
  data2=0;

  int32 retVal = (int32)(OSCL_FileRead((void*)bytes,1,N,fp));

  if ( retVal < N )
    return false;

  int32 i;
  for ( i=0;i<4;i++ )
    data1 = (data1<<8) | bytes[i];

  for ( i=4;i<8;i++ )
    data2 = (data2<<8) | bytes[i];

  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::read24

DESCRIPTION
  Read in the 24 bits byte by byte and take most significant byte first

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read24(OSCL_FILE *fp, uint32 &data)
{
  const int32 N=3;
  uint8 bytes[N];
  data=0;

  int32 retVal = (int32)(OSCL_FileRead((void*)bytes,1,N,fp));

  if ( retVal < N )
    return false;

  for ( int32 i=0;i<N;i++ )
    data = (data<<8) | bytes[i];

  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::read16

DESCRIPTION
  Read in the 16 bits byte by byte and take most significant byte first

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read16(OSCL_FILE *fp, uint16 &data)
{
  const int32 N=2;
  uint8 bytes[N];
  data=0;

  int32 retVal = (int32)(OSCL_FileRead((void*)bytes,1,N,fp));

  if ( retVal < N )
    return false;

  for ( int32 i=0;i<N;i++ )
    data =(uint16) ( (data<<8) |(uint16) bytes[i]);

  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::read16read16

DESCRIPTION
  Read in the 16 bits byte by byte and take most significant byte first
  This is equivalent to two read16 calls

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read16read16(OSCL_FILE *fp, uint16 &data1, uint16 &data2)
{
  const int32 N=4;
  uint8 bytes[N];
  data1=0;
  data2=0;

  int32 retVal = (int32)(OSCL_FileRead((void*)bytes,1,N,fp));

  if ( retVal < N )
    return false;

  int32 i;
  for ( i=0;i<2;i++ )
    data1 = (uint16)((data1<<8) | (uint16) bytes[i]);

  for ( i=2;i<4;i++ )
    data2 = (uint16)((data2<<8) | (uint16) bytes[i]);

  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::read8

DESCRIPTION
  Read in the 8 bit byte

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read8(OSCL_FILE *fp, uint8 &data)
{
  data = 0;

  int32 retVal = (int32)(OSCL_FileRead((void*)&data,1,1,fp));

  if ( retVal < 1 )
    return false;

  /*
    int8 byte;
    is.get(byte);
    data = (uint8)byte;
    */
  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::read8read8

DESCRIPTION
  Read in the 8 bit byte
  This is equivalent to two read8 calls

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read8read8(OSCL_FILE *fp, uint8 &data1, uint8 &data2)
{
  const int32 N=2;
  uint8 bytes[N];
  data1=0;
  data2=0;

  int32 retVal = (int32)(OSCL_FileRead((void*)bytes,1,N,fp));

  if ( retVal < N )
    return false;

  data1 = bytes[0];
  data2 = bytes[1];

  return true;
}
/* ======================================================================
FUNCTION
  AtomUtils::readNullTerminatedString

DESCRIPTION
  Read in a NULL terminated string byte by byte and take most significant byte first
  and convert to a ZString (UNICODE)

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::readNullTerminatedString(OSCL_FILE *fp, FILESOURCE_STRING &data)
{
  uint8 buf[256];
  int32 index = 0;

  if ( !AtomUtils::read8(fp, buf[index]) )
    return false;

  bool nextChar = (buf[index] == 0) ? false : true;

  while ( nextChar && (index < 255) )
  {
    index++;

    if ( !AtomUtils::read8(fp, buf[index]) )
      return false;

    nextChar = (buf[index] == 0) ? false : true;
  }
  // String buffer filled - now create ZString
  // ZString is a wchar_t string so no conversion needed
  wchar_t outbuf[256*sizeof(wchar_t)];
  UTF8ToUnicode((int8 *)buf, index, outbuf, 256);
  FILESOURCE_STRING temp(outbuf);
  data = temp;
  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::readNullTerminatedUnicodeString

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::readNullTerminatedUnicodeString(OSCL_FILE *fp, FILESOURCE_STRING &data)
{

   int32 index = 0;
   wchar_t buf[256];

  /*
   * Offset is used in case in some platforms if the size of wchar is greater
   * than 2, then we need to check the lower few bytes to see if the null character
   * has been reached or not.
   */


  // Need to be careful of the byte-ordering when creating the wchar_t array
  uint8 firstbyte;
  uint8 secondbyte;

  if ( !AtomUtils::read8read8(fp, firstbyte, secondbyte) )
    return false;

  //buf[index++] = secondbyte;
  //buf[index++] = firstbyte;

  // Allow the OS to do the bit shifting to get the correct byte ordering
  // for the CHAR value

  wchar_t *wptr = &buf[index];

  *wptr = (uint16) (firstbyte << 8 | (uint16) secondbyte);


  bool nextChar = (buf[index] == 0) ? false : true;
  index += 1;


  while ( nextChar && (index < (int32)(256)) )
  {
    if ( !AtomUtils::read8read8(fp, firstbyte, secondbyte) )
      return false;

    //buf[index++] = secondbyte;
    //buf[index++] = firstbyte;

    // Allow the OS to do the bit shifting to get the correct byte ordering
    // for the CHAR value

    wptr  = &buf[index];
    *wptr = (uint16) (firstbyte << 8 | (uint16) secondbyte);
    nextChar = (buf[index] == 0) ? false : true;
    index++;
  }

  // String (wchar_t) buffer filled - now create FILESOURCE_STRING

  // FILESOURCE_STRING is a wchar_t string so no conversion needed
  FILESOURCE_STRING temp((const OSCL_TCHAR *)buf);
  data = temp;
  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::readNullTerminatedAsciiString

DESCRIPTION
  Read in a NULL terminated ascii (8-bit char) string byte by byte and take most
  significant byte first and convert to a ZString
  (wchar_t if UNICODE is defined - CHAR if not)

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::readNullTerminatedAsciiString(OSCL_FILE *fp, FILESOURCE_STRING &data)
{
  return readNullTerminatedString(fp, data);
}

/* ======================================================================
FUNCTION
  AtomUtils::readByteData

DESCRIPTION
  Read in byte data and take most significant byte first

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::readByteData(OSCL_FILE *fp, uint32 length, uint8 *data)
{
  uint32 bytesRead;
  bytesRead=OSCL_FileRead(data,1,length,fp);

  if ( bytesRead < (uint32)length ) // read byte data failed
    return false;
  /*
    for(int32 i=0; i<length; i++) {
        AtomUtils::read8(fp, data[i]);
    }
  */
  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::getNumberOfBytesUsedToStoreSizeOfClass

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32
AtomUtils::getNumberOfBytesUsedToStoreSizeOfClass(uint32 contentSize)
{
  // The actual _sizeOfClass value includes the size of the class's contents PLUS
  // the number of bytes needed to store the _sizeOfClass field. The parameter
  // contentSize represents the number of bytes needed to store ONLY the members
  // of the class NOT including the _sizeOfClass field.
  if ( contentSize <= 0x7e ) return 1; // _sizeOfClass field can be rendered in 1 byte (7 LS bits)
  else if ( contentSize <= 0x3ffd ) return 2; // _sizeOfClass field can be rendered in 2 bytes (7 LS bits each)
  else if ( contentSize <= 0x1ffffc ) return 3; // _sizeOfClass field can be rendered in 3 bytes (7 LS bits each)
  else if ( contentSize <= 0xfffffffb ) return 4; // _sizeOfClass field can be rendered in 4 bytes (7 LS bits each)
  else return 0; // ERROR condition
}


/* ======================================================================
FUNCTION
  AtomUtils::getNextAtomType

DESCRIPTION
  Returns the atom type from parsing the input stream

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32
AtomUtils::getNextAtomType(OSCL_FILE *fp)
{
  uint64 filePointer = OSCL_FileTell(fp);

  if( ( filePointer >= AtomUtils::fileSize )||  (filePointer < 8) )
  {
    return 0;
  }

  uint32 size;
  uint32 type;
  if ( !AtomUtils::read32read32(fp, size, type) )
  {
    return UNKNOWN_ATOM;
  }

  // Rewinding the stream back to atom start
  (void)OSCL_FileSeek(fp,(filePointer-8),SEEK_CUR);

  if ( type == MOVIE_ATOM ||
       type == MOVIE_HEADER_ATOM ||
       type == TRACK_ATOM ||
       type == TRACK_HEADER_ATOM ||
       type == TRACK_REFERENCE_ATOM ||
       type == MEDIA_ATOM ||
       type == EDIT_ATOM ||
       type == EDIT_LIST_ATOM ||
       type == MEDIA_HEADER_ATOM ||
       type == HANDLER_ATOM ||
       type == MEDIA_INFORMATION_ATOM ||
       type == VIDEO_MEDIA_HEADER_ATOM ||
       type == SOUND_MEDIA_HEADER_ATOM ||
       type == HINT_MEDIA_HEADER_ATOM ||
       type == MPEG4_MEDIA_HEADER_ATOM ||
       type == NULL_MEDIA_HEADER_ATOM ||
       type == DATA_INFORMATION_ATOM ||
       type == DATA_REFERENCE_ATOM ||
       type == DATA_ENTRY_URL_ATOM ||
       type == DATA_ENTRY_URN_ATOM ||
       type == SAMPLE_TABLE_ATOM ||
       type == TIME_TO_SAMPLE_ATOM ||
       type == COMPOSITION_OFFSET_ATOM ||
       type == SAMPLE_DESCRIPTION_ATOM ||
       type == ESD_ATOM ||
       type == SAMPLE_SIZE_ATOM ||
       type == SAMPLE_TO_CHUNK_ATOM ||
       type == CHUNK_OFFSET_ATOM ||
       type == SYNC_SAMPLE_ATOM ||
       type == SHADOW_SYNC_SAMPLE_ATOM ||
       type == DEGRADATION_PRIORITY_ATOM ||
       type == OBJECT_DESCRIPTOR_ATOM ||
       type == MEDIA_DATA_ATOM ||
       type == FREE_SPACE_ATOM ||
       type == SKIP_ATOM ||
       type == USER_DATA_ATOM ||
       type == WMF_SET_MEDIA ||
       type == CONTENT_VERSION_ATOM ||
       type == VIDEO_INFO_ATOM ||
       type == RANDOM_ACCESS_ATOM ||
       type == WMF_SET_SESSION ||
       type == FILE_TYPE_ATOM||
       type == PVUSER_DATA_ATOM ||
       type == HINT_INFORMATION ||
       type == TRACK_INFO_ATOM||
       type == SYNC_INFO_ATOM ||
       type == REQUIREMENTS_ATOM||
       type == PVUSER_DATA_ATOM||
       type == DOWNLOAD_ATOM ||
       type == TRACK_INFO_SESSION_ATOM ||
       type == TRACK_INFO_MEDIA_ATOM ||
       type == AMR_SAMPLE_ENTRY_ATOM ||
       type == H263_SAMPLE_ENTRY_ATOM ||
       type == AUDIO_SAMPLE_ENTRY ||
       type == VIDEO_SAMPLE_ENTRY ||
       type == MPEG_SAMPLE_ENTRY ||
       type == UUID_ATOM ||
       type == AMR_SPECIFIC_ATOM ||
       type == H263_SPECIFIC_ATOM ||
       type == COPYRIGHT_ATOM ||
       type == TEXT_SAMPLE_ENTRY ||
       type == HINT_TRACK_REFERENCE_TYPE ||
       type == DPND_TRACK_REFERENCE_TYPE ||
       type == IPIR_TRACK_REFERENCE_TYPE ||
       type == MPOD_TRACK_REFERENCE_TYPE ||
       type == SYNC_TRACK_REFERENCE_TYPE ||
       type == KDDI_GPS_ATOM   ||
       type == KDDI_GPS_EXTENSION_ATOM   ||
       type == FONT_TABLE_ATOM )
  {
    return type;
  }
  else
  {
    return UNKNOWN_ATOM; // ERROR condition
  }
}

/* ======================================================================
FUNCTION
  AtomUtils::getMediaTypeFromHandlerType

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32
AtomUtils::getMediaTypeFromHandlerType(uint32 handlerType)
{
  if (
     handlerType == MEDIA_TYPE_AUDIO ||
     handlerType == MEDIA_TYPE_VISUAL ||
     handlerType == MEDIA_TYPE_HINT ||
     handlerType == MEDIA_TYPE_OBJECT_DESCRIPTOR ||
     handlerType == MEDIA_TYPE_CLOCK_REFERENCE ||
     handlerType == MEDIA_TYPE_SCENE_DESCRIPTION ||
     handlerType == MEDIA_TYPE_MPEG7 ||
     handlerType == MEDIA_TYPE_OBJECT_CONTENT_INFO ||
     handlerType == MEDIA_TYPE_IPMP ||
     handlerType == MEDIA_TYPE_TEXT ||
     handlerType == MEDIA_TYPE_MPEG_J
     )
  {
    return handlerType;
  }
  else
  {
    return MEDIA_TYPE_UNKNOWN;
  }
}

/* ======================================================================
FUNCTION
  AtomUtils::getNumberOfBytesUsedToStoreContent

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32
AtomUtils::getNumberOfBytesUsedToStoreContent(uint32 sizeOfClass)
{
  // The content in a descriptor class is stored immediately after the descriptor tag
  if ( sizeOfClass <= 0x7f ) return sizeOfClass - 2; // _sizeOfClass field is 1 byte (7 LS bits)
  else if ( sizeOfClass <= 0x3fff ) return sizeOfClass - 3; // _sizeOfClass is 2 bytes (7 LS bits each)
  else if ( sizeOfClass <= 0x1fffff ) return sizeOfClass - 4; // _sizeOfClass is 3 bytes (7 LS bits each)
  else if ( sizeOfClass <= 0x0fffffff ) return sizeOfClass - 5; // _sizeOfClass is 4 bytes (7 LS bits each)
  else return 0; // ERROR condition
}

/* ======================================================================
FUNCTION
  AtomUtils::getNextAtomSize

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 AtomUtils::getNextAtomSize(OSCL_FILE *fp)
{
  uint32 size = 0;
  if(fp)
  {
    uint64 filePointer = OSCL_FileTell(fp);
    if(filePointer >= 4)
    {
      (void)AtomUtils::read32(fp, size);
      (void)OSCL_FileSeek(fp,filePointer-4,SEEK_CUR);
    }
  }
  return (int32)size;
}

/* ======================================================================
FUNCTION
  AtomUtils::peekNextNthBytes

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32 AtomUtils::peekNextNthBytes(OSCL_FILE *fp, int32 n)
{
  uint32 tag= 0;
  if(fp && n)
  {
    uint64 filePointer = OSCL_FileTell(fp);
    if(filePointer >= (uint32)(4*n))
    {
      for ( int32 i=0; i<n; i++ )
      {
        (void)AtomUtils::read32(fp, tag);
      }
      (void)OSCL_FileSeek(fp, (filePointer-(4*n)), SEEK_CUR);
    }
  }
  return tag;
}

/* ======================================================================
FUNCTION
  AtomUtils::peekNextByte

DESCRIPTION
  Peeks and returns the next Nth bytes (8 bits) from the file

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint8 AtomUtils::peekNextByte(OSCL_FILE *fp)
{
  uint8 tag= 0;
  if(fp)
  {
    uint64 filePointer = OSCL_FileTell(fp);
    if(filePointer >=1)
    {
      (void)AtomUtils::read8(fp, tag);
      (void)OSCL_FileSeek(fp, filePointer-1, SEEK_CUR);
    }
  }
  return tag;
}

/* ======================================================================
FUNCTION
  AtomUtils::getNextUUIDAtomType

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32
AtomUtils::getNextUUIDAtomType(OSCL_FILE *fp)
{
  uint64 filePointer = OSCL_FileTell(fp);

  if( ( filePointer >= AtomUtils::fileSize )||(filePointer < 12))
  {
    return 0;
  }

  uint32 uuidSize;
  uint32 type;
  if ( !AtomUtils::read32read32(fp, uuidSize, type) )
  {
    return UNKNOWN_ATOM;
  }

  uint32 uuidType;
  if ( !AtomUtils::read32(fp, uuidType) )
  {
    return UNKNOWN_ATOM;
  }

  // Rewinding the stream back to atom start
  (void)OSCL_FileSeek(fp,filePointer-12,SEEK_CUR);

  if ( uuidType == KDDI_DRM_ATOM ||
       uuidType == KDDI_CONTENT_PROPERTY_ATOM ||
       uuidType == KDDI_MOVIE_MAIL_ATOM ||
       uuidType == KDDI_GPS_ATOM ||
       uuidType == KDDI_TELOP_ATOM ||
       uuidType == KDDI_ENCODER_INFO_ATOM )
  {
    return uuidType;
  }
  else
  {
    return UNKNOWN_ATOM; // ERROR condition
  }
}

/* ======================================================================
FUNCTION
  AtomUtils::read32

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read32(uint8 *&buf, uint32 &data)
{
  const int32 N=4;
  data=0;

  for ( int32 i=0;i<N;i++ )
    data = (data<<8) | buf[i];

  buf += N;
  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::read32read32

DESCRIPTION
  Read in the 32 bits byte by byte and take most significant byte first.
  This is equivalent to two read32 calls.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read32read32(uint8 *&buf, uint32 &data1, uint32 &data2)
{
  const int32 N=8;
  data1=0;
  data2=0;

  int32 i;
  for ( i=0;i<4;i++ )
    data1 = (data1<<8) | buf[i];

  for ( i=4;i<8;i++ )
    data2 = (data2<<8) | buf[i];

  buf += N;
  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::read16

DESCRIPTION
  Read in the 16 bits byte by byte and take most significant byte first

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read16(uint8 *&buf, uint16 &data)
{
  const int32 N=2;
  data=0;

  for ( int32 i=0;i<N;i++ )
    data =(uint16) ( (data<<8) |(uint16) buf[i]);

  buf += N;
  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::read8

DESCRIPTION
  Read in the 8 bit byte

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::read8(uint8 *&buf, uint8 &data)
{
  data = 0;
  data = *buf;
  buf++;
  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::readByteData

DESCRIPTION
  Read in byte data and take most significant byte first

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool
AtomUtils::readByteData(uint8 *&buf, uint32 length, uint8 *data)
{
  memcpy(data, buf, length);

  buf += length;
  return true;
}

/* ======================================================================
FUNCTION
  AtomUtils::getNextAtomType

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32
AtomUtils::getNextAtomType(uint8 *buf)
{
  uint32 size;
  uint32 type;
  if ( !AtomUtils::read32read32(buf, size, type) )
  {
    return UNKNOWN_ATOM;
  }

  // Rewinding the stream back to atom start
  buf -= 8;

  if ( type == MOVIE_ATOM ||
       type == MOVIE_HEADER_ATOM ||
       type == TRACK_ATOM ||
       type == TRACK_HEADER_ATOM ||
       type == TRACK_REFERENCE_ATOM ||
       type == MEDIA_ATOM ||
       type == EDIT_ATOM ||
       type == EDIT_LIST_ATOM ||
       type == MEDIA_HEADER_ATOM ||
       type == HANDLER_ATOM ||
       type == MEDIA_INFORMATION_ATOM ||
       type == VIDEO_MEDIA_HEADER_ATOM ||
       type == SOUND_MEDIA_HEADER_ATOM ||
       type == HINT_MEDIA_HEADER_ATOM ||
       type == MPEG4_MEDIA_HEADER_ATOM ||
       type == NULL_MEDIA_HEADER_ATOM ||
       type == DATA_INFORMATION_ATOM ||
       type == DATA_REFERENCE_ATOM ||
       type == DATA_ENTRY_URL_ATOM ||
       type == DATA_ENTRY_URN_ATOM ||
       type == SAMPLE_TABLE_ATOM ||
       type == TIME_TO_SAMPLE_ATOM ||
       type == COMPOSITION_OFFSET_ATOM ||
       type == SAMPLE_DESCRIPTION_ATOM ||
       type == ESD_ATOM ||
       type == SAMPLE_SIZE_ATOM ||
       type == SAMPLE_TO_CHUNK_ATOM ||
       type == CHUNK_OFFSET_ATOM ||
       type == SYNC_SAMPLE_ATOM ||
       type == SHADOW_SYNC_SAMPLE_ATOM ||
       type == DEGRADATION_PRIORITY_ATOM ||
       type == OBJECT_DESCRIPTOR_ATOM ||
       type == MEDIA_DATA_ATOM ||
       type == FREE_SPACE_ATOM ||
       type == SKIP_ATOM ||
       type == USER_DATA_ATOM ||
       type == WMF_SET_MEDIA ||
       type == CONTENT_VERSION_ATOM ||
       type == VIDEO_INFO_ATOM ||
       type == RANDOM_ACCESS_ATOM ||
       type == WMF_SET_SESSION ||
       type == FILE_TYPE_ATOM||
       type == PVUSER_DATA_ATOM ||
       type == HINT_INFORMATION ||
       type == TRACK_INFO_ATOM||
       type == SYNC_INFO_ATOM ||
       type == REQUIREMENTS_ATOM||
       type == PVUSER_DATA_ATOM||
       type == DOWNLOAD_ATOM ||
       type == TRACK_INFO_SESSION_ATOM ||
       type == TRACK_INFO_MEDIA_ATOM ||
       type == AMR_SAMPLE_ENTRY_ATOM ||
       type == H263_SAMPLE_ENTRY_ATOM ||
       type == AUDIO_SAMPLE_ENTRY ||
       type == VIDEO_SAMPLE_ENTRY ||
       type == MPEG_SAMPLE_ENTRY ||
       type == UUID_ATOM ||
       type == AMR_SPECIFIC_ATOM ||
       type == H263_SPECIFIC_ATOM ||
       type == COPYRIGHT_ATOM ||
       type == FONT_TABLE_ATOM ||
       type == TEXT_SAMPLE_ENTRY ||
       type == TEXT_STYLE_BOX ||
       type == TEXT_HIGHLIGHT_BOX ||
       type == TEXT_HILIGHT_COLOR_BOX ||
       type == TEXT_KARAOKE_BOX||
       type == TEXT_SCROLL_DELAY_BOX ||
       type == TEXT_HYPER_TEXT_BOX ||
       type == TEXT_OVER_RIDE_BOX ||
       type == TEXT_BLINK_BOX ||
       type == HINT_TRACK_REFERENCE_TYPE ||
       type == DPND_TRACK_REFERENCE_TYPE ||
       type == IPIR_TRACK_REFERENCE_TYPE ||
       type == MPOD_TRACK_REFERENCE_TYPE ||
       type == SYNC_TRACK_REFERENCE_TYPE ||
       type == KDDI_GPS_EXTENSION_ATOM )
  {
    return type;
  }
  else
  {
    return UNKNOWN_ATOM; // ERROR condition
  }
}

/* ======================================================================
FUNCTION
  AtomUtils::getNextAtomSize

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 AtomUtils::getNextAtomSize(uint8 *buf)
{
  uint32 size;
  (void)AtomUtils::read32(buf, size);

  return (int32)size;
}
