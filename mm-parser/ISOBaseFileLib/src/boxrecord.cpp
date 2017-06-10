/* =======================================================================
                              boxrecord.cpp
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

Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/boxrecord.cpp#10 $
$DateTime: 2012/01/06 01:37:56 $
$Change: 2128420 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "boxrecord.h"
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
  BoxRecord::BoxRecord

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
BoxRecord::BoxRecord (OSCL_FILE *fp)
{
  _fileErrorCode = PARSER_ErrorNone;
  _success = true;
  _top = 0;
  _left = 0;
  _bottom = 0;
  _right = 0;
  uint16 tmp = 0;

  if ( !AtomUtils::read16( fp, tmp) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  else
  {
    _top = (int16)tmp;
  }


  if ( !AtomUtils::read16( fp, tmp) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  else
  {
    _left = (int16)tmp;
  }

  if ( !AtomUtils::read16( fp, tmp) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  else
  {
    _bottom = (int16)tmp;
  }

  if ( !AtomUtils::read16( fp, tmp) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  else
  {
    _right = (int16)tmp;
  }
}

/* ======================================================================
FUNCTION
  BoxRecord::BoxRecord

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
BoxRecord::BoxRecord (uint8 *&buf)
{
  _fileErrorCode = PARSER_ErrorNone;
  _success = true;

  uint16 tmp = 0;

  if ( !AtomUtils::read16( buf, tmp) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  else
  {
    _top = (int16)tmp;
  }


  if ( !AtomUtils::read16( buf, tmp) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  else
  {
    _left = (int16)tmp;
  }

  if ( !AtomUtils::read16( buf, tmp) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  else
  {
    _bottom = (int16)tmp;
  }

  if ( !AtomUtils::read16( buf, tmp) )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
  else
  {
    _right = (int16)tmp;
  }
}
