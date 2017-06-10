#ifndef __STYLE_RECORD_H__
#define __STYLE_RECORD_H__
/* =======================================================================
                              stylerecord.h
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/stylerecord.h#7 $
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
MACRO MYOBJ

ARGS
  xx_obj - this is the xx argument

DESCRIPTION:
  Complete description of what this macro does
========================================================================== */

/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  StyleRecord

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
class StyleRecord
{

public:
  StyleRecord (OSCL_FILE *fp); // Default constructor
  StyleRecord (uint8 *&buf); // Default constructor
  virtual ~StyleRecord (); // Destructor

  uint16 getStartChar() { return _startChar; }

  uint16 getEndChar() { return _endChar; }

  uint16 getFontID() { return _fontID; }

  uint8 getFontStyleFlags() { return _fontStyleFlags; }

  uint8 getfontSize() { return _fontSize; }

  uint8 *getTextColourRGBA () { return _pRGBA; }

  bool FileSuccess() { return _success; }

  PARSER_ERRORTYPE GetFileError() { return _fileErrorCode; }
private:
  uint16 _startChar;
  uint16 _endChar;
  uint16 _fontID;
  uint8  _fontStyleFlags;
  uint8  _fontSize;
  uint8 *_pRGBA;

  bool  _success;
  PARSER_ERRORTYPE _fileErrorCode;
};


#endif
