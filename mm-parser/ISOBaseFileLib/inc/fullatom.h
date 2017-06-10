#ifndef __FullAtom_H__
#define __FullAtom_H__
/* =======================================================================
                              fullatom.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2011-2012 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/fullatom.h#9 $
$DateTime: 2012/08/06 07:08:16 $
$Change: 2664350 $


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
// tag - 4; extended tag - 16; version - 1; flags - 3
#define DEFAULT_UUID_ATOM_SIZE 28
const uint32 DEFAULT_FULL_ATOM_SIZE = 12; // (8 bytes from Atom + 1 for
                                          //  version and 3 for flags)

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
  FullAtom

DESCRIPTION
  Thorough, meaningful description of what this function does

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class FullAtom : public Atom
{

public:
  FullAtom(uint32 type, uint8 version, uint32 flags); // Constructor
  FullAtom(OSCL_FILE *fp); // stream-in constructor
  FullAtom(uint8* &buf);
  virtual ~FullAtom();

  // No "set" methods as they get set directly in the constructor
  uint8  getVersion() const
  {
    return _version;
  }
  uint32 getFlags() const
  {
    return _flags;
  }
  uint32 getUUIDType() const
  {
    return _uuidType;
  }

#ifndef __CC_ARM
  inline
#endif
  virtual uint32 getDefaultSize() const
  {
    return DEFAULT_FULL_ATOM_SIZE;
  }



private:
  uint8 _version; // 1 (8bits)
  uint32 _flags; // 3 (24bits) -- Will need to crop when writing to stream
  uint32 _uuidType; // 32 bit Kddi UUID Type
  uint32 _uuidExt1; // 32 bits ext 1
  uint32 _uuidExt2; // 32 bits ext 2
  uint32 _uuidExt3; // 32 bits ext 3
};


#endif
