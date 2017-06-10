
#ifndef MMMALLOC_H
#define MMMALLOC_H


#include <stdlib.h>

/*===========================================================================
                          M M    W r a p p e r
                   f o r   C   M e m o r y   R o u t i n e s

*//** @file  MMMalloc.h
   This module defines routines to track and debug memory related usage

   Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc.
   All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/platform/OSAbstraction/main/latest/inc/MMMalloc.h#1 $
$DateTime: 2009/10/15 12:24:41 $
$Change: 1054915 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/28/09   rmandal     Converted comments to doxygen style.
07/27/09   rmandal     Replaced qtv_ by MM_ in comments
07/27/09   rmandal     Replaced QTVMALLOC_H by MMMALLOC_H

========================================================================== */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/**
 * This macro calls MM_malloc in order to allocate memory from the heap and
 * track memory allocations for debugging purposes.  A pointer to the allocated
 * memory is returned.
 *
 * sz - the number of bytes to allocate
 */
#define MM_Malloc(sz)          MM_malloc(sz, __FILE__, __LINE__)

/**
 * This macro calls MM_realloc in order to allocate memory from the heap and
 * track memory allocations for debugging purposes.  A pointer to the allocated
 * memory is returned.
 *
 * p - original pointer returned by MM_Malloc
 * sz - the number of bytes to allocate
 */
#define MM_Realloc(p, sz)   MM_realloc(p, sz, __FILE__, __LINE__)

/*
 * This macro calls MM_free in order to free memory from the heap and
 * track memory allocations for debugging purposes.
 *
 * p - pointer to memory which will be free'd
 */
#define MM_Free(p)          MM_free(p, __FILE__, __LINE__)

#define MM_ASSERT
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
#ifndef _MM_HANDLE
typedef void* MM_HANDLE;
#define _MM_HANDLE
#endif


/* =======================================================================
**                        Class & Function Definations
** ======================================================================= */

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/*
 * Initialize the information needed to start logging the memory details
 *
 * @return zero when successfull else failure
 */
int MM_Memory_InitializeCheckPoint( void );

/*
 * Releases a memory check point and prints leaks if any
 *
 * @return zero when successfull else failure
 */
int MM_Memory_ReleaseCheckPoint( void );

/**
 * This function allocates the memory of requested size.
 *
 *  This function should not be called directly, instead, call MM_Malloc().
 *
 * @param[in] size - memory size requested
 * @param[in] file - the file name for debugging
 * @param[in] line - line number for debugging
 *
 * @return void* a pointer to memory
 */
 void * MM_malloc(size_t size, const char *file, int line);

/**
 * This function reallocates the memory for requested size and
 * returns the new pointer.
 *
 * This function should not be called directly, instead, call MM_Realloc().
 *
 * @param[in] pMemory - pointer to old memory
 * @param[in] size - new memory size requested
 * @param[in] file - the file name for debugging
 * @param[in] line - line number for debugging
 *
 * @return void* a pointer to memory
 */
 void * MM_realloc(void *pMemory , size_t size, const char *file, int line);

/**
 * This function frees the memory allocated using MM_malloc or
 * MM_free
 *
 *  This function should not be called directly, instead, call MM_Free().
 *
 * @param[in] pMemory - pointer to the memory
 * @param[in] file - the file name for debugging
 * @param[in] line - line number for debugging
 */
 void MM_free(void *pMemory, const char *file, int line);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif // MMMALLOC_H
