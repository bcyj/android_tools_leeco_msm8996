/* =======================================================================
                               qcplayer_oscl_utils.cpp
DESCRIPTION
  Some utilities common across zrex oscl, psos oscl, and qcplayer.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/src/qcplayer_oscl_utils.cpp#5 $
$DateTime: 2011/03/28 16:40:17 $
$Change: 1675367 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "qcplayer_oscl_utils.h"

#include "filesourcestring.h"
/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
const int ZUtils::npos = -1;

static const char eos = '\0';

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
  ZUtils::Find

DESCRIPTION
//
// Find a pattern in a string, return starting point or npos
//

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int ZUtils::Find(const char *s, const char *pat, int i)
{

  int nsource = (int)strlen(s)+1;
  if (0 <= i && i < nsource)
  {
    int npat= (int)strlen(pat);
    for (int pos=i;s[pos]!=eos;pos++)
    {
      if (StrncmpI(&s[pos],pat,npat))
      {
        return pos;
      }
    }
  }
  return npos;
}

/* ======================================================================
FUNCTION
  ZUtils::StrncmpI

DESCRIPTION
// case-insensitive strncmp.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool ZUtils::StrncmpI(const char *s1, const char *s2, int n)
{

  for (int i=0;i<n;i++)
  {
    if (s1[i]==eos || s2[i]==eos) return false;
    if (Lower(s1[i]) != Lower(s2[i])) return false;
  }
  return true;
}

/* ======================================================================
FUNCTION
  ZUtils::Lower

DESCRIPTION
// just like tolower, which isn't available yet in zrex.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
char ZUtils::Lower(const char c)
{
  if ('A' <= c && c <= 'Z')
  {
    return char(c + 'a' - 'A');
  }
  return c;
}

/* ======================================================================
FUNCTION
  ZUtils::StrcpyN

DESCRIPTION
//
// strcpy with truncation.
//

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Returns the length of the string, not including the last null
  terminator.

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int ZUtils::StrcpyN(char *d, const char *s,int max_length,bool bNullTerm)
{
  int i;
  for (i = 0; i < max_length; i++)
  {
    d[i] = s[i];
    if ( ! s[i] )
    {
      return i;
    }
  }

  //null term may overwrite the last char
  if (bNullTerm)
  {
    d[max_length-1]='\0';
  }

  return max_length;
}

/* ======================================================================
FUNCTION
  ZUtils::FindR

DESCRIPTION
//
// Find a pattern at the end of a string, return starting point or npos
//

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int ZUtils::FindR(const char *s, const char *pat)
{
  return (Find(s,pat,int(strlen(s)-strlen(pat)) ));
}

/* ======================================================================
FUNCTION
  ZUtils::ceil

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
double ZUtils::ceil( double x )
{
  long xlong=(long)x;
  return ( ((double)xlong < x) ? (double)(xlong+1) : (double)xlong );
}

/* ======================================================================
FUNCTION
  ZUtils::Init

DESCRIPTION
//////////////////////////////////////////////////////////////////////
//
// Common Init Routine
//
//////////////////////////////////////////////////////////////////////

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
void ZUtils::Init()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "ZUtils::Init");
}

