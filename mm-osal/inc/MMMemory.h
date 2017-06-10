#ifndef MMMEMORY_H
#define MMMEMORY_H
/*===========================================================================
                          M M    W r a p p e r
                   f o r   C + +   M e m o r y   R o u t i n e s

*//** @file  MMMalloc.h
   This module defines routines to track and debug memory related usage

   Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc.
   All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/platform/OSAbstraction/main/latest/inc/MMMemory.h#1 $
$DateTime: 2009/10/15 12:24:41 $
$Change: 1054915 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/28/09   rmandal     Converted comments to doxygen style.
07/27/09   rmandal     Replaced qtv_ by MM_ in comments

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "MMMalloc.h"

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/**
 * This macro calls the C++ 'new' operator in order to allocate and construct an
 * object of the requested type.  Then MM_new is called in order to track memory
 * allocations for debugging purposes.  A pointer to the newly constructed object
 * is returned.
 *
 * t - type of object to allocate and construct
 */
#define MM_New(t)  ((t*)MM_new(new t, sizeof(t), __FILE__, __LINE__))

/**
 * This macro calls the C++ 'new' operator in order to allocate and construct an
 * object of the requested type passing the arguments specified.  Then MM_new is
 * called in order to track memory allocations for debugging purposes.  A pointer
 * to the newly constructed object is returned.
 *
 * t - type of object to allocate and construct
 * a - list of arguments to pass to constructor.  MUST BE SURROUNDED BY
 *     PARENTHESES.
 */
#ifdef _ANDROID_
#define MM_New_Args(t,a) ((t*)MM_new(new t a, sizeof(t), __FILE__, __LINE__))
#else
#define MM_New_Args(t,a) ((t*)MM_new(new t##a, sizeof(t), __FILE__, __LINE__))
#endif //_ANDROID_

/**
 * This macro calls the C++ 'delete' operator in order to deconstruct and free
 * the specified object.  Then MM_delete is called in order to track memory
 * allocations for debugging purposes.
 *
 * p - pointer to object which will be destructed and free'd
 */
#define MM_Delete(p)  MM_delete((p), __FILE__, __LINE__);delete(p)

/**
 * This macro calls the C++ 'new' operator in order to allocate and construct the
 * specified number of object of the requested type.  Then MM_new is called in
 * order to track memory allocations for debugging purposes.  A pointer to the
 * newly constructed array of objects is returned.
 *
 * t - type of object to allocate and construct
 * n - number of objects to allocate and construct
 */
#define MM_New_Array(t,n)  ((t*)MM_new(new t[n], (n)*sizeof(t), __FILE__, __LINE__))

/**
 * This macro calls the C++ 'new' operator in order to allocate and construct the
 * specified number of object of the requested type with the specified
 * parameters.  Then MM_new is called in order to track memory allocations for
 * debugging purposes.  A pointer to the newly constructed array of objects is
 * returned.
 *
 * t - type of object to allocate and construct
 * a - list of arguments to pass to constructor.  MUST BE SURROUNDED BY
 *     PARENTHESES.
 * n - number of objects to allocate and construct
 */
#define MM_New_Array_Args(t,a, n)  ((t*)MM_new(new t##a##[n], (n)*sizeof(t), __FILE__, __LINE__))

/**
 * This macro calls the C++ 'delete[]' operator in order to deconstruct and free
 * the specified object.  Then MM_delete is called in order to track memory
 * allocations for debugging purposes.
 *
 * p - pointer to object which will be destructed and free'd
 */
#define MM_Delete_Array(p)  MM_delete((p), __FILE__, __LINE__);delete[](p)

/* =======================================================================
**                        Class & Function Definations
** ======================================================================= */

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
 * Allocate memory from the heap.  This function replaces the standard C++ 'new'
 * operator in order to track memory usage by MM OSAL user module.
 *
 * This function should not be called directly, instead, call MM_New()
 *
 * @param[in] pMemory - pointer to the memory
 * @param[in] size  - number of bytes
 * @param[in] file - the file name for debugging
 * @param[in] line - line number for debugging
 *
 */
void *MM_new
(
  void* pMemory,
  size_t size,
  const char *file,
  int line
);

/**
 * Free memory back to the heap to be reused later.  This function assumes that
 * the memory has already been deleted.  This function just keeps track of
 * information for debugging memory errors.
 *
 * This function should not be called directly, instead, call MM_Delete()
 *
 * @param[in] pMemory - pointer to the memory
 * @param[in] file - the file name for debugging
 * @param[in] line - line number for debugging
 *
 */
void MM_delete
(
  void* pMemory,
  const char *file,
  int line
);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */


#endif // MMMEMORY_H
