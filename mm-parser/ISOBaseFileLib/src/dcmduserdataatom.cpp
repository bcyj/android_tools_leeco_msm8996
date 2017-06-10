/* =======================================================================
                              dcmdUserDataAtom.cpp
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

  Copyright (c) 2008-2013 Qualcomm Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/dcmduserdataatom.cpp#14 $
$DateTime: 2013/09/19 20:13:39 $
$Change: 4465453 $


========================================================================== */
#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include "AEEStdDef.h"              /* Definitions for byte, word, etc.        */

#include "dcmduserdataatom.h"
#include "MMMalloc.h"

/* ======================================================================
FUNCTION
  DcmdDrmAtom::DcmdDrmAtom

DESCRIPTION
  DcmdDrmAtom atom: dcmd constructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
DcmdDrmAtom::DcmdDrmAtom(OSCL_FILE *fp)
: Atom(fp)
{
  _dcmdData = NULL;
  _dcmdDataSize = 0;

  if ( _success )
  {

    /* skip 4 bytes size, 4 bytes type to get ftyp data size */
    _dcmdDataSize = Atom::getSize() - 8;
    if(_dcmdDataSize)
    {
      _dcmdData = (uint8*)MM_Malloc(_dcmdDataSize); /* free in destructure */
      if(_dcmdData)
      {
        if ( !AtomUtils::readByteData(fp, _dcmdDataSize, _dcmdData) )
        {
          _dcmdDataSize = 0;
          _success = false;
          _fileErrorCode = PARSER_ErrorReadFail;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "DcmdDrmAtom::DcmdDrmAtomMemory Read failed.");
          return;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "DcmdDrmAtom::DcmdDrmAtom Memory allocation failed.");
        _dcmdDataSize = 0;
        _success = false;
        _fileErrorCode = PARSER_ErrorMemAllocFail;
        return;
      }
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "DcmdDrmAtom::DcmdDrmAtom _success is already false.");
    _fileErrorCode = PARSER_ErrorReadFail;
    return;
  }
}

/* ======================================================================
FUNCTION
  DcmdDrmAtom::~DcmdDrmAtom

DESCRIPTION
  DcmdDrmAtom atom: dcmd destructor.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
DcmdDrmAtom::~DcmdDrmAtom()
{
  if(_dcmdData)
    MM_Free(_dcmdData);
}

/* ======================================================================
FUNCTION:
  DcmdDrmAtom::getDcmdDataSize

DESCRIPTION:
  returns dcmd data size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  size of dcmd data

SIDE EFFECTS:
  None.
======================================================================*/
uint32 DcmdDrmAtom::getDcmdDataSize()
{
  return _dcmdDataSize;
}

/* ======================================================================
FUNCTION:
  DcmdDrmAtom::getDcmdData

DESCRIPTION:
  copies the dcmd data into supplied buffer.

INPUT/OUTPUT PARAMETERS:
  pBuf      - INPUT/OUTPUT  - buffer for data to be copied.
  dwSize    - INPUT         - size of buffer and max data to be copied.
  dwOffset  - INPUT         - 0 based offset of data to be copied from start

RETURN VALUE:
  actual bytes copied into buffer

SIDE EFFECTS:
  None.
======================================================================*/
uint32 DcmdDrmAtom::getDcmdData(uint8* pBuf, uint32 dwSize, uint32 dwOffset)
{
  if(_dcmdDataSize)
  {
    uint32 dwDataToBeCopied = FILESOURCE_MIN(dwSize, _dcmdDataSize-dwOffset);
    memcpy(pBuf, _dcmdData+dwOffset, dwDataToBeCopied);
    return dwDataToBeCopied;
  }
  return 0;
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
