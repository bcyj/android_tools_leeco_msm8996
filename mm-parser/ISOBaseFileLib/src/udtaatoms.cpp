/* =======================================================================
                              udtaAtoms.cpp
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

  Copyright(c) 2008-2014 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/udtaatoms.cpp#18 $
$DateTime: 2013/09/19 20:43:24 $
$Change: 4465512 $

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "filebase.h"
#include "ztl.h"
#include "udtaatoms.h"
#include "videofmt_common.h"
#include "atomdefs.h"
#include "atomutils.h"

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

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */
#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* ======================================================================
FUNCTION
  UdtaMidiAtom::UdtaMidiAtom

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
UdtaMidiAtom::UdtaMidiAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _midiData = NULL;
  _midiDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get midi data size */
    _midiDataSize = Atom::getSize() - 12;
    if(_midiDataSize)
    {
      //! Update seek pointer to end of signature and version flags
      (void)OSCL_FileSeek(fp, _offsetInFile + 3 * FOURCC_SIGNATURE_BYTES,
                          SEEK_SET);
      _midiData = (uint8*)MM_Malloc(_midiDataSize); /* free in destructure */
      if(_midiData)
      {
        if ( !AtomUtils::readByteData(fp, _midiDataSize, _midiData) )
        {
          _midiDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for MIDI failed.");
        _midiDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaMidiAtom::UdtaMidiAtom

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
UdtaMidiAtom::~UdtaMidiAtom()
{
  if(_midiData)
    MM_Free(_midiData);
}

/* ======================================================================
FUNCTION:
  UdtaMidiAtom::getUdtaMidiDataSize

DESCRIPTION:
  returns MIDI data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of MIDI data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaMidiAtom::getUdtaMidiDataSize()
{
  return _midiDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaMidiAtom::getUdtaMidiData

DESCRIPTION:
  copies the MIDI data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaMidiAtom::getUdtaMidiData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_midiDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _midiDataSize-dwOffset);
    memcpy(pBuf, _midiData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}


/* ======================================================================
FUNCTION
  UdtaLinkAtom::UdtaLinkAtom

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
UdtaLinkAtom::UdtaLinkAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _linkData = NULL;
  _linkDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _linkDataSize = Atom::getSize() - 12;
    if(_linkDataSize)
    {
      _linkData = (uint8*)MM_Malloc(_linkDataSize); /* free in destructure */
      if(_linkData)
      {
        if ( !AtomUtils::readByteData(fp, _linkDataSize, _linkData) )
        {
          _linkDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for LINK DATA failed.");
        _linkDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaLinkAtom::~UdtaLinkAtom

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
UdtaLinkAtom::~UdtaLinkAtom()
{
  if(_linkData)
    MM_Free(_linkData);
}

/* ======================================================================
FUNCTION:
  UdtaLinkAtom::getUdtaLinkDataSize

DESCRIPTION:
  returns Link data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of Link data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaLinkAtom::getUdtaLinkDataSize()
{
  return _linkDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaLinkAtom::getUdtaLinkData

DESCRIPTION:
  copies the Link data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaLinkAtom::getUdtaLinkData(uint8* pBuf, uint32 dwSize)
{
  if(_linkDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _linkDataSize);
    memcpy(pBuf, _linkData, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

/* ======================================================================
FUNCTION
  FtypAtom::FtypAtom

DESCRIPTION
  Filt Type atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
FtypAtom::FtypAtom(OSCL_FILE *fp)
: Atom(fp)
{
  _ftypData = NULL;
  _ftypDataSize = 0;

  if ( _success )
  {

    /* skip 4 bytes size, 4 bytes type to get ftyp data size */
    _ftypDataSize = Atom::getSize() - 8;
    if(_ftypDataSize)
    {
      _ftypData = (uint8*)MM_Malloc(_ftypDataSize); /* free in destructure */
      if(_ftypData)
      {
        if ( !AtomUtils::readByteData(fp, _ftypDataSize, _ftypData) )
        {
          _ftypDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for FTYP DATA failed.");
        _ftypDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  FtypAtom::~FtypAtom

DESCRIPTION
  Filt Type atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
FtypAtom::~FtypAtom()
{
  if(_ftypData)
    MM_Free(_ftypData);
}

/* ======================================================================
FUNCTION:
  FtypAtom::getFtypDataSize

DESCRIPTION:
  returns ftyp data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of ftyp data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FtypAtom::getFtypDataSize()
{
  return _ftypDataSize;
}

/* ======================================================================
FUNCTION:
  FtypAtom::getFtypData

DESCRIPTION:
  copies the ftyp data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FtypAtom::getFtypData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_ftypDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _ftypDataSize-dwOffset);
    memcpy(pBuf, _ftypData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}


/* ======================================================================
FUNCTION
  DrefAtom::DrefAtom

DESCRIPTION
  Data Reference atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
DrefAtom::DrefAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _drefData = NULL;
  _drefDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _drefDataSize = Atom::getSize() - 12;
    if(_drefDataSize)
    {
      _drefData = (uint8*)MM_Malloc(_drefDataSize); /* free in destructure */
      if(_drefData)
      {
        if ( !AtomUtils::readByteData(fp, _drefDataSize, _drefData) )
        {
          _drefDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for DREF DATA failed.");
        _drefDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  DrefAtom::~DrefAtom

DESCRIPTION
  Data Reference atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
DrefAtom::~DrefAtom()
{
  if(_drefData)
    MM_Free(_drefData);
}

/* ======================================================================
FUNCTION:
  DrefAtom::getDrefDataSize

DESCRIPTION:
  returns dref data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of dref data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 DrefAtom::getDrefDataSize()
{
  return _drefDataSize;
}

/* ======================================================================
FUNCTION:
  DrefAtom::getDrefData

DESCRIPTION:
  copies the dref data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 DrefAtom::getDrefData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_drefDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _drefDataSize-dwOffset);
    memcpy(pBuf, _drefData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
/* ======================================================================
FUNCTION
  CSubsAtom::CSubsAtom

DESCRIPTION
  Sub sample atom constructor.

DEPENDENCIES
  None.

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
CSubsAtom::CSubsAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  pSubsData = NULL;
  ulSubsDataSize = 0;
  ulEntryCount = 0;
  ulSampleDelta = 0;
  usSubSampleCount = 0;

  if ( _success )
  {
    /* skip 4 bytes size, 4 bytes type */
    ulSubsDataSize = Atom::getSize() - DEFAULT_ATOM_SIZE;
    // Get current file position
    uint64 ullCurPos = OSCL_FileTell (fp);
    // Move back to (ullCurPos - 4) to include version & flag in data
    (void)OSCL_FileSeek(fp, ullCurPos - 4, SEEK_SET);
    if(ulSubsDataSize)
    {
      pSubsData = (uint8*)MM_Malloc(ulSubsDataSize); /* free in DTOR */
      if(pSubsData)
      {
        if ( !AtomUtils::readByteData(fp, ulSubsDataSize, pSubsData) )
        {
          ulSubsDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        else
        {
          uint8* pTemp = pSubsData + 4;
          //Read Entry count & Sample Delta
          if ( !AtomUtils::read32read32(pTemp, ulEntryCount, ulSampleDelta) )
          {
            _success = false;
            _fileErrorCode = PARSER_ErrorReadFail;
            return;
          }
          //Read subsample count
          if ( !AtomUtils::read16(pTemp, usSubSampleCount) )
          {
            _success = false;
            _fileErrorCode = PARSER_ErrorReadFail;
            return;
          }
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for SUBS DATA failed.");
        ulSubsDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  CSubsAtom::~CSubsAtom

DESCRIPTION
  Sub Sample atom destructor.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  None.

========================================================================== */
CSubsAtom::~CSubsAtom()
{
  if(pSubsData)
    MM_Free(pSubsData);
}

/* ======================================================================
FUNCTION:
  CSubsAtom::GetSubsData

DESCRIPTION:
  copies the subs data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 CSubsAtom::GetSubsData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(ulSubsDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, ulSubsDataSize - dwOffset);
    memcpy(pBuf, pSubsData + dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

/* ======================================================================
FUNCTION
  UdtaCprtAtom::UdtaCprtAtom

DESCRIPTION
  Copyright atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaCprtAtom::UdtaCprtAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _cprtData = NULL;
  _cprtDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _cprtDataSize = Atom::getSize() - 12;
    if(_cprtDataSize)
    {
      _cprtData = (uint8*)MM_Malloc(_cprtDataSize); /* free in destructure */
      if(_cprtData)
      {
        if ( !AtomUtils::readByteData(fp, _cprtDataSize, _cprtData) )
        {
          _cprtDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for CPRT DATA failed.");
        _cprtDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaCprtAtom::~UdtaCprtAtom

DESCRIPTION
  Copyright atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaCprtAtom::~UdtaCprtAtom()
{
  if(_cprtData)
    MM_Free(_cprtData);
}

/* ======================================================================
FUNCTION:
  UdtaCprtAtom::getUdtaCprtDataSize

DESCRIPTION:
  returns cprt data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of cprt data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaCprtAtom::getUdtaCprtDataSize()
{
  return _cprtDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaCprtAtom::getUdtaCprtData

DESCRIPTION:
  copies the cprt data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaCprtAtom::getUdtaCprtData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_cprtDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _cprtDataSize-dwOffset);
    memcpy(pBuf, _cprtData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}


/* ======================================================================
FUNCTION
  UdtaAuthAtom::UdtaAuthAtom

DESCRIPTION
  Author atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaAuthAtom::UdtaAuthAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _authData = NULL;
  _authDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _authDataSize = Atom::getSize() - 12;
    if(_authDataSize)
    {
      _authData = (uint8*)MM_Malloc(_authDataSize); /* free in destructure */
      if(_authData)
      {
        if ( !AtomUtils::readByteData(fp, _authDataSize, _authData) )
        {
          _authDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for AUTH DATA failed.");
        _authDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaAuthAtom::~UdtaAuthAtom

DESCRIPTION
  Author atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaAuthAtom::~UdtaAuthAtom()
{
  if(_authData)
    MM_Free(_authData);
}

/* ======================================================================
FUNCTION:
  UdtaAuthAtom::getUdtaAuthDataSize

DESCRIPTION:
  returns auth data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of auth data.

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaAuthAtom::getUdtaAuthDataSize()
{
  return _authDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaAuthAtom::getUdtaAuthData

DESCRIPTION:
  copies the auth data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaAuthAtom::getUdtaAuthData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_authDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _authDataSize-dwOffset);
    memcpy(pBuf, _authData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

/* ======================================================================
FUNCTION
  UdtaAlbumAtom::UdtaAlbumAtom

DESCRIPTION
  Author atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaAlbumAtom::UdtaAlbumAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _albumData = NULL;
  _albumDataSize = 0;
  memset(_TrackNum, 0, 4);

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _albumDataSize = Atom::getSize() - 12;
    if(_albumDataSize)
    {
      //! Update seek pointer to end of signature and version flags
      (void)OSCL_FileSeek(fp, _offsetInFile + 3 * FOURCC_SIGNATURE_BYTES,
                          SEEK_SET);
      _albumData = (uint8*)MM_Malloc(_albumDataSize); /* free in destructur */
      if(_albumData)
      {
        if ( !AtomUtils::readByteData(fp, _albumDataSize, _albumData) )
        {
          _albumDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        else
        {
          /* This field can max be 255 (8bits). To store this number in string,
             we need 4 bytes, 3bytes for data and one byte for NULL char.
             Last byte is typically used to store Track Number.
             If this field is non-ZERO, Use it to create Track num string */
          if (_albumData[_albumDataSize - 1])
          {
#ifdef _ANDROID_
            snprintf((char*)_TrackNum, 4, (char*)"%d", (int)_albumData[_albumDataSize - 1]);
#else
            std_strlprintf((char*)_TrackNum, 4, (char*)"%d", (int)_albumData[_albumDataSize - 1]);
#endif
          //! Last byte is not included in Album String
          _albumDataSize--;
          }
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for ALBUM DATA failed.");
        _albumDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaAlbumAtom::~UdtaAlbumAtom

DESCRIPTION
  Author atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaAlbumAtom::~UdtaAlbumAtom()
{
  if(_albumData)
    MM_Free(_albumData);
}

/* ======================================================================
FUNCTION:
  UdtaAlbumAtom::getUdtaAlbumDataSize

DESCRIPTION:
  returns auth data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of auth data.

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaAlbumAtom::getUdtaAlbumDataSize()
{
  return _albumDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaAlbumAtom::getUdtaAlbumData

DESCRIPTION:
  copies the auth data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaAlbumAtom::getUdtaAlbumData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_albumDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _albumDataSize-dwOffset);
    memcpy(pBuf, _albumData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

/* ======================================================================
FUNCTION
  UdtaYrrcAtom::UdtaYrrcAtom

DESCRIPTION
  YRCC atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaYrrcAtom::UdtaYrrcAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _yrrcData = NULL;
  _yrrcDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _yrrcDataSize = Atom::getSize() - 12;
    if(_yrrcDataSize)
    {
      //! Update seek pointer to end of signature and version flags
      (void)OSCL_FileSeek(fp, _offsetInFile + 3 * FOURCC_SIGNATURE_BYTES,
                          SEEK_SET);
      _yrrcData = (uint8*)MM_Malloc(FILESOURCE_MAX(_yrrcDataSize + 1,
                                                   2*FOURCC_SIGNATURE_BYTES));
      if(_yrrcData)
      {
        memset(_yrrcData, 0, _yrrcDataSize + 1);
        if ( !AtomUtils::readByteData(fp, _yrrcDataSize, _yrrcData) )
        {
          _yrrcDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        else
        {
          int nValue = 0;
          //! Generate integer format number from string (It is in Hex Format)
          for (int nIndex = 0; nIndex < (int)_yrrcDataSize; nIndex++)
          {
            nValue <<= 8;
            nValue   |= _yrrcData[nIndex];
          }
          //! Convert integer in Decimal format to string format
          //! Typically year size can not be more than 4 bytes and one byte
          //! for NULL character. Use 5 bytes to store Year info.
#ifdef _ANDROID_
          snprintf((char*)_yrrcData, 5, (char*)"%d", (int)nValue);
#else
          std_strlprintf((char*)_yrrcData, 5, (char*)"%d", (int)nValue);
#endif
          _yrrcDataSize = strlen((const char*)_yrrcData) + 1;
          _yrrcData[_yrrcDataSize - 1] = '\0';
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM, "Year Atom Size = %lu, _yrrcData %s",
                       _yrrcDataSize, _yrrcData);
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for YRRC DATA failed.");
        _yrrcDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaYrrcAtom::~UdtaYrrcAtom

DESCRIPTION
  YRCC atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaYrrcAtom::~UdtaYrrcAtom()
{
  if(_yrrcData)
    MM_Free(_yrrcData);
}


/* ======================================================================
FUNCTION:
  UdtaYrrcAtom::getUdtaYrrcDataSize

DESCRIPTION:
  returns YRCC data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of auth data.

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaYrrcAtom::getUdtaYrrcDataSize()
{
  return _yrrcDataSize;
}


/* ======================================================================
FUNCTION:
  UdtaYrrcAtom::getUdtaYrrcData

DESCRIPTION:
  copies the YRCC data.

INPUT/OUTPUT PARAMETERS:
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaYrrcAtom::getUdtaYrrcData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_yrrcDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _yrrcDataSize-dwOffset);
    memcpy(pBuf, _yrrcData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

/* ======================================================================
FUNCTION
  UdtaTitlAtom::UdtaTitlAtom

DESCRIPTION
  Title atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaTitlAtom::UdtaTitlAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _titlData = NULL;
  _titlDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _titlDataSize = Atom::getSize() - 12;
    if(_titlDataSize)
    {
      //! Update seek pointer to end of signature and version flags
      (void)OSCL_FileSeek(fp, _offsetInFile + 3 * FOURCC_SIGNATURE_BYTES,
                          SEEK_SET);
      _titlData = (uint8*)MM_Malloc(_titlDataSize); /* free in destructure */
      if(_titlData)
      {
        if ( !AtomUtils::readByteData(fp, _titlDataSize, _titlData) )
        {
          _titlDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for TITL DATA failed.");
        _titlDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaTitlAtom::~UdtaTitlAtom

DESCRIPTION
  Title atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaTitlAtom::~UdtaTitlAtom()
{
  if(_titlData)
    MM_Free(_titlData);
}

/* ======================================================================
FUNCTION:
  UdtaTitlAtom::getUdtaTitlDataSize

DESCRIPTION:
  returns titl data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of titl data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaTitlAtom::getUdtaTitlDataSize()
{
  return _titlDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaTitlAtom::getUdtaTitlData

DESCRIPTION:
  copies the titl data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaTitlAtom::getUdtaTitlData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_titlDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _titlDataSize-dwOffset);
    memcpy(pBuf, _titlData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}


/* ======================================================================
FUNCTION
  UdtaDscpAtom::UdtaDscpAtom

DESCRIPTION
  Description atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaDscpAtom::UdtaDscpAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _dscpData = NULL;
  _dscpDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _dscpDataSize = Atom::getSize() - 12;
    if(_dscpDataSize)
    {
      _dscpData = (uint8*)MM_Malloc(_dscpDataSize); /* free in destructure */
      if(_dscpData)
      {
        if ( !AtomUtils::readByteData(fp, _dscpDataSize, _dscpData) )
        {
          _dscpDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for TITL DATA failed.");
        _dscpDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaDscpAtom::~UdtaDscpAtom

DESCRIPTION
  Description atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaDscpAtom::~UdtaDscpAtom()
{
  if(_dscpData)
    MM_Free(_dscpData);
}

/* ======================================================================
FUNCTION:
  UdtaDscpAtom::getUdtaDscpDataSize

DESCRIPTION:
  returns dscp data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of dscp data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaDscpAtom::getUdtaDscpDataSize()
{
  return _dscpDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaDscpAtom::getUdtaDscpData

DESCRIPTION:
  copies the dscp data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaDscpAtom::getUdtaDscpData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_dscpDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _dscpDataSize-dwOffset);
    memcpy(pBuf, _dscpData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
FUNCTION
  UdtaRtngAtom::UdtaRtngAtom

DESCRIPTION
  Rating atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaRtngAtom::UdtaRtngAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _rtngData = NULL;
  _rtngDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag ,4bytes of Rating Entity,4 bytes Rating Criteria to get link data size */
    _rtngDataSize = Atom::getSize() - 20;
    if(_rtngDataSize)
    {
      //! Update seek pointer to end of signature and version flags
      (void)OSCL_FileSeek(fp, _offsetInFile + 3 * FOURCC_SIGNATURE_BYTES,
                          SEEK_SET);
      _rtngData = (uint8*)MM_Malloc(_rtngDataSize); /* free in destructure */
      if(_rtngData )
      {
        if ( !AtomUtils::readByteData(fp, _rtngDataSize, _rtngData) )
        {
          _rtngDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for RTNG DATA failed.");
        _rtngDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaRtngAtom::~UdtaRtngAtom

DESCRIPTION
  Rating atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaRtngAtom::~UdtaRtngAtom()
{
  if(_rtngData)
    MM_Free(_rtngData);
}

/* ======================================================================
FUNCTION:
  UdtaRtngAtom::getUdtaRtngDataSize

DESCRIPTION:
  returns rtng data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of rtng data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaRtngAtom::getUdtaRtngDataSize()
{
  return _rtngDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaRtngAtom::getUdtaRtngData

DESCRIPTION:
  copies the rtng data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaRtngAtom::getUdtaRtngData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_rtngDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _rtngDataSize-dwOffset);
    memcpy(pBuf, _rtngData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
FUNCTION
  UdtaGnreAtom::UdtaGnreAtom

DESCRIPTION
  Genreatom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaGnreAtom::UdtaGnreAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _gnreData = NULL;
  _gnreDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _gnreDataSize = Atom::getSize() - 12;
    if(_gnreDataSize)
    {
      _gnreData = (uint8*)MM_Malloc(_gnreDataSize); /* free in destructure */
      if(_gnreData )
      {
        if ( !AtomUtils::readByteData(fp, _gnreDataSize, _gnreData) )
        {
          _gnreDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for GNRE DATA failed.");
        _gnreDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaGnreAtom::~UdtaGnreAtom

DESCRIPTION
  Genre atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaGnreAtom::~UdtaGnreAtom()
{
  if(_gnreData)
    MM_Free(_gnreData);
}

/* ======================================================================
FUNCTION:
  UdtaGnreAtom::getUdtaGnreDataSize

DESCRIPTION:
  returns gnre data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of gnre data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaGnreAtom::getUdtaGnreDataSize()
{
  return _gnreDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaGnreAtom::getUdtaGnreData

DESCRIPTION:
  copies the Gnre data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaGnreAtom::getUdtaGnreData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_gnreDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _gnreDataSize-dwOffset);
    memcpy(pBuf, _gnreData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
FUNCTION
  UdtaPerfAtom::UdtaPerfAtom

DESCRIPTION
  Perf atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaPerfAtom::UdtaPerfAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _perfData = NULL;
  _perfDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _perfDataSize = Atom::getSize() - 12;
    if(_perfDataSize)
    {
      //! Update seek pointer to end of signature and version flags
      (void)OSCL_FileSeek(fp, _offsetInFile + 3 * FOURCC_SIGNATURE_BYTES,
                          SEEK_SET);
      _perfData = (uint8*)MM_Malloc(_perfDataSize); /* free in destructure */
      if(_perfData )
      {
        if ( !AtomUtils::readByteData(fp, _perfDataSize, _perfData) )
        {
          _perfDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for PERF DATA failed.");
        _perfDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaPerfAtom::~UdtaPerfAtom

DESCRIPTION
  Performance atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaPerfAtom::~UdtaPerfAtom()
{
  if(_perfData)
    MM_Free(_perfData);
}

/* ======================================================================
FUNCTION:
  UdtaPerfAtom::getUdtaPerfDataSize

DESCRIPTION:
  returns perf data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of perf data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaPerfAtom::getUdtaPerfDataSize()
{
  return _perfDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaPerfAtom::getUdtaPerfData

DESCRIPTION:
  copies the Perf data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaPerfAtom::getUdtaPerfData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_perfDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _perfDataSize-dwOffset);
    memcpy(pBuf, _perfData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
FUNCTION
  UdtaClsfAtom::UdtaClsfAtom

DESCRIPTION
  Classification atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaClsfAtom::UdtaClsfAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _clsfData = NULL;
  _clsfDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag ,4bytes of classification Entity,
  2bytes of classification table to get link data size */
    _clsfDataSize = Atom::getSize() - 18;
    if(_clsfDataSize)
    {
      _clsfData = (uint8*)MM_Malloc(_clsfDataSize); /* free in destructure */
      if(_clsfData )
      {
        if ( !AtomUtils::readByteData(fp, _clsfDataSize, _clsfData) )
        {
          _clsfDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for CLSF DATA failed.");
        _clsfDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaClsfAtom::~UdtaClsfAtom

DESCRIPTION
  Classification atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaClsfAtom::~UdtaClsfAtom()
{
  if(_clsfData)
    MM_Free(_clsfData);
}

/* ======================================================================
FUNCTION:
  UdtaClsfAtom::getUdtaClsfDataSize

DESCRIPTION:
  returns clsf data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of clsf data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaClsfAtom::getUdtaClsfDataSize()
{
  return _clsfDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaClsfAtom::getUdtaClsfData

DESCRIPTION:
  copies the Classification data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaClsfAtom::getUdtaClsfData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_clsfDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _clsfDataSize-dwOffset);
    memcpy(pBuf, _clsfData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
FUNCTION
  UdtaKywdAtom::UdtaKywdAtom

DESCRIPTION
  Keyword atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaKywdAtom::UdtaKywdAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _kywdData = NULL;
  _kywdDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _kywdDataSize = Atom::getSize() - 12;
    if(_kywdDataSize)
    {
      _kywdData = (uint8*)MM_Malloc(_kywdDataSize); /* free in destructure */
      if(_kywdData )
      {
        if ( !AtomUtils::readByteData(fp, _kywdDataSize, _kywdData) )
        {
          _kywdDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for KYWD DATA failed.");
        _kywdDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaKywdAtom::~UdtaKywdAtom

DESCRIPTION
  Keyword atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaKywdAtom::~UdtaKywdAtom()
{
  if(_kywdData)
    MM_Free(_kywdData);
}

/* ======================================================================
FUNCTION:
  UdtaKywdAtom::getUdtaKywdDataSize

DESCRIPTION:
  returns kywd data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of kywd data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaKywdAtom::getUdtaKywdDataSize()
{
  return _kywdDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaKywdAtom::getUdtaKywdData

DESCRIPTION:
  copies the Keyword data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaKywdAtom::getUdtaKywdData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_kywdDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _kywdDataSize-dwOffset);
    memcpy(pBuf, _kywdData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
/* ======================================================================
FUNCTION
  UdtaLociAtom::UdtaLociAtom

DESCRIPTION
  Location atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaLociAtom::UdtaLociAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _lociData = NULL;
  _lociDataSize = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _lociDataSize = Atom::getSize() - 12;
    if(_lociDataSize)
    {
      //! Update seek pointer to end of signature and version flags
      (void)OSCL_FileSeek(fp, _offsetInFile + 3 * FOURCC_SIGNATURE_BYTES,
                          SEEK_SET);
      _lociData = (uint8*)MM_Malloc(_lociDataSize); /* free in destructure */
      if(_lociData )
      {
        if ( !AtomUtils::readByteData(fp, _lociDataSize, _lociData) )
        {
          _lociDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for LOCI DATA failed.");
        _lociDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaLociAtom::~UdtaLociAtom

DESCRIPTION
  Location atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaLociAtom::~UdtaLociAtom()
{
  if(_lociData)
    MM_Free(_lociData);
}

/* ======================================================================
FUNCTION:
  UdtaLociAtom::getUdtaLociDataSize

DESCRIPTION:
  returns loci data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of loci data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaLociAtom::getUdtaLociDataSize()
{
  return _lociDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaLociAtom::getUdtaLociData

DESCRIPTION:
  copies the Location data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaLociAtom::getUdtaLociData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_lociDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _lociDataSize-dwOffset);
    memcpy(pBuf, _lociData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

/* ======================================================================
FUNCTION
  UdtaMetaAtom::UdtaMetaAtom

DESCRIPTION
  Meta atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaMetaAtom::UdtaMetaAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _metaData         = NULL;
  _metaDataSize     = 0;
  m_uliLstAtomCount = 0;
  m_ulID3AtomCount  = 0;

  if ( _success )
  {
    if( getVersion() != 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    /* skip 4 bytes size, 4 bytes type and 4 bytes ver/flag to get link data size */
    _metaDataSize = Atom::getSize() - 12;
    if(_metaDataSize)
    {
      _metaData = (uint8*)MM_Malloc(_metaDataSize); /* free in destructure */
      if(_metaData )
      {
        //! Update seek pointer to end of signature and version flags
        (void)OSCL_FileSeek(fp, _offsetInFile + 3 * FOURCC_SIGNATURE_BYTES,
                            SEEK_SET);

        if ( AtomUtils::readByteData(fp, _metaDataSize, _metaData) )
        {
          Parse();
        }
        else
        {
          _metaDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Memory allocation for META DATA failed.");
        _metaDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaMetaAtom::~UdtaMetaAtom

DESCRIPTION
  Location atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaMetaAtom::~UdtaMetaAtom()
{
  uint32 ulCount = 0;
  if(_metaData)
    MM_Free(_metaData);
  for (ulCount = 0; ulCount < m_uliLstAtomCount; ulCount++)
  {
    MM_Delete(m_aiLstAtomEntryArray[ulCount]);
  }
  for (ulCount = 0; ulCount < m_ulID3AtomCount; ulCount++)
  {
    metadata_id3v2_type* pID3Meta = m_aID3AtomArray[ulCount];
    FreeID3V2MetaDataMemory(pID3Meta);
    MM_Free( pID3Meta);
  }

}

/* ======================================================================
FUNCTION:
  UdtaLociAtom::getUdtaLociDataSize

DESCRIPTION:
  returns loci data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of loci data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaMetaAtom::getUdtaMetaDataSize()
{
  return _metaDataSize;
}

/* ======================================================================
FUNCTION:
  UdtaLociAtom::getUdtaMetaData

DESCRIPTION:
  copies the Location data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaMetaAtom::getUdtaMetaData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_metaDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _metaDataSize-dwOffset);
    memcpy(pBuf, _metaData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

/* ======================================================================
FUNCTION
  UdtaMetaAtom

DESCRIPTION
  Parse the iLst Atom and update relevant atom field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
void UdtaMetaAtom::Parse()
{
  uint8*  puciLstDataStartPtr = NULL;
  uint32  ulByteConsumed      = 0;
  uint32  ulAtomType          = 0;
  uint32  ulAtomSize          = 0;
  uint32  uliLstDataOffset    = 0;
  bool    bSupportedHdlrType  = false;

  //! uliLstDataOffset keep track of Atom offset.
  uliLstDataOffset = 0;

  //! _metaData point to start of each atom
  puciLstDataStartPtr = _metaData;

  while((uliLstDataOffset < _metaDataSize) && _metaData)
  {
    uint8* pTempBuf = puciLstDataStartPtr;
    /* ! Get Atom Size and type from Meta Data buffer.
       ! This function internally updates buffer pointer to end of signature.*/
    (void)AtomUtils::read32read32(pTempBuf, ulAtomSize, ulAtomType);
    ulByteConsumed += DEFAULT_ATOM_SIZE;
    //! Validate Atom type and size fields
    if (!ulAtomType || !ulAtomSize || (ulAtomSize >= _metaDataSize))
    {
      break;
    }

    /* If atom type is "iLst" and handler type is "mdir" then create iLstAtom
       object to parse iLst atom. */
    if ((ILST_TYPE == ulAtomType) && (true == bSupportedHdlrType))
    {
      //Reset flag
      bSupportedHdlrType = false;

      //! Pass the buffer to "iLst" constructor.
      UdtaiLstAtom *piLstAtom= MM_New_Args( UdtaiLstAtom,
                                            (puciLstDataStartPtr) );
      if (piLstAtom)
      {
        m_aiLstAtomEntryArray += piLstAtom;
        m_uliLstAtomCount++;
      }
    }
    else if ((ID32_TYPE == ulAtomType) && (true == bSupportedHdlrType))
    {
      //Reset flag
      bSupportedHdlrType = false;
      PARSER_ERRORTYPE eRetStatus = PARSER_ErrorNone;

      // Get an object pointer of the ID3v1 parser class
      ID3v2 *pID3v2 = MM_New_Args(ID3v2,(eRetStatus));
      metadata_id3v2_type* pId3MetaData = (metadata_id3v2_type*)
                                     MM_Malloc(sizeof(metadata_id3v2_type));
      if ((pID3v2) && (pId3MetaData) )
      {
        memset(pId3MetaData, 0, sizeof(metadata_id3v2_type));
        /*
         Version    - 24 bits
         Flags      - 8  bits
         Pad        - 1  bit
         Language   - 15 bits
         Ignore these first 6 bytes before calling ID3 class.
        */
        ulByteConsumed += 6;
        OSCL_FILE* pFilePtr = OSCL_FileOpen(puciLstDataStartPtr+ulByteConsumed,
                                            ulAtomSize - ulByteConsumed);
        // Parse the ID3 tag and save the contents
        eRetStatus = pID3v2->parse_ID3v2_tag(pFilePtr, 0, pId3MetaData, false);
        m_aID3AtomArray += pId3MetaData;
        m_ulID3AtomCount++;
        (void)OSCL_FileClose( pFilePtr );
      }
      if (pID3v2)
        MM_Delete(pID3v2);
    }
    else if (HDLR_TYPE == ulAtomType)
    {
      /* Skip 1byte for version, 3bytes for flag and 4bytes for reserved for
         future purpose. */
      ulByteConsumed += (2 * FOURCC_SIGNATURE_BYTES);
      if((!memcmp(puciLstDataStartPtr + ulByteConsumed, "mdir",
                  FOURCC_SIGNATURE_BYTES)) ||
         (!memcmp(puciLstDataStartPtr + ulByteConsumed, "ID32",
                  FOURCC_SIGNATURE_BYTES)) )
      {
        bSupportedHdlrType = true;
      }
    }

    //! uliLstDataOffset will point to next atom.
    uliLstDataOffset += ulAtomSize;
    puciLstDataStartPtr =  _metaData + uliLstDataOffset;
    ulByteConsumed = 0;
  }
}

/* ======================================================================
FUNCTION
  UdtaiLstAtom::UdtaiLstAtom

DESCRIPTION
  Classification atom constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaiLstAtom::UdtaiLstAtom(uint8 *pBuf): Atom(pBuf)
{
  m_puciLstData     = NULL;
  m_uliLstDataSize  = 0;
  m_ulMetaAtomCount = 0;

  if ( (_success) && (pBuf) )
  {
    /* skip 4 bytes size, 4 bytes type  */
    m_uliLstDataSize = Atom::getSize() - DEFAULT_ATOM_SIZE;
    if(m_uliLstDataSize)
    {
      m_puciLstData = (uint8*)MM_Malloc(m_uliLstDataSize);
      if(m_puciLstData )
      {
        /* iLst atom is container only, it will not have any version and flag
           info associated with this atom.*/
        memcpy(m_puciLstData, pBuf, m_uliLstDataSize);
        Parse();
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "Memory allocation for iLst DATA failed.");
        m_uliLstDataSize = 0;
        _success         = false;
        _fileErrorCode   = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    _success       = false;
    return;
  }
}

/* ======================================================================
FUNCTION
  UdtaiLstAtom::~UdtaiLstAtom

DESCRIPTION
  Classification atom destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
UdtaiLstAtom::~UdtaiLstAtom()
{
  if(m_puciLstData)
  {
    MM_Free(m_puciLstData);
    m_puciLstData = NULL;
  }
  for(uint32 ulCount = 0; ulCount < m_ulMetaAtomCount; ulCount++)
  {
    if((m_aMetaAtomEntryArray)[ulCount] &&
       (m_aMetaAtomEntryArray)[ulCount] != NULL)
    {
      ItunesMetaData* pMetaData = (m_aMetaAtomEntryArray)[ulCount];
      if (pMetaData->pucMetaData)
      {
        MM_Free(pMetaData->pucMetaData);
      }
      MM_Free( pMetaData );
      (m_aMetaAtomEntryArray)[ulCount] = NULL;
    }
  }
}

/* ======================================================================
FUNCTION:
  UdtaiLstAtom::getUdtaiLstData

DESCRIPTION:
  copies the Classification data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 UdtaiLstAtom::getUdtaiLstData(uint8* pBuf, uint32 dwSize,
                                     uint32 dwOffset)
{
  if((m_uliLstDataSize) && (pBuf))
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize,
                                             (m_uliLstDataSize - dwOffset));
    memcpy(pBuf, m_puciLstData + dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}

/* ======================================================================
FUNCTION
UdtaiLstAtom::Parse

DESCRIPTION
  Parse the iLst Atom and update relevant atom field.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
void UdtaiLstAtom::Parse()
{
  uint8 * puciLstDataStartPtr = NULL;
  uint32  ulByteConsumed = 0;
  uint32  ulAtomType = 0,ulAtomSize = 0;
  uint32  uliLstDataOffset = 0;

  ItunesMetaData *pMetaData = NULL;

  //! uliLstDataOffset keep track of Atom offset.
  uliLstDataOffset = 0;

  //! m_pucSinfData point to start
  puciLstDataStartPtr = m_puciLstData;

  while((uliLstDataOffset < m_uliLstDataSize) && (puciLstDataStartPtr))
  {
    uint8* pTempBuf = puciLstDataStartPtr;
    ulByteConsumed = 0;
    pMetaData      = NULL;

    //Get Atom size and type info
    (void)AtomUtils::read32read32(pTempBuf, ulAtomSize, ulAtomType);
    //!Validate Atom type and size fields
    if ((!ulAtomType) || (!ulAtomSize) || (ulAtomSize >= m_uliLstDataSize))
    {
      break;
    }

    ulByteConsumed += DEFAULT_ATOM_SIZE;

    if (FREE_FORM == ulAtomType)
    {
      pMetaData = ParseFreeAtom(puciLstDataStartPtr, ulAtomSize);
    }
    else
    {
      FileSourceMetaDataType eMetaType = FILE_SOURCE_MD_UNKNOWN;
      if (IALB_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_ALBUM;
      }
      else if (IART_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_ARTIST;
      }
      else if (ICMT_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_INFO;
      }
      else if (ICVR_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_ALBUM_ART;
      }
      else if (IDAT_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_REC_YEAR;
      }
      else if (IGEN_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_GENRE;
      }
      else if (INAM_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_TITLE;
      }
      else if (IWRT_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_WRITER;
      }
      else if (ALB_ART_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_ALBUM_ARTIST;
      }
      else if (CPIL_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_COMPILATION;
      }
      else if (GNRE_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_GENRE;
      }
      else if (CPRT_TYPE == ulAtomType)
      {
        eMetaType = FILE_SOURCE_MD_COPYRIGHT;
      }

      if (FILE_SOURCE_MD_UNKNOWN != eMetaType)
      {
        pMetaData = (ItunesMetaData *)MM_Malloc(sizeof(ItunesMetaData));

        if (pMetaData)
        {
          memset(pMetaData, 0, sizeof(ItunesMetaData));
          uint32 ulDataSize   = 0;
          uint32 ulDataType   = 0;
          //It keeps track of amount of data consumed in current atom
          uint32 ulDataOffset = 0;

          //Get DATA Atom size and type info
          (void)AtomUtils::read32read32(pTempBuf, ulDataSize, ulDataType);
          ulByteConsumed += DEFAULT_ATOM_SIZE;
          ulDataOffset   += DEFAULT_ATOM_SIZE;
          //Skip 3bytes used for typeReserved (2) and typeset Identifier (1)
          ulByteConsumed += 3;

          //!Validate Atom type and size fields
          if ((DATA_TYPE == ulDataType) && (ulDataSize) &&
               (ulDataSize < ulAtomSize))
          {
            pMetaData->eMetaType = eMetaType;
            pMetaData->eDataType = (ITUNES_DATA_TYPE)
                                   puciLstDataStartPtr[ulByteConsumed];
            ulByteConsumed++;
            ulDataOffset   += FOURCC_SIGNATURE_BYTES;
            copyByteSwapData((uint8*)&pMetaData->ulLocaleIndicator, 4,
                             puciLstDataStartPtr + ulByteConsumed, 1,
                             m_uliLstDataSize);
            ulByteConsumed += FOURCC_SIGNATURE_BYTES;
            ulDataOffset   += FOURCC_SIGNATURE_BYTES;
            pMetaData->pucMetaData = (uint8*)MM_Malloc(ulDataSize );
            if (pMetaData->pucMetaData)
            {
              //Update Metadata Length field
              pMetaData->ulMetaDataLen = ulDataSize - ulDataOffset;
              memcpy(pMetaData->pucMetaData,
                     puciLstDataStartPtr + ulByteConsumed,
                     pMetaData->ulMetaDataLen);
              pMetaData->pucMetaData[pMetaData->ulMetaDataLen] = '\0';
              pMetaData->ulMetaDataLen++;

              /* If Data type is integer format, then convert this field into
                 string format */
              if ((ITUNES_GENRE == pMetaData->eDataType) ||
                  (ITUNES_INTEGER == pMetaData->eDataType))
              {
#ifdef _ANDROID_
                snprintf((char*)pMetaData->pucMetaData,
                         pMetaData->ulMetaDataLen, (char*)"%lu",
                         (unsigned long)(puciLstDataStartPtr + ulByteConsumed));
#else
                std_strlprintf((char*)pMetaData->pucMetaData,
                               pMetaData->ulMetaDataLen, (char*)"%lu",
                               puciLstDataStartPtr + ulByteConsumed);
#endif
              }
            } //if (pMetaData->pucMetaData)
          } //if (DATA_TYPE == ulDataType)
        } //if (pMetaData)
      } //if (FILE_SOURCE_MD_UNKNOWN != eMetaType)
    } // else of if(FREE_FORM == ulAtomType)
    if(pMetaData)
    {
      m_aMetaAtomEntryArray += pMetaData;
      m_ulMetaAtomCount++;
    }
    //! uliLstDataOffset will point to next atom.
    uliLstDataOffset += ulAtomSize;
    puciLstDataStartPtr =  m_puciLstData + uliLstDataOffset;
    ulByteConsumed = 0;
  } //while((uliLstDataOffset < m_uliLstDataSize) && (puciLstDataStartPtr))
  return;
}

/* ======================================================================
FUNCTION
  UdtaiLstAtom::ParseFreeAtom

DESCRIPTION
  Function to parse Free Form Atom in iLst

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Pointer to Metadata Structure in successful case, else NULL

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
ItunesMetaData* UdtaiLstAtom::ParseFreeAtom(uint8 *pucBufPtr,
                                            uint32 ulFreeAtomSize)
{
  ItunesMetaData *pMetaData = (ItunesMetaData*)
                              MM_Malloc(sizeof(ItunesMetaData));

  if((pMetaData) && (ulFreeAtomSize) && (pucBufPtr))
  {
    uint32 ulAtomSize     = 0;
    uint32 ulAtomType     = 0;
    uint32 ulByteConsumed = 8; //Parent atom signature and size
    memset(pMetaData, 0, sizeof(ItunesMetaData));
    while(ulByteConsumed < ulFreeAtomSize)
    {
      //This variable is used to keep track of amount of data consumed
      uint32 ulAtomOffset = 0;
      uint8* pTempBuf     = pucBufPtr + ulByteConsumed;
      //! Get Atom size and type info
      (void)AtomUtils::read32read32(pTempBuf, ulAtomSize, ulAtomType);
      ulByteConsumed += DEFAULT_ATOM_SIZE;
      ulAtomOffset   += DEFAULT_ATOM_SIZE;

      //! Validate Atom size before using
      if ((!ulAtomSize) || (ulAtomSize >= ulFreeAtomSize))
      {
        if (pMetaData)
        {
          MM_Free(pMetaData);
          pMetaData = NULL;
        }
        break;
      }

      if (MEAN_TYPE == ulAtomType)
      {
        //Version and flags
        ulAtomOffset   += FOURCC_SIGNATURE_BYTES;
        ulByteConsumed += FOURCC_SIGNATURE_BYTES;
        /* Tool name used to generate Metadata. Currently it supports
           apple iTunes only. */
        if (memcmp(pucBufPtr + ulByteConsumed, "com.apple.iTunes",
                   sizeof("com.apple.iTunes")))
        {
          if(pMetaData)
          {
            MM_Free(pMetaData);
            pMetaData = NULL;
            break;
          }
        }
      }
      else if (NAME_TYPE == ulAtomType)
      {
        //Version and flags
        ulAtomOffset   += FOURCC_SIGNATURE_BYTES;
        ulByteConsumed += FOURCC_SIGNATURE_BYTES;
        if (!memcmp(pucBufPtr + ulByteConsumed, "iTunSMPB",
                    sizeof("iTunSMPB")))
        {
          pMetaData->eMetaType = FILE_SOURCE_MD_ENC_DELAY;
        }
      }
      else if (DATA_TYPE == ulAtomType)
      {
        /* typesReserved      - 16bits
           typesetIdentifier  - 8bits
           typecode           - 8bits
           locale             - 32bits
        */
        ulByteConsumed += 3; //First three bytes are not required
        pMetaData->eDataType = (ITUNES_DATA_TYPE)pucBufPtr[ulByteConsumed];
        ulByteConsumed++;
        ulAtomOffset += FOURCC_SIGNATURE_BYTES;

        copyByteSwapData((uint8*)&pMetaData->ulLocaleIndicator,
                         FOURCC_SIGNATURE_BYTES, pucBufPtr + ulByteConsumed, 1,
                         ulAtomSize);
        ulAtomOffset   += FOURCC_SIGNATURE_BYTES;
        ulByteConsumed += FOURCC_SIGNATURE_BYTES;
        uint8* pTagData = (uint8*)MM_Malloc(ulAtomSize);
        if (pTagData)
        {
          //Copy remaining data
          memcpy(pTagData, pucBufPtr + ulByteConsumed, ulAtomSize - ulAtomOffset);
          pTagData[ulAtomSize - ulAtomOffset] = '\0';
        }
        pMetaData->pucMetaData = pTagData;
      }
      ulByteConsumed += (ulAtomSize - ulAtomOffset);
    }
  }
  return pMetaData;
}

