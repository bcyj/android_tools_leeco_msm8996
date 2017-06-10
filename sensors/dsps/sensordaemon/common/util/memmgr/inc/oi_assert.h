#ifndef _OI_ASSERT_H
#define _OI_ASSERT_H
/** @file   
  This file provides macros and functions for compile-time and run-time assertions.
  
  When the OI_DEBUG preprocessor value is defined, the macro OI_ASSERT is compiled into
  the program, providing for a runtime assertion failure check. 
  C_ASSERT is a macro that can be used to perform compile time checks.
*/
/**********************************************************************************
  $AccuRev-Revision: 563/1 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/


/** \addtogroup Debugging Debugging APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


#ifdef OI_DEBUG

/** The macro OI_ASSERT takes a condition argument. If the asserted condition 
    does not evaluate to true, the OI_ASSERT macro calls the host-dependent function,
    OI_AssertFail(), which reports the failure and generates a runtime error. 
*/
//sean
void OI_AssertFail(char* file, int line, char* reason);
  
#define OI_ASSERT(condition) \
    { if (!(condition)) OI_AssertFail(__FILE__, __LINE__, #condition); }


#define OI_ASSERT_FAIL(msg) \
      { OI_AssertFail(__FILE__, __LINE__, msg); }

#else


#define OI_ASSERT(condition) 
#define OI_ASSERT_FAIL(msg) 
 
#endif


/**
   C_ASSERT() can be used to perform many compile-time assertions: type sizes, field offsets, etc.
   An assertion failure results in compile time error C2118: negative subscript.
   Unfortunately, this elegant macro doesn't work with GCC, so it's all commented out
   for now. Perhaps later..... 
   Does work with RVDS. Enabling this macro for DSPS target builds only.
*/

#ifdef SNS_DSPS_BUILD
 #define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]
 //#define C_ASSERT(e)
#endif


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_ASSERT_H */

