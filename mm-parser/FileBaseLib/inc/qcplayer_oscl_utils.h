#ifndef _QCPLAYER_OSCL_UTILS_H_
#define _QCPLAYER_OSCL_UTILS_H_
/* =======================================================================
                               qcplayer_oscl_utils.h
DESCRIPTION
//    Some utilities common across zrex , qcplayer.
  
Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/qcplayer_oscl_utils.h#5 $
$DateTime: 2011/03/28 16:40:17 $
$Change: 1675367 $


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

// for some platforms the order is important, so this file includes
// all the system and OS-related includes.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
////////////////
// Support for floating point print
////////////////

//zrex message routines won't take %f, so use
//  message(" text %d.%03d",whole(f),decimal(f),0)
//instead of
//  message("   text %f",f);

//whole part of a float
#define ZWHOLE(f) ((int)f)

//1st 3 decimals of a float
#define ZDECIMAL(f) ((int)(1000*f - 1000*(int)f))


///////////////////
// Clock accuracy.
///////////////////

  // clock granularity for zrex clock.  this is built into the OS.
#define COMMON_MIN_TIME 5

///////////////////
// Task delete message
///////////////////


#define QC_TASK_DELETE_MESSAGE 0


//////////////////////////////////////////////////////////////////////
//
// Min, Max macros
//
//////////////////////////////////////////////////////////////////////

#define ZMAX(a,b) (((a)>(b)) ? (a) : (b))
#define ZMIN(a,b) (((a)<(b)) ? (a) : (b))

//limit a to range (b,c)
#define ZLIMIT(a,b,c) (ZMIN(ZMAX(a,b),c))

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
**                        Function Declarations
** ======================================================================= */

/* ======================================================================
CLASS 
  ZUtils

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
class ZUtils
{

public:

  ///////////////////////////////
  //Init & Terminate routine for this module.
  ///////////////////////////////
  static void Init();
  static void Terminate();

  ///////////////////////////////
  //Clock utils
  ///////////////////////////////
  static unsigned long Clock();

  ///////////////////////////////
  //String utilities
  ///////////////////////////////

  static const int npos;

  static int Find(const char *s, const char *pat, const int i=0);
  static bool StrncmpI(const char *s1, const char *s2, int n);
  static char Lower(const char c);
  static int FindR(const char *s, const char *pat) ;
  static int StrcpyN(char *d, const char *s,int max_length,bool bNullTerm=true);
  ///////////////////////////////
  // Math
  ///////////////////////////////
  static double ceil( double x ) ;

private:

  static void InitClock();

};

#endif //_QCPLAYER_OSCL_UTILS_H_
