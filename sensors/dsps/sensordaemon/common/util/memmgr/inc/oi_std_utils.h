#ifndef _UTILS_H
#define _UTILS_H

/** 
@file
@internal
   
 This file contains utilities that are commonly found in stdlib libraries.
*/

/**********************************************************************************
  $AccuRev-Revision: 563/1 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/

#include "oi_stddefs.h"
//#include "oi_bt_spec.h"
//sean
#include "oi_string.h"
#include "oi_utils.h"

/** \addtogroup Misc_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


//extern const OI_BD_ADDR OI_ZeroAddr;
/*
void OI_StrToUpper(OI_CHAR *str);
*/

/** Convert a character to upper case. */
#define OI_toupper(c) ( ((c) >= 'a') && ((c) <= 'z') ? ((c) - 32) : (c) )

/**
 * This macro initializes a BD_ADDR. This currently zeros all bytes.
 */

#define INIT_BD_ADDR(addr)      OI_MemZero((addr), OI_BD_ADDR_BYTE_SIZE)

#define OI_IS_ZERO_ADDR(addr)  SAME_BD_ADDR((addr), &OI_ZeroAddr) 

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _UTILS_H */

