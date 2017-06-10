/* =======================================================================
                               tsml.cpp
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

Copyright 2008-2013 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/tsml.cpp#12 $
$DateTime: 2013/09/19 20:13:39 $
$Change: 4465453 $


========================================================================== */
#ifdef FEATURE_MP4_CUSTOM_META_DATA
/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"
#include "telop.h"

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
  TsmlAtom :: ParseTelopElementText

DESCRIPTION
  The String is passed to this function along with its size. The function
  stores the information in the TsmlAtom and returns its pointer. If error
  in parsing occurs, NULL pointer is returned.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TsmlAtom * TsmlAtom :: ParseTelopElementText (char * string, int32 size,
                                              int32 contentTime, int32 * parsingResult)
{
  TsmlAtom * pTsmlAtom;

  pTsmlAtom = MM_New_Args( TsmlAtom, (string, size, contentTime, parsingResult) );

  if ( pTsmlAtom )
  {
    if ( ! * parsingResult )
    {
      return pTsmlAtom;
    }

    MM_Delete( pTsmlAtom );

  }

  return NULL;
}

/* ======================================================================
FUNCTION
  TsmlAtom :: TsmlAtom

DESCRIPTION
  Constructor of the TsmlAtom.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TsmlAtom :: TsmlAtom (char * string, int32 size, int32 contentTime,
                      int32 * parsingResult)
{
  TsmlHandle * pTsmlHandle;

  telopHeader . backGroundColor  = 0x00FFFFFF;
  telopHeader . defaultFontColor = 0x00000000;

  currentIndex = 0;

  pTsmlHandle = MM_New_Args( TsmlHandle, (this) );

  if ( pTsmlHandle )
  {
    * parsingResult = pTsmlHandle -> Parse (string, size, contentTime);

    MM_Delete( pTsmlHandle );
  }
  else
  {
    * parsingResult = ERROR_IN_ALLOCATING_MEMORY_FOR_TSMLHANDLE;
  }
}

#if 0
/* ======================================================================
FUNCTION
  TsmlAtom :: GetTelopHeader

DESCRIPTION
  Returns the pointer to the Telop Header.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TelopHeader * TsmlAtom :: GetTelopHeader ()
{
  return & telopHeader;
};

/* ======================================================================
FUNCTION
  TsmlAtom :: GetSampleSizeTable

DESCRIPTION
  Returns the pointer to the Sample Size Table.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
ZArray <int32> * TsmlAtom :: GetSampleSizeTable ()
{
  return & SampleSizeTable;
};

/* ======================================================================
FUNCTION
  TsmlAtom :: GetTimeStampTable

DESCRIPTION
  Returns the pointer to the Time Stamp Table.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
ZArray <int32> * TsmlAtom :: GetTimeStampTable ()
{
  return & TimeStampTable;
};

/* ======================================================================
FUNCTION
  TsmlAtom :: GetSampleOffsetTable

DESCRIPTION
  Returns the pointer to the Sample Offset Table.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
ZArray <int32> * TsmlAtom :: GetSampleOffsetTable ()
{
  return & SampleOffsetTable;
};

/* ======================================================================
FUNCTION
  TsmlAtom :: GetTelopElementVector

DESCRIPTION
  Returns the pointer to the Telop Element Vector

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
ZArray <TelopElement *> * TsmlAtom :: GetTelopElementVector ()
{
  return & TelopElementTable;
};
#endif

/* ======================================================================
FUNCTION
  TsmlAtom :: GetTelopElementByIndex

DESCRIPTION
  Returns the pointer to the Telop Element.
  If index is invalid then NULL pointer is returned.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TelopElement * TsmlAtom :: GetTelopElementByIndex (uint32 index)
{
  if ( index < TelopElementTable . GetLength () )
  {
    return TelopElementTable . ElementAt (index);
  }
  else
  {
    return NULL;
  }
}

/* ======================================================================
FUNCTION
  TsmlAtom :: GetTelopElementByTime

DESCRIPTION
  Returns the pointer to the Telop Element.
  Three TIME options are valid.

  If CURRENT is specified the telopelement whose begin time is equal to
  the specified time will be returned. If not available then NULL is returned.

  If PAST is specified the telop element whose begin time is equal to or just
  less than the specified time, is returned. If not available then NULL is
  returned.

  If FUTURE is specified the telop element whose begin time is equal to or
  just greater than the specified time, is returned. If not available
  then NULL is returned.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TelopElement * TsmlAtom :: GetTelopElementByTime (int32 time, TIME flag)
{
  int32 i;

  if ( flag == CURRENT )
  {
    for ( i = 0; i < (int32)TimeStampTable . GetLength (); i ++ )
    {
      if ( TimeStampTable.ElementAt (i) == time )
      {
        return GetTelopElementByIndex (i);
      }
    }
  }
  else if ( flag == PAST )
  {
    for ( i = TimeStampTable . GetLength () - 1; i >= 0; i -- )
    {
      if ( TimeStampTable . ElementAt (i) <= time )
      {
        return GetTelopElementByIndex (i);
      }
    }
  }
  else if ( flag == FUTURE )
  {
    for ( i = 0; i < (int32)TimeStampTable . GetLength (); i ++ )
    {
      if ( TimeStampTable . ElementAt (i) >= time )
      {
        return(GetTelopElementByIndex (i));
      }
    }
  }

  return NULL;
}

/* ======================================================================
FUNCTION
  TsmlAtom :: ResetTelopVectorIndex

DESCRIPTION
  Reset the current index of the telop element to - 1.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void TsmlAtom :: ResetTelopVectorIndex ()
{
  currentIndex = 0;
}

/* ======================================================================
FUNCTION
  TsmlAtom :: GetCurrentIndex

DESCRIPTION
  Returns the current index of the telop element in the vector.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int32 TsmlAtom :: GetCurrentIndex ()
{
  return currentIndex;
}

/* ======================================================================
FUNCTION
  TsmlAtom :: GetNextTelopElement

DESCRIPTION
  Returns the next telop element. The index is initialized as - 1.
  Increment the current index by one.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TelopElement * TsmlAtom :: GetNextTelopElement ()
{
  TelopElement * pTelop;
  if ( currentIndex < (int32)TimeStampTable . GetLength () )
  {
    pTelop = GetTelopElementByIndex (currentIndex);
    currentIndex ++;
    return pTelop;
  }
  else
  {
    return NULL;
  }
}

/* ======================================================================
FUNCTION
  TsmlAtom :: GetTelopTrackDuration

DESCRIPTION
  Returns the next telop track duration.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32 TsmlAtom :: GetTelopTrackDuration ()
{
  int32 lastTelop = TelopElementTable . GetLength () - 1;

  if ( lastTelop > 0 )
  {
    TelopElement *pTelopelement = GetTelopElementByIndex (lastTelop);

    if ( pTelopelement != NULL )
    {
      return (uint32)(pTelopelement->GetBeginTime() + pTelopelement->GetDuration());
    }
  }

  return 0;
}

/* ======================================================================
FUNCTION
  TsmlAtom :: ResetTelopVectorIndexByTime

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
bool TsmlAtom :: ResetTelopVectorIndexByTime(int32 time)
{
  for ( int32 i = TimeStampTable . GetLength () - 1; i >= 0; i -- )
  {
    if ( TimeStampTable . ElementAt (i) < time )
    {
      currentIndex = i;
      return true;
    }
  }
  /* if even first element's time is greater than given time, then
     set current index to 0 (as it is biginning) */
  currentIndex = 0;
  return true;
}


/* ======================================================================
FUNCTION
  TsmlAtom :: ~TsmlAtom

DESCRIPTION
  Delete the TsmlAtom

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TsmlAtom :: ~TsmlAtom ()
{
  uint32 i;
  TelopElement * pTelopelement;

  for ( i = 0; i < TelopElementTable .GetLength (); i ++ )
  {
    pTelopelement = GetTelopElementByIndex (i);

    if ( pTelopelement )
    {
      MM_Delete( pTelopelement );
    }
  }

  TelopElementTable .Clear ();
  SampleSizeTable .Clear ();
  TimeStampTable .Clear ();
  SampleOffsetTable .Clear ();
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
