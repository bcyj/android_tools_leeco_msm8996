/* =======================================================================
                              ztl.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/src/ztl.cpp#6 $
$DateTime: 2011/03/28 16:40:17 $
$Change: 1675367 $

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

//#include "defs.h"


#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#include <stdio.h>
#include <string.h>
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "ztl.h"
#include "MMDebugMsg.h"

#ifndef INT_MAX
#define INT_MAX       2147483647
#endif

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
FUNCTION:
  ZArrayBase :: Stress

DESCRIPTION:
  Make sure we have room for nSize + 1 elements

INPUT/OUPUT PARAMETERS:
  nSize

RETURN VALUE:
  m_nLength or -1 in case of error

SIDE EFFECTS:
  None.
========================================================================== */
int32 ZArrayBase :: Stress (int32 nSize)
{
	nSize++;
	if(MakeRoomFor(nSize)== -1)
	{
		return -1;
	}
	if (m_nLength < nSize)
	  m_nLength = nSize;//number of elements
	  return m_nLength;
}

/* ======================================================================
FUNCTION:
  ZArrayBase :: MakeRoomAt

DESCRIPTION:
  Make room for new element at position i

INPUT/OUPUT PARAMETERS:
  i

RETURN VALUE:
m_nLength or -1 in case of error

SIDE EFFECTS:
  None.
========================================================================== */
int32 ZArrayBase :: MakeRoomAt (int32 i)
{
  if(i < 0)  return(-1);

  if((Stress (m_nLength)) == -1)
  {
    return -1;
  }
  if (m_nLength > i + 1)
    (void)memmove (
      ((int8 *) m_pBuffer) + (i + 1) * m_nElementSize,
      ((int8 *) m_pBuffer) + i * m_nElementSize,
      (m_nLength - i - 1) * m_nElementSize
      );
  if( i < m_nLength )
  {
    memset (((int8 *) m_pBuffer) + i * m_nElementSize, 0, m_nElementSize);
    return m_nLength;
  }
  return -1;
}

/* ======================================================================
FUNCTION:
  ZArrayBase :: DeleteFrom

DESCRIPTION:
  Delete element at position i

INPUT/OUPUT PARAMETERS:
  i

RETURN VALUE:
  m_nLength or -1 in case of error

SIDE EFFECTS:
  None.
========================================================================== */
int32 ZArrayBase :: DeleteFrom (int32 i)
{
  if( (i < 0) || (m_nLength < (i+1)) ) return -1;

  if (m_nLength > i + 1)
   (void) memmove (
      ((int8 *) m_pBuffer) + i * m_nElementSize,
      ((int8 *) m_pBuffer) + (i + 1) * m_nElementSize,
      (m_nLength - i - 1) * m_nElementSize
      );
  --m_nLength;
  return m_nLength;
}

/* ======================================================================
FUNCTION:
  ZArrayBase :: MakeRoomFor

DESCRIPTION:
Make room for i number of elements for the fragmentInfoArray initially through malloc.
This would avoid numerous realloc of fragmentInfoArray during the playback of a large fragmented file
that may lead to heap fragmentation

INPUT/OUPUT PARAMETERS:


RETURN VALUE:
-1 in case of error

SIDE EFFECTS:
  None.
========================================================================== */
   int32 ZArrayBase::MakeRoomFor(int32 i )
   {

	  if(i < 0)  return(-1);

	  void *temp_m_pBuffer;

	  if((i< 0) || (i >= ( INT_MAX /m_nElementSize - 1 ) ) ) return -1;
	  int32 nNewSize = i * m_nElementSize;

	  /*-----------------------------------------------------------------
		If necessary, extend the buffer by 25% or GROW_BY, whatever is larger,
		but not less than what we need.
		-----------------------------------------------------------------*/
	  if (nNewSize > m_nSize)
	  {
		if( (m_nSize >= ( INT_MAX  - ( MAX(GROW_BY, (m_nSize / 4) ) ) ) )) return -1;
		nNewSize =  MAX(nNewSize, (m_nSize + (MAX (GROW_BY, (m_nSize / 4)))));
		if (m_pBuffer == NULL)
		{
		  m_pBuffer = (char*)MM_Malloc(nNewSize);
		  if (m_pBuffer == NULL)
		  {
			m_nLength = 0;
			m_nSize = 0;
			return -1;
		  }
		}
		else
		{
		  /* perfrm a realloc */
		  temp_m_pBuffer = (char*)MM_Malloc(nNewSize);
		  if (temp_m_pBuffer)
		  {
			memcpy(temp_m_pBuffer, m_pBuffer, m_nSize);
			MM_Free(m_pBuffer);
			m_pBuffer = temp_m_pBuffer;
		  }
		  else
		  {
				return -1;
		  }
		}
		m_nSize = nNewSize;
	   }
	  return 1;
   }
