#ifndef __pdcfAtoms_H__
#define __pdcfAtoms_H__
/* =======================================================================
                              pdcfatoms.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2008-2013 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/pdcfatoms.h#10 $


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

#include "isucceedfail.h"
#include "oscl_file_io.h"

#ifdef FEATURE_MP4_CUSTOM_META_DATA
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
/* This is to maintain the hash table
   Right now we have 8 different PDCF atoms
*/
#define MPEG4_MAX_PDCF_ATOMS 8
#define ATOM_HEADER_SIZE 8
/* =======================================================================

========================================================================== */

/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/* ======================================================================
CLASS
  PdcfAtom

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
class PdcfAtom
{
public:
  PdcfAtom(); // Constructor
  uint32 getAtomSize(DataT dType, uint32 track_id);
  uint32 getAtomData(OSCL_FILE *fp, DataT dType,uint8 *pBuf, uint32 size, uint64 offset);
  uint32 getAtomOffset(DataT dType,uint8 *pBuf, uint32 size, uint32 offset);
  bool saveAtom(DataT dType,DataT dType_offset, uint32 track_id, uint32 size, uint64 offset);
  bool isAtomPresent(DataT dType, uint32 track_id );

private:
  /* This is to hold the each PDCF atom */
  typedef struct
  {
   uint32 track_id;
   DataT type;
   DataT offset_type;
   uint64 offset;
   uint32 size;
  }Atom_type;

  /* Array to hold the PDCF atoms */
  Atom_type  m_AtomData[MPEG4_MAX_PDCF_ATOMS];
  /* offset to m_AtomData array */
  uint8 m_OffsetinAtomArray;
};
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
#endif /* __pdcfAtoms_H__ */

