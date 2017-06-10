#ifndef __SampleEntry_H__
#define __SampleEntry_H__
/* =======================================================================
                              sampleentry.h
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/sampleentry.h#6 $
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

#include "oscl_file_io.h"
#include "atom.h"
#include "isucceedfail.h"

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
  SampleEntry

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
class SampleEntry : public Atom
{

public:
  SampleEntry(OSCL_FILE *fp); // Stream-in ctor
  virtual ~SampleEntry();

  // Member gets and sets
  uint16 getDataReferenceIndex() const
  {
    return _dataReferenceIndex;
  }

  virtual uint32 getESID() const
  {
    return 0;
  } // Should get overridden

  // Getting and setting the Mpeg4 VOL header
  virtual uint8 getObjectTypeIndication() const
  {
    return 0;
  }

protected:
  // Reserved constants
  uint8 _reserved[6];
  uint16 _dataReferenceIndex;

};

/* ======================================================================
CLASS
  GenericSampleEntry

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
class GenericSampleEntry : public Atom
{

public:
  GenericSampleEntry(OSCL_FILE *fp); // Stream-in ctor
  virtual ~GenericSampleEntry();

};

#endif
