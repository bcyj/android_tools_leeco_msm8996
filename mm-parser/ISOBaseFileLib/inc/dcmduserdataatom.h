#ifndef __DCMDUserDataAtom_H__
#define __DCMDUserDataAtom_H__
/* =======================================================================
                              dcmdUserDataAtom.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2011-2013 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/dcmduserdataatom.h#6 $
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

#include "atom.h"
#include "atomdefs.h"
#include "isucceedfail.h"
#include "atomutils.h"
#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  DcmdDrmAtom

DESCRIPTION
  DcmdDrmAtom atom: dcmd.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
class DcmdDrmAtom : public Atom
{

public:
    DcmdDrmAtom(OSCL_FILE *fp); // Constructor
    virtual ~DcmdDrmAtom();

    uint32 getDcmdDataSize();
    uint32 getDcmdData(uint8* pBuf, uint32 dwSize, uint32 offset);

private:
    uint32  _dcmdDataSize;
    uint8 * _dcmdData;
};
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
#endif //__DCMDUserDataAtom_H__
