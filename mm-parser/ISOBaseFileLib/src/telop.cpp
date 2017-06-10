/* =======================================================================
                               telop.cpp
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
  QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/src/telop.cpp#22 $
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

                 DATA DEFINITIONS AND DECLARATIONS

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

/* ==========================================================================

                        MACRO DECLARATIONS

========================================================================== */

/* ==========================================================================

                   CLASS/FUNCTION DEFINITIONS

========================================================================== */

/* ======================================================================
FUNCTION
  TelopElement :: TelopElement

DESCRIPTION
  Constructor for the TelopElement.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TelopElement :: TelopElement ()
{
  oWrapFlag = 0;
  numSubStrings = 0;
  beginTime = 0;
  duration = 0;
  telopSize = 0;
}

/* ======================================================================
FUNCTION
  TelopElement :: TelopElement

DESCRIPTION
  Copy Constructor for the TelopElement.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TelopElement :: TelopElement (TelopElement & srcTelop, int32 * constructionresult)
{
  int32 i;
  SubStrings * src, * des=NULL;
  oWrapFlag = 0;
  numSubStrings = 0;
  beginTime = 0;
  duration = 0;
  telopSize = 0;

  if(constructionresult)
  {
    * constructionresult = SUCCESSFUL;

    this -> oWrapFlag       = srcTelop . oWrapFlag;
    this -> numSubStrings   = srcTelop . numSubStrings;
    this -> beginTime       = srcTelop . beginTime;
    this -> duration        = srcTelop . duration;
    this -> telopSize       = srcTelop . telopSize;

    for ( i = 0; i < srcTelop . numSubStrings; i ++ )
    {
      src = srcTelop . subStringVector . ElementAt (i);

      if(des)
      {
        if ( des->textSubString )
        {
          MM_Free(des->textSubString);
        }

        if ( des->linkValue )
        {
          MM_Free(des->linkValue);
        }
        MM_Free(des);
        des = NULL;
      }

      if(!src)
      {
        break;
      }

      des = (SubStrings*)MM_Malloc(sizeof(SubStrings));

      if ( des )
      {
        memset(des, 0, sizeof(SubStrings));
        this -> subStringVector += des;
      }
      else
      {
        * constructionresult = ERROR_IN_ALLOCATING_MEMORY_FOR_SUBSTRINGS;
        break;
      }

      des ->oLineFeed = src ->oLineFeed;
      des ->oUnderLine = src ->oUnderLine;
      des ->oReversal = src ->oReversal;
      des ->oLinking = src ->oLinking;
      des ->sizeofTextSampleInBytes = src ->sizeofTextSampleInBytes;
      des ->fontColor = src ->fontColor;
      des ->linkSize = src ->linkSize;

      if(src ->sizeofTextSampleInBytes)
      {
        des ->textSubString = (char*)MM_Malloc(src ->sizeofTextSampleInBytes + 1);

        if ( des ->textSubString )
        {
#ifdef _ANDROID_
          strlcpy(des ->textSubString, src ->textSubString,(src ->sizeofTextSampleInBytes + 1));
#else
          std_strlcpy(des ->textSubString, src ->textSubString,(src ->sizeofTextSampleInBytes + 1));
#endif
        }
        else
        {
          * constructionresult = ERROR_IN_ALLOCATING_MEMORY_FOR_STRING_MEMBER;
          MM_Free(des);
          des = NULL;
          break;
        }
      }
      if ( src ->oLinking )
      {
        des ->linkValue = (char*)MM_Malloc(src ->linkSize + 1);

        if ( des -> linkValue )
        {
#ifdef _ANDROID_
          strlcpy(des ->linkValue,src ->linkValue,(src ->linkSize + 1));
#else
          std_strlcpy (des ->linkValue,src ->linkValue,(src ->linkSize + 1));
#endif
        }
        else
        {
          * constructionresult = ERROR_IN_ALLOCATING_MEMORY_FOR_LINKVALUE;
          MM_Free(des ->textSubString);
          MM_Free(des);
          des = NULL;
          break;
        }
      }
      else
        des ->linkValue = 0;
    }
    if(des)
    {
      if ( des->textSubString )
      {
        MM_Free(des->textSubString);
        des->textSubString = NULL;
      }

      if ( des->linkValue )
      {
        MM_Free(des->linkValue);
        des->linkValue = NULL;
      }
      MM_Free(des);
      des = NULL;
    }
  }
}

/* ======================================================================
FUNCTION
  TelopElement :: GetWrapFlag

DESCRIPTION
  Returns Wrap Flag of a TelopElement.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool TelopElement :: GetWrapFlag ()const /*made constant */
{
  return oWrapFlag;
}

/* ======================================================================
FUNCTION
  TelopElement :: ~TelopElement

DESCRIPTION
  Destructor of TelopElement

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
TelopElement :: ~TelopElement ()
{
  uint32 i;
  SubStrings * substring;

  for ( i = 0; i < subStringVector .GetLength (); i ++ ) /*lint !e1551 */
  {
    substring = GetSubStringStructAt (i);/*lint !e1551 */

    if ( substring ->textSubString )
    {
      MM_Free(substring ->textSubString);
    }

    if ( substring ->linkValue )
    {
      MM_Free(substring ->linkValue);
    }

    MM_Free(substring);
  }

  subStringVector .Clear ();/*lint !e1551 */
}
#endif /* FEATURE_MP4_CUSTOM_META_DATA */
