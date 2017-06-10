/* =======================================================================
                              atom.cpp
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/atom.cpp#14 $
$DateTime: 2013/02/27 22:52:58 $
$Change: 3415293 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "atom.h"
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
  Atom::Atom

DESCRIPTION
  Normal constructor from fullatom constructor

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
Atom::Atom(uint32 type)
{
    _type = type;
    _pparent = NULL;
    _size=0;/*size is intialized */
    _success=true; /*success is intialized */
    _fileErrorCode = PARSER_ErrorNone; /*_filerrorcode is intialized*/
    _offsetInFile = 0;
}


/* ======================================================================
FUNCTION
  Atom::Atom

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
Atom::Atom(OSCL_FILE *fp)
{
  uint64 atomendPointer; // file pointer at the end of the current atom

  _success = true;
  _fileErrorCode = PARSER_ErrorNone;
   _pparent = NULL; /*_pparent is intialized */
   _size=0; /*size is intialized */
   _type=0;
  // SAVE THE CURRENT FILE POSITION
   bool bError = true;
  _offsetInFile = OSCL_FileTell(fp,&bError);

  if ( bError )
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorDefault;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "Atom::Atom _success is false due to filetell error");
  }

  if( _success && !AtomUtils::read32read32(fp, _size, _type))
  {
    _success = false;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "Atom::Atom _success is false due to read fail");
  }

  int32 size = (int32)_size;

  if (size <= DEFAULT_ATOM_SIZE)
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "Atom::Atom _success is false due to size < DEFAULT_ATOM_SIZE");
  }

  if (_success)
  {
    /* atom size must atleast be DEFAULT_ATOM_SIZE */
    if (size >= DEFAULT_ATOM_SIZE )
    {
      /*
       * Seek to the end of the atom, as size + tag field is included in
       * the atom Size, and as they have already been read, we subtract
       * (DEFAULT_ATOM_SIZE), from _size.
       */
      atomendPointer = _offsetInFile + ((uint32)size - DEFAULT_ATOM_SIZE);

      /* In case the fp is greater than file size, _size is corrupt */
      if (atomendPointer > AtomUtils::fileSize)
      {
        _success = false;
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
              "Atom(fp) _success is false due to offset %llu > fileSize %llu",
              atomendPointer, AtomUtils::fileSize);
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
          "Atom(fp) failed atom type %x, size %lu", (unsigned int)_type,_size);
      }
    }
  }
}

/* ======================================================================
FUNCTION
  Atom::Atom

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
Atom::Atom(uint8 *&buf)
{
  _success = true;
  _fileErrorCode = PARSER_ErrorNone;
   _pparent = NULL;/*_pparent is intialized*/
  _offsetInFile = 0;
  if(!AtomUtils::read32read32(buf, _size, _type))
  {
    _success = false;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Atom: _success is false due to read fail");
  }

  int32 size = (int32)_size;

  if (size <= DEFAULT_ATOM_SIZE)
  {
    _success = false;
    _fileErrorCode = PARSER_ErrorReadFail;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "Atom: _success is false due to size < DEFAULT_ATOM_SIZE");
  }

}
