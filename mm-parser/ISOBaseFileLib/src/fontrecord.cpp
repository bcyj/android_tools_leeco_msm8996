/* =======================================================================
                              fontrecord.cpp
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/fontrecord.cpp#10 $
$DateTime: 2013/01/02 01:46:45 $
$Change: 3185089 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "fontrecord.h"
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
  FontRecord::FontRecord

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
FontRecord::FontRecord (OSCL_FILE *fp)
{
  uint8 data;
  static bool return_code;

  _fileErrorCode = PARSER_ErrorNone;
  _success = true;

  (void)AtomUtils::read16(fp, _fontID);
  (void)AtomUtils::read8(fp, data);

  _fontLength = (int8)data;

  _pFontName = NULL;

  if ( _fontLength >  0 )
  {
    _pFontName = (uint8*)MM_Malloc(_fontLength+1);
    if (!_pFontName)
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorMemAllocFail;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "FontRecord::FontRecord memory allocation failed");
      return;
    }
  }
  else
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FontRecord::FontRecord fontlength <= 0");
    return;
  }

  return_code =AtomUtils::readByteData(fp, _fontLength, _pFontName);
  if ( return_code == false )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FontRecord::FontRecord read failed");
  }
  else
  {
    _pFontName[_fontLength] = '\0';   /* make it NULL terminated string */
    _success = true;
  }
}

/* ======================================================================
FUNCTION
  FontRecord::~FontRecord

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
FontRecord::~FontRecord()
{
  if ( _pFontName != NULL )
  {
    MM_Free(_pFontName);
  }
}
