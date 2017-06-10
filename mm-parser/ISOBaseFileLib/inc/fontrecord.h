#ifndef __FONT_RECORD_H__
#define __FONT_RECORD_H__
/* =======================================================================
                              fontrecord.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/fontrecord.h#7 $
$DateTime: 2011/11/24 00:50:52 $
$Change: 2061573 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "atomutils.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  FontRecord

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
class FontRecord
{

public:
  FontRecord (OSCL_FILE *fp); // Default constructor
  virtual ~FontRecord (); // Destructor

  uint16 getFontID()
  {
    return _fontID;
  }

  int8   getFontLength()
  {
    return _fontLength;
  }

  uint8* getFontName()
  {
    return _pFontName;
  }

  bool GetFileSuccess()
  {
    return _success;
  }

  PARSER_ERRORTYPE GetFileError()
  {
    return _fileErrorCode;
  }

private:
  uint16 _fontID;
  int8   _fontLength;
  uint8  *_pFontName;

  bool  _success;
  PARSER_ERRORTYPE _fileErrorCode;
};

#endif /* __FONT_RECORD_H__ */
