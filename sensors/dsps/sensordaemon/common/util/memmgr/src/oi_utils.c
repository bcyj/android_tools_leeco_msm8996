/**
@file
@internal  
This file contains utilities that are commonly found in stdlib libraries,
including functions for manipulating and comparing strings.
*/

/**********************************************************************************
  $AccuRev-Revision: 344/4 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/
//#define __SNS_MODULE__ OI_MODULE_SUPPORT

#include "oi_std_utils.h"
#include "oi_assert.h"
#include "oi_debug.h"
//#include "oi_bt_assigned_nos.h"

/** The zero address.  Used to test for an uninitialized address. */
//const OI_BD_ADDR OI_ZeroAddr = { {0,0,0,0,0,0} };

#ifndef USE_NATIVE_MEMCPY

OI_UINT OI_StrLen(OI_CHAR const *pStr)
{
    OI_UINT len = 0;

    while (pStr[len] != 0) {
        ++len;
    }    
    return(len);
}


//used by OI_MEMMGR_Dump
//set the define flag not to compile when release
OI_INT OI_Strcmp(OI_CHAR const *p1,
                 OI_CHAR const *p2)
{
    OI_ASSERT(p1 != NULL);
    OI_ASSERT(p2 != NULL);

    for (;;) {
        if (*p1 < *p2) return -1;
        if (*p1 > *p2) return 1;
        if (*p1 == '\0') return 0;
        p1++;
        p2++;
    }
}

#endif // ifndef USE_NATIVE_MEMCPY

/*****************************************************************************/
/* Parses BD_ADDR from a string of 12 hex digits, optionally separated by colons:
   "xxxxxxxxxxxx"
   OR
   "xx:xx:xx:xx:xx:xx"
   */

#define OI_isdigit(c)   ( ((c) >= '0') && ((c) <= '9') ? TRUE : FALSE)
