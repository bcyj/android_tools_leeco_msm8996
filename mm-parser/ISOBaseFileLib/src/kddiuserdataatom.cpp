/* =======================================================================
                              kddiuserdataatom.cpp
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

  Copyright (c) 2008-2013 Qualcomm Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/kddiuserdataatom.cpp#15 $
$DateTime: 2013/09/19 20:13:39 $
$Change: 4465453 $


========================================================================== */
#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "kddiuserdataatom.h"
#include "atomdefs.h"

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

/* ======================================================================
FUNCTION
  KDDIDrmAtom::KDDIDrmAtom

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
KDDIDrmAtom::KDDIDrmAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
    _copyGaurdAttribute = 0;
    _limitDate = 0;
    _limitPeriod = 0;
    _limitCount = 0;
  if ( _success )
  {
    if ( !AtomUtils::read32(fp, _copyGaurdAttribute) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }
    if ( !AtomUtils::read32(fp, _limitDate) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }
    if ( !AtomUtils::read32(fp, _limitPeriod) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }
    if ( !AtomUtils::read32(fp, _limitCount) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
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
  KDDIContentPropertyAtom::KDDIContentPropertyAtom

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
KDDIContentPropertyAtom::KDDIContentPropertyAtom(OSCL_FILE *fp) : FullAtom(fp)
{
   uint32 readSize = 0;
   _dllVersion = 0;
   if ( _success )
  {
    uint32 atomType, atomSize;
    uint8 tempBuf[256]; /* temp buffer to hold data */
    _dllVersion = 0;

    uint32 count = DEFAULT_ATOM_SIZE + 4 + 16;

    /* SKA: Since this is stored as another atom, first read its size and type
    */
    if ( !AtomUtils::read32read32(fp, atomSize, atomType) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }
    count += 8;

    while ( (count < _size) &&
            ((atomType == FourCharConstToUint32('t', 'i', 't', 'l')) ||
             (atomType == FourCharConstToUint32('r', 'g', 'h', 't')) ||
             (atomType == FourCharConstToUint32('a', 't', 'h', 'r')) ||
             (atomType == FourCharConstToUint32('m', 'e', 'm', 'o')) ||
             (atomType == FourCharConstToUint32('v', 'r', 's', 'n'))) )
    {
      if ( atomType == FourCharConstToUint32('t', 'i', 't', 'l') )
      {
        readSize = (uint8)FILESOURCE_MIN(atomSize-8, sizeof(tempBuf)-1);
        if ( !AtomUtils::readByteData(fp, readSize, tempBuf) ) /* 4 bytes for size and 4 bytes for type */
        {
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        tempBuf[readSize] = '\0';
        _title = (const OSCL_TCHAR *)tempBuf;
        count += (atomSize-8);
      }
      else if ( atomType == FourCharConstToUint32('r', 'g', 'h', 't') )
      {
        readSize = (uint8)FILESOURCE_MIN(atomSize-8, sizeof(tempBuf) -1);
        if ( !AtomUtils::readByteData(fp, readSize, tempBuf) ) /* 4 bytes for size and 4 bytes for type */
        {
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        tempBuf[readSize] = '\0';
        _copyRight = (const OSCL_TCHAR *)tempBuf;
        count += (atomSize-8);
      }
      else if ( atomType == FourCharConstToUint32('a', 't', 'h', 'r') )
      {
        readSize = (uint8)FILESOURCE_MIN(atomSize-8, sizeof(tempBuf)-1);
        if ( !AtomUtils::readByteData(fp, readSize, tempBuf) ) /* 4 bytes for size and 4 bytes for type */
        {
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        tempBuf[readSize] = '\0';
        _author = (const OSCL_TCHAR *)tempBuf;
        count += (atomSize-8);
      }
      else if ( atomType == FourCharConstToUint32('m', 'e', 'm', 'o') )
      {
        readSize = (uint8)FILESOURCE_MIN(atomSize-8, sizeof(tempBuf)-1);
        if ( !AtomUtils::readByteData(fp, readSize, tempBuf) ) /* 4 bytes for size and 4 bytes for type */
        {
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        tempBuf[readSize] = '\0';
        _memo = (const OSCL_TCHAR *)tempBuf;
        count += (atomSize-8);
      }
      else if ( atomType == FourCharConstToUint32('v', 'r', 's', 'n') )
      {
        if ( !AtomUtils::read32(fp, _dllVersion) )
        {
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        count += 4;
      }

      if ( count < _size )
      {
        /* get next atom info */
        if ( !AtomUtils::read32read32(fp, atomSize, atomType) )
        {
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        count += 8;
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
  KDDIMovieMailAtom::KDDIMovieMailAtom

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
KDDIMovieMailAtom::KDDIMovieMailAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _editFlags =0 ;
  _recordingMode = 0;
  _recDate =0 ;
  if ( _success )
  {
    uint32 temp;

    if ( !AtomUtils::read32(fp, temp) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }
    _editFlags     = (temp >> 8) & 0x00FFFFFF;
    _recordingMode = (uint8)(temp & 0xFF);

    if ( !AtomUtils::read32(fp, _recDate) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
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
  KDDIEncoderInformationAtom::KDDIEncoderInformationAtom

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
KDDIEncoderInformationAtom::KDDIEncoderInformationAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _deviceName         = NULL;
  _modelName          = NULL;
  _encoderInformation = NULL;
  _muxInformation     = NULL;

  if ( _success )
  {
    _deviceName         = (uint8*)MM_Malloc(8);
    _modelName          = (uint8*)MM_Malloc(8);
    _encoderInformation = (uint8*)MM_Malloc(8);
    _muxInformation     = (uint8*)MM_Malloc(8);

    if ( !AtomUtils::readByteData(fp, 8, _deviceName) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }
    if ( !AtomUtils::readByteData(fp, 8, _modelName) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }
    if ( !AtomUtils::readByteData(fp, 8, _encoderInformation) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }
    if ( !AtomUtils::readByteData(fp, 8, _muxInformation) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
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
  KDDIEncoderInformationAtom::~KDDIEncoderInformationAtom

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
KDDIEncoderInformationAtom::~KDDIEncoderInformationAtom()
{
  if ( _deviceName != NULL )
  {
    MM_Free(_deviceName);
  }
  if ( _modelName != NULL )
  {
    MM_Free(_modelName);
  }
  if ( _encoderInformation != NULL )
  {
    MM_Free(_encoderInformation);
  }
  if ( _muxInformation != NULL )
  {
    MM_Free(_muxInformation);
  }
}

/* ======================================================================
FUNCTION
  KDDITelopAtom::KDDITelopAtom

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
KDDITelopAtom::KDDITelopAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _pKddiTelopText = NULL;
  _telopSize      = 0;

  if ( _success )
  {
    _telopSize = _size - (DEFAULT_UUID_ATOM_SIZE);

    _pKddiTelopText = (uint8*)MM_Malloc(_telopSize);

    if ( !AtomUtils::readByteData(fp, _telopSize, _pKddiTelopText) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
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
  KDDITelopAtom::~KDDITelopAtom

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
KDDITelopAtom::~KDDITelopAtom()
{
  if ( _pKddiTelopText != NULL )
  {
    MM_Free(_pKddiTelopText);
  }
}

/* ======================================================================
FUNCTION
  KDDIGPSIFDEntry::KDDIGPSIFDEntry

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
KDDIGPSIFDEntry::KDDIGPSIFDEntry(OSCL_FILE *fp)
{
  _success = true;
   _tagID = 0;
  _typeValue = 0;
  _dataCount = 0;
  _value = 0;

  if ( !AtomUtils::read16(fp, _tagID) )
  {
    _success = false;
    return;
  }
  if ( !AtomUtils::read16(fp, _typeValue) )
  {
    _success = false;
    return;
  }
  if ( !AtomUtils::read32(fp, _dataCount) )
  {
    _success = false;
    return;
  }
  if ( !AtomUtils::read32(fp, _value) )
  {
    _success = false;
    return;
  }
}

/* ======================================================================
FUNCTION
  KDDIGPSExtensionAtom::KDDIGPSExtensionAtom

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
KDDIGPSExtensionAtom::KDDIGPSExtensionAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _mapScaleInfo = 0 ;
  if ( _success )
  {
    if ( !AtomUtils::read64(fp, _mapScaleInfo) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
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
  KDDIGPSAtom::KDDIGPSAtom

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
KDDIGPSAtom::KDDIGPSAtom(OSCL_FILE *fp)
: FullAtom(fp)
{
  _kddiGPSExtensionAtom = NULL;
  _gpsIFDEntryVec = MM_New( ZArray<KDDIGPSIFDEntry*> );
  _byteOrder =0 ;
  _numIFD = 0;
  _versionID = 0;
  _latitudeRef = 0;
  memset(&_gpsLatitude,0,QTV_MAX_GPS_LATITUDE);
  memset(&_gpsLongitude,0,QTV_MAX_GPS_LONGITUDE);
  _longitudeRef = 0;
  _altitudeRef = 0;
  _gpsAltitude = 0;
  _nextIFDOffset = 0;


  uint32 i, j, type, atomSize, versionAndFlags, startOfAtom, startOfData;
  KDDIGPSIFDEntry *ifdElement;


  startOfAtom = (uint32)OSCL_FileTell(fp);

  /* Since this is stored as another atom, first read its size, type and flags
  */
  if ( !AtomUtils::read32read32(fp, atomSize, type) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }

  if ( !AtomUtils::read32(fp, versionAndFlags) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }

  startOfData = (uint32)OSCL_FileTell(fp);

  if ( !AtomUtils::read16(fp, _byteOrder) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }

  if ( !AtomUtils::read16(fp, _numIFD) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }

  /* read all the IFD entries */
  for ( i = 0; i < _numIFD; i++ )
  {
    ifdElement = MM_New_Args( KDDIGPSIFDEntry, (fp) );
    if(!ifdElement)
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }

    if ( ifdElement->getSuccess() && _gpsIFDEntryVec)
    {
      (*_gpsIFDEntryVec) += ifdElement;
    }
    else
    {
      _success = false;
      if(ifdElement)
      {
        MM_Delete(ifdElement);
      }
      _fileErrorCode = PARSER_ErrorReadFail;
      return;
    }
  }

  if ( !AtomUtils::read32(fp, _nextIFDOffset) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }

  /* get data from all the IFD entries */
  uint32 ifdValue = 0;
  uint32 tagID = 0;
  uint32 dataCount = 0;
  for ( i = 0; i < _numIFD; i++ )
  {
    ifdElement  = getIFDEntryAt((int32)i);
    if(ifdElement != NULL)
    {
      ifdValue    = ifdElement->getValue();
      tagID       = ifdElement->getTagID();
      dataCount   = ifdElement->gatDataCount();

      switch ( tagID )
      {
      case KDDI_GPS_IFD_TAG_VERSION:
        _versionID = ifdValue;
        break;

      case KDDI_GPS_IFD_TAG_LATITUDE_REF:
        _latitudeRef = ifdValue;
        break;

      case KDDI_GPS_IFD_TAG_LATITUDE:
        /* ifdVakue is offset for data */
        (void)OSCL_FileSeek(fp, (startOfData+ifdValue), SEEK_SET);
        for ( j = 0; j < QTV_MAX_GPS_LATITUDE; j++ )
        {
          if ( !AtomUtils::read64(fp, _gpsLatitude[j]) )
          {
            _success = false;
            _fileErrorCode = PARSER_ErrorReadFail;
            return;
          }
        }
        break;

      case KDDI_GPS_IFD_TAG_LONGITUDE_REF:
        _longitudeRef = ifdValue;
        break;

      case KDDI_GPS_IFD_TAG_LONGITUDE:
        /* ifdVakue is offset for data */
        (void)OSCL_FileSeek(fp, (startOfData+ifdValue), SEEK_SET);
        for ( j = 0; j < QTV_MAX_GPS_LONGITUDE; j++ )
        {
          if ( !AtomUtils::read64(fp, _gpsLongitude[j]) )
          {
            _success = false;
            _fileErrorCode = PARSER_ErrorReadFail;
            return;
          }
        }
        break;

      case KDDI_GPS_IFD_TAG_ALTITUDE_REF:
        _altitudeRef = ifdValue;
        break;

      case KDDI_GPS_IFD_TAG_ALTITUDE:
        /* ifdVakue is offset for data */
        (void)OSCL_FileSeek(fp, (startOfData+ifdValue), SEEK_SET);
        if ( !AtomUtils::read64(fp, _gpsAltitude) )
        {
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        break;

      case KDDI_GPS_IFD_TAG_TIMESTAMP:
        /* ifdVakue is offset for data */
        (void)OSCL_FileSeek(fp, (startOfData+ifdValue), SEEK_SET);
        for ( j = 0; j < QTV_MAX_GPS_TIME; j++ )
        {
          if ( !AtomUtils::read64(fp, _gpsTime[j]) )
          {
            _success = false;
            _fileErrorCode = PARSER_ErrorReadFail;
            return;
          }
        }
        break;

      case KDDI_GPS_IFD_TAG_GEODETIC_SRV:
       /* ifdVakue is offset for data */
        (void)OSCL_FileSeek(fp, (startOfData+ifdValue), SEEK_SET);
        if ( !AtomUtils::readNullTerminatedString(fp, _gpsGeodeticSurveyData) )
        {
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        break;

      case KDDI_GPS_IFD_TAG_POS_SYSTEM:
        {
          uint8 * tempBuf = (uint8*)MM_Malloc(256);
          if (!tempBuf)
          {
            _success = false;
            _fileErrorCode = PARSER_ErrorMemAllocFail;
            return;
          }
          /* ifdVakue is offset for data */
          (void)OSCL_FileSeek(fp, (startOfData+ifdValue), SEEK_SET);
          if ( !AtomUtils::readByteData(fp, dataCount, tempBuf) )
          {
            _success = false;
            _fileErrorCode = PARSER_ErrorReadFail;
            MM_Free(tempBuf);
            return;
          }
          tempBuf[dataCount] = '\0';
          FILESOURCE_STRING tempStr((const OSCL_TCHAR *)(tempBuf+8));
         _positioningMethod = tempStr;
          MM_Free(tempBuf);
          break;
       }

       case KDDI_GPS_IFD_TAG_POS_POINT_NAME:
        {
          uint8 * tempBuf = (uint8*)MM_Malloc(256);
          if (!tempBuf)
          {
            _success = false;
            _fileErrorCode = PARSER_ErrorMemAllocFail;
            return;
          }
          /* ifdVakue is offset for data */
          (void)OSCL_FileSeek(fp, (startOfData+ifdValue), SEEK_SET);
          if ( !AtomUtils::readByteData(fp, dataCount, tempBuf) )
          {
            _success = false;
            _fileErrorCode = PARSER_ErrorReadFail;
            MM_Free(tempBuf);
            return;
          }
          tempBuf[dataCount] = '\0';
          FILESOURCE_STRING tempStr((const OSCL_TCHAR *)(tempBuf+8));
          _positioningMethod = tempStr;
          MM_Free(tempBuf);
          break;
        }

      case KDDI_GPS_IFD_TAG_DATESTAMP:
        /* ifdVakue is offset for data */
        (void)OSCL_FileSeek(fp, (startOfData+ifdValue), SEEK_SET);
        if ( !AtomUtils::readNullTerminatedString(fp, _gpsDate) )
        {
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          return;
        }
        break;

      default:
        break;
      }
    }
  } /* end of for(num of IFDs) */

  /* set the file pointer to end of GPS Atom */
  (void)OSCL_FileSeek(fp, (startOfAtom+atomSize), SEEK_SET);

  if ( AtomUtils::getNextAtomType(fp) == KDDI_GPS_EXTENSION_ATOM )
  {
    _kddiGPSExtensionAtom = MM_New_Args( KDDIGPSExtensionAtom, (fp) );
  }
}

/* ======================================================================
FUNCTION
  KDDIGPSAtom::~KDDIGPSAtom

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
KDDIGPSAtom::~KDDIGPSAtom()
{
  if ( _kddiGPSExtensionAtom != NULL )
  {
    MM_Delete( _kddiGPSExtensionAtom );
    _kddiGPSExtensionAtom = NULL;
  }

  for ( uint32 i = 0; _gpsIFDEntryVec && i < _gpsIFDEntryVec->GetLength(); i++)
  {
    MM_Delete( (*_gpsIFDEntryVec)[i] );
    (*_gpsIFDEntryVec)[i] = NULL;
  }

  MM_Delete( (_gpsIFDEntryVec) );
  _gpsIFDEntryVec = NULL;
}
/* ======================================================================
FUNCTION
  KDDIGPSAtom::getIFDEntryAt

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
KDDIGPSIFDEntry *KDDIGPSAtom::getIFDEntryAt(int32 index)
{
  if ( (_gpsIFDEntryVec->GetLength() == 0) ||
       (index > ((int32)_gpsIFDEntryVec->GetLength())) )
  {
    return NULL;
  }
  else
  {
    return(*_gpsIFDEntryVec)[index];
  }
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
