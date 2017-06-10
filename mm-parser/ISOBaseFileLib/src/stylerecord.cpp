/* =======================================================================
                              stylerecord.cpp
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/stylerecord.cpp#12 $
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
#include "stylerecord.h"
#include "atomutils.h"
#include "isucceedfail.h"

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
  StyleRecord:: StyleRecord

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
StyleRecord::StyleRecord (OSCL_FILE *fp)
{
  _success = true;
  _fileErrorCode = PARSER_ErrorNone;
  _startChar = 0;
  _endChar = 0;
  _fontID = 0;
  _fontStyleFlags = 0;
  _fontSize = 0;
  _pRGBA = NULL;

  _pRGBA = (uint8*)MM_Malloc(4);
  if ( !_pRGBA )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorMemAllocFail;
    return;
  }

  if ( !AtomUtils::read16( fp, _startChar) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::read16( fp, _endChar) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::read16( fp, _fontID) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::read8( fp, _fontStyleFlags) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::read8( fp, _fontSize) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::readByteData(fp, 4, _pRGBA) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if (_success == false)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "StyleRecord::StyleRecord read failure");
  }
}

/* ======================================================================
FUNCTION
  StyleRecord:: StyleRecord

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
StyleRecord:: StyleRecord (uint8 *&buf)
{
  _endChar = 0;
  _fontID = 0;
  _fontSize = 0;
  _fontStyleFlags = 0;
  _startChar = 0;
  _success = true;
  _fileErrorCode = PARSER_ErrorNone;

  _pRGBA = (uint8*)MM_Malloc(4);
  if ( !_pRGBA )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorMemAllocFail;
    return;
  }

  if ( !AtomUtils::read16( buf, _startChar) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::read16( buf, _endChar) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::read16( buf, _fontID) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::read8( buf, _fontStyleFlags) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::read8( buf, _fontSize) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }

  if ( !AtomUtils::readByteData(buf, 4, _pRGBA) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
  }
  if(_success == false)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "StyleRecord::StyleRecord (buf) read failure");
  }
}

/* ======================================================================
FUNCTION
  StyleRecord::~StyleRecord

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
StyleRecord::~StyleRecord()
{
  if ( _pRGBA != NULL )
  {
    MM_Free(_pRGBA);
  }
}
