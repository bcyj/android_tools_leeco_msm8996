/* =======================================================================
                              sampleentry.cpp
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/sampleentry.cpp#9 $
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

#include "sampleentry.h"
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
  SampleEntry::SampleEntry

DESCRIPTION
  Stream-in Constructor

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
SampleEntry::SampleEntry(OSCL_FILE *fp)
: Atom(fp)
{
  for(int i=0;i< 6;i++)
  {
    _reserved[i] = 0;
  }

  _dataReferenceIndex = 0;

  if ( _success )
  {

    if ( !AtomUtils::read8read8(fp, _reserved[0], _reserved[1]) )
      _success = false;
    if ( !AtomUtils::read8read8(fp, _reserved[2], _reserved[3]) )
      _success = false;
    if ( !AtomUtils::read8read8(fp, _reserved[4], _reserved[5]) )
      _success = false;

    if ( !AtomUtils::read16(fp, _dataReferenceIndex) )
      _success = false;

    if ( !_success )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "SampleEntry::SampleEntry read failure");
      _fileErrorCode = PARSER_ErrorReadFail;
    }
  }
  else
  {
    _fileErrorCode = PARSER_ErrorReadFail;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "SampleEntry::SampleEntry _success is already false ");
  }
}

/* ======================================================================
FUNCTION
  SampleEntry::~SampleEntry

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
SampleEntry::~SampleEntry()
{
  // Empty
}

/* ======================================================================
FUNCTION
  GenericSampleEntry::GenericSampleEntry

DESCRIPTION
  Stream-in Constructor

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
GenericSampleEntry::GenericSampleEntry(OSCL_FILE *fp)
: Atom(fp)
{
  if ( _success )
  {
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "GenericSampleEntry _success is already false ");
    _fileErrorCode = PARSER_ErrorReadFail;
  }
}

/* ======================================================================
FUNCTION
  GenericSampleEntry::~GenericSampleEntry

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
GenericSampleEntry::~GenericSampleEntry()
{
  // Empty
}

