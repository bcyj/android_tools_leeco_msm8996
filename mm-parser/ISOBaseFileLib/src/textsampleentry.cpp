/* =======================================================================
                               textsampleentry.cpp
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

  Copyright (c) 2008-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/textsampleentry.cpp#12 $
$DateTime: 2013/01/02 01:46:45 $
$Change: 3185089 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "textsampleentry.h"
#include "atomutils.h"
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
  TextSampleEntry:: TextSampleEntry

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
TextSampleEntry:: TextSampleEntry (OSCL_FILE *fp, DataT eDataType) : SampleEntry (fp)
{
  _pparent           = NULL;
  _pBackgroundRGBA   = NULL;
  _pBoxRecord        = NULL;
  _pStyleRecord      = NULL;
  _pFontTableAtom    = NULL;
  _displayFlags      = 0;
  _horzJustification = 0;
  _vertJustification = 0;


  if( ( _success )&& ( DATA_ATOM_STPP == eDataType ) )
  {
    //Process stpp information: get name space information
    if ( !AtomUtils::readNullTerminatedString( fp, szNameSpace) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
    }
    // get schema_location information
    if ( _success && !AtomUtils::readNullTerminatedString( fp, szSchemaName) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
    }
    // get mime_type information
    if ( _success && !AtomUtils::readNullTerminatedString( fp, szMimeType) )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
    }
    if(false == _success)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "TextSampleEntry: read failure");
      return;
    }
  }
  else if( ( _success ) && ( DATA_ATOM_TX3G == eDataType ) )
  {
    _pBackgroundRGBA = (uint8*)MM_Malloc(PVTEXT_MAX_TEXT_COLORS);
    if ( !_pBackgroundRGBA )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorMemAllocFail;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "TextSampleEntry malloc fail in _pBackgroundRGBA");
      return;
    }
    (void)AtomUtils::read32( fp, _displayFlags);
    (void)AtomUtils::read8( fp,  _horzJustification);
    (void)AtomUtils::read8( fp,  _vertJustification);
    (void)AtomUtils::readByteData(fp, 4, _pBackgroundRGBA);

    _success = false;

    _pBoxRecord = MM_New_Args( BoxRecord , (fp) );
    if (!_pBoxRecord)
    {
      _fileErrorCode = PARSER_ErrorDefault;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "TextSampleEntry:: TextSampleEntry _pBoxRecord is NULL");
      return;
    }
    else if (!_pBoxRecord->FileSuccess())
    {
      _success = false;
      _fileErrorCode = _pBoxRecord->GetFileError();
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
        "TextSampleEntry:: TextSampleEntry _pBoxRecord is corrupted");
      return;
    }

    _pStyleRecord = MM_New_Args( StyleRecord , (fp) );
    if (!_pStyleRecord)
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorDefault;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "TextSampleEntry malloc failure for _pStyleRecord");
      return;
    }
    else if (!_pStyleRecord->FileSuccess())
    {
      _fileErrorCode = _pStyleRecord->GetFileError();
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
        "TextSampleEntry:: TextSampleEntry _pStyleRecord is corrupted");
      return;
    }

    _pFontTableAtom = MM_New_Args( FontTableAtom , (fp) );
    if (!_pFontTableAtom)
    {
      _fileErrorCode = PARSER_ErrorDefault;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "TextSampleEntry malloc failure for _pFontTableAtom");
      return;
    }
    else if (!_pFontTableAtom->FileSuccess())
    {
      _fileErrorCode = _pFontTableAtom->GetFileError();
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
        "TextSampleEntry:: TextSampleEntry _pFontTableAtom is corrupted");
      return;
    }
  }//if(DATA_TYPE_TX3G)
  _success = true;
}

/* ======================================================================
FUNCTION
  TextSampleEntry::~TextSampleEntry

DESCRIPTION
  Destructor

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TextSampleEntry::~TextSampleEntry()
{
  if (_pBackgroundRGBA != NULL)
  {
    MM_Free(_pBackgroundRGBA);
    _pBackgroundRGBA = NULL;
  }
  if (_pBoxRecord != NULL)
  {
    MM_Delete( _pBoxRecord );
    _pBoxRecord = NULL;
  }
  if (_pStyleRecord != NULL)
  {
    MM_Delete( _pStyleRecord );
    _pStyleRecord = NULL;
  }
  if (_pFontTableAtom != NULL)
  {
    MM_Delete( _pFontTableAtom );
    _pFontTableAtom = NULL;
  }
}
