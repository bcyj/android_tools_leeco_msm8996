/* =======================================================================
                              fullatom.cpp
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/fullatom.cpp#12 $
$DateTime: 2013/08/24 08:24:59 $
$Change: 4329860 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */


/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "fullatom.h"
#include "atomutils.h"

#include "atomdefs.h"

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
  FullAtom::FullAtom

DESCRIPTION
  Constructor

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
FullAtom::FullAtom(uint32 type, uint8 version, uint32 flags)
: Atom(type)
{
  _version = version;
  _flags = flags;
  _uuidType=0;
  _uuidExt1=0;
  _uuidExt2=0;
  _uuidExt3=0;
}

/* ======================================================================
FUNCTION
  FullAtom::FullAtom

DESCRIPTION
  Constructor

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
FullAtom::FullAtom(OSCL_FILE *fp)
: Atom(fp)
{
  _version  = 0;
  _flags    = 0;
  _uuidExt1 = 0;
  _uuidExt2 = 0;
  _uuidExt3 = 0;
  _uuidType = UNKNOWN_ATOM;

  if ( _success )
  {
    // Size and type set in Atom constructor
    uint32 data;

    // Read in the UUID extension if present
    if ( getType() == UUID_ATOM )
    {
      if ( !AtomUtils::read32(fp, _uuidType) )
        _success = false;
      if ( !AtomUtils::read32(fp, _uuidExt1) )
        _success = false;
      if ( !AtomUtils::read32(fp, _uuidExt2) )
        _success = false;
      if ( !AtomUtils::read32(fp, _uuidExt3) )
        _success = false;
    }

    //! Update seek pointer to end of signature
    (void)OSCL_FileSeek(fp, _offsetInFile + DEFAULT_ATOM_SIZE, SEEK_SET);

    if ( !AtomUtils::read32(fp, data) )
      _success = false;
    if (_success == false)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FullAtom::FullAtom(fp) read failure");
    }

    _version = (uint8)(data >> 24);
    _flags   = data & 0x00ffffff;
  }
}
/* ======================================================================
FUNCTION
  FullAtom::FullAtom

DESCRIPTION
  Constructor

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

========================================================================== */
FullAtom::FullAtom(uint8 *&buf): Atom(buf)
{

  _version  = 0;
  _flags    = 0;
  _uuidExt1 = 0;
  _uuidExt2 = 0;
  _uuidExt3 = 0;
  _uuidType = UNKNOWN_ATOM;

  if ( _success )
  {
    // Size and type set in Atom constructor
    uint32 data;

    // Read in the UUID extension if present
    if ( getType() == UUID_ATOM )
    {
      if ( !AtomUtils::read32(buf, _uuidType) )
        _success = false;
      if ( !AtomUtils::read32(buf, _uuidExt1) )
        _success = false;
      if ( !AtomUtils::read32(buf, _uuidExt2) )
        _success = false;
      if ( !AtomUtils::read32(buf, _uuidExt3) )
        _success = false;
    }

    if ( !AtomUtils::read32(buf, data) )
      _success = false;
    if (_success == false)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FullAtom::FullAtom(buf) read failure");
    }

    _version = (uint8)(data >> 24);
    _flags   = data & 0x00ffffff;
  }
}

/* ======================================================================
FUNCTION
  FullAtom::~FullAtom

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
FullAtom::~FullAtom()
{
  // Empty
}


