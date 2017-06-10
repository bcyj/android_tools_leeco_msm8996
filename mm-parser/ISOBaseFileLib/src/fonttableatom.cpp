/* =======================================================================
                              fonttableatom.cpp
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/fonttableatom.cpp#13 $
$DateTime: 2013/01/07 02:52:01 $
$Change: 3197629 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "fonttableatom.h"
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
/* ======================================================================
FUNCTION
  FontTableAtom::FontTableAtom

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
FontTableAtom::FontTableAtom (OSCL_FILE *fp)
: Atom(fp)
{
  _pFontRecordArray = NULL;
  _entryCount = 0;

  if ( _success )
  {
    (void)AtomUtils::read16( fp, _entryCount);

    int16 tmp = (int16)_entryCount;

    if ( tmp < 0 )
    {
      _success = false;
      _fileErrorCode = PARSER_ErrorReadFail;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
        "FontTableAtom::FontTableAtom _entryCount < 0");
      return;
    }

    _pFontRecordArray = MM_New( ZArray <FontRecord *> );

    for ( uint32 i = 0; i  <  _entryCount && _pFontRecordArray;  i++ )
    {
      FontRecord *rec = MM_New_Args( FontRecord, (fp) );

      /* In erroneous file scenarios, update __entrycount variable with number of mem allocations
         successfully done (i), which is used in destructor to free the memory allocated to
         _pFontRecordArray structure. By updating __entrycount parameter, parser will not crash
         by freeing the memory which is not allocated */
      if ( (rec) && !rec-> GetFileSuccess() )
      {
        MM_Delete( rec );
        _success = false;
        _entryCount    = (uint16)i;
        _fileErrorCode = PARSER_ErrorReadFail;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "FontTableAtom::FontTableAtom read failed");
        return;
      }
      if(_pFontRecordArray)
      {
        (*_pFontRecordArray) += rec;
      }
    }
  }
  else
  {
    _entryCount    = 0;
    _fileErrorCode = PARSER_ErrorReadFail;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "FontTableAtom::FontTableAtom _success is already false");
  }
}

/* ======================================================================
FUNCTION
  FontTableAtom::getFontRecordAt

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
FontRecord *
FontTableAtom::getFontRecordAt (int32 index)
{
  if ( index < (int32)_entryCount )
  {
    return(FontRecord *) (*_pFontRecordArray)[index];
  }
  else
  {
    return NULL;
  }
}

/* ======================================================================
FUNCTION
  FontTableAtom::~FontTableAtom

DESCRIPTION
  Desctuctor

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
FontTableAtom::~FontTableAtom()
{
  if(NULL == _pFontRecordArray)
  {
    return;
  }
  for ( uint32 i = 0; i < _entryCount; i++ )
  {
  if((*_pFontRecordArray) && (*_pFontRecordArray)[i] != NULL)
  {
    MM_Delete( (*_pFontRecordArray)[i] );
    (*_pFontRecordArray)[i] = NULL;
  }

  }

  MM_Delete( (_pFontRecordArray) );
  _pFontRecordArray = NULL;
}
