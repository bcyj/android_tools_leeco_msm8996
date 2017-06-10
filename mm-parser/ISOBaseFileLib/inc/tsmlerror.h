#ifndef __TSML_ERROR_H__
#define __TSML_ERROR_H__
/* =======================================================================
                               tsmlerror.h
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

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/ISOBaseFileLib/main/latest/inc/tsmlerror.h#5 $
$DateTime: 2011/11/24 00:50:52 $
$Change: 2061573 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define SUCCESSFUL 0
#define ERROR_endTSML_BEFORE_startTSML 1
#define ERROR_startTSML_BEFORE_endTSML 2
#define ERROR_endTELOP_BEFORE_startTELOP 3
#define ERROR_startTELOP_BEFORE_endTELOP 4
#define ERROR_PREVIOUS_HAS_NO_ENDTIME_AND_CURRENT_HAS_NO_BEGINTIME 5
#define ERROR_endFONT_BODY_BEFORE_startFONT_BODY 8
#define ERROR_startFONT_BODY_BEFORE_endFONT_BODY 9
#define ERROR_COLOR_INFORMATION_WITHOUT_BODYFONT_TAG_STARTS 10
#define ERROR_endUNDERLINE_BEFORE_startUNDERLINE 11
#define ERROR_startUNDERLINE_BEFORE_endUNDERLINE 12
#define ERROR_endREVERSAL_BEFORE_startREVERSAL 13
#define ERROR_startREVERSAL_BEFORE_endREVERSAL 14
#define ERROR_endA_ELEMENT_BEFORE_startA_ELEMENT 15
#define ERROR_startA_ELEMENT_BEFORE_endA_ELEMENT 16
#define ERROR_WITHOUT_A_ELEMENT_HREF 17
#define ERROR_endHEAD_BEFORE_startHEAD 18
#define ERROR_startHEAD_BEFORE_endHEAD 19
#define ERROR_endBODY_BEFORE_startBODY 20
#define ERROR_startBODY_BEFORE_endBODY 21
#define ERROR_endLAYOUT_BEFORE_startLAYOUT 22
#define ERROR_startLAYOUT_BEFORE_endLAYOUT 23
#define ERROR_endREGION_BEFORE_startREGION 24
#define ERROR_startREGION_BEFORE_endREGION 25
#define ERROR_endFONT_HEAD_BEFORE_startFONT_HEAD 26
#define ERROR_startFONT_HEAD_BEFORE_endFONT_HEAD 27
#define ERROR_IN_ALOCATING_MEMORY_FOR_TELOP_HEADER 28
#define ERROR_IN_ALLOCATING_MEMORY_FOR_STRING_MEMBER 29
#define ERROR_IN_ALLOCATING_MEMORY_FOR_LINKVALUE 30
#define ERROR_IN_ALLOCATING_MEMORY_FOR_TELOP_ELEMENT 31
#define ERROR_IN_ALLOCATING_MEMORY_FOR_SUBSTRINGS 32
#define ERROR_IN_ALLOCATING_MEMORY_FOR_TSMLHANDLE 33

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
FUNCTION
  SAMPLE_FUNC

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

#endif
