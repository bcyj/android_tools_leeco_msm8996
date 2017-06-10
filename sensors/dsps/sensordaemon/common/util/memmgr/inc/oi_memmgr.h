#ifndef _MEMMGR_H
#define _MEMMGR_H
/** 
@file
@internal
   
  This header file provides memory management functions.
  
  The memory allocator assumes that all dynamic memory is allocated from a fixed
  size pool. For a desktop environment this pool can be large, but for embedded
  applications this pool may be quite small. A block-based allocation scheme is used to
  prevent memory fragmentation. The size and number of blocks of each allocation
  size will be tuned to the needs of the specific embedded application.  
  
  See @ref memMgr_docpage for more information on the basic function of the Memory Manager.

*/
/**********************************************************************************
  $AccuRev-Revision: 344/1 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/

#include "oi_stddefs.h"
#include "oi_status.h"
#include "oi_debug.h"
#include "oi_string.h"

#include "sns_memmgr.h"

#ifdef USE_NATIVE_MALLOC
#include <stdlib.h>
#endif

/** \addtogroup MemMgr_Internal */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/*
 * IF OI_DEBUG is defined then MEMMGR is compiled for debugging and memory
 * profiling. If OI_DEBUG is not defined memmgr debugging and profiling can be
 * turned on indepdentently by defining MEMMGR_DEBUG and MEMMGR_PROFILE
 */

#ifdef OI_DEBUG
    #define MEMMGR_DEBUG
    #ifndef SMALL_MEMORY
        #define MEMMGR_PROFILE
    #endif
#endif

/*
 * Cannot use memory manager debugging or profiling is using native malloc
 * and using native malloc implies use native memcpy.
 */
#ifdef USE_NATIVE_MALLOC
//#undef MEMMGR_DEBUG
//#undef MEMMGR_PROFILE
#endif

/*
 * OI_Malloc, OI_Calloc
 *
 * Allocates the smallest available memory block that is large enough to hold
 * the specified number of bytes. Returns 0 if there is no memory block of the
 * required size available
 *
 * For debugging, OI_Malloc is a macro which adds file and line number to the
 * malloc call so that we can trace memory hogs and memory leaks.  
 *
 * OI_Calloc does the allocation and then zeroes the memory before returning it.
 */

#ifdef USE_NATIVE_MALLOC

#include <stdlib.h>

#define OI_Malloc(size)        malloc((size))
#define OI_Calloc(size)        calloc((size), 1)
#define OI_Free(x)             free((x))
#define OI_StaticMalloc(size)  calloc((size),1)
#define OI_FreeIf(x)  if (*(x) != NULL) { free(*(x)); *(x) = NULL; }

#else /* USE_NATIVE_MALLOC */

#ifdef MEMMGR_DEBUG

#define OI_Malloc(module, size) _OI_Malloc_Dbg((size), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__))
#define OI_Calloc(size) _OI_Calloc_Dbg((size), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__))

void* _OI_Malloc_Dbg(OI_UINT32 size, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);
void* _OI_Calloc_Dbg(OI_UINT32 size, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);

#else

#define OI_Malloc(module,size) _OI_Malloc(module,size)
void* _OI_Malloc(OI_UINT8 module, OI_UINT32 size);

#define OI_Calloc(size) _OI_Calloc(size)
void* _OI_Calloc(OI_UINT32 size);

#define OI_Realloc(new_mod,block) _OI_Realloc(new_mod,block);
void* _OI_Realloc(OI_UINT8 new_mod, void *block);

#ifdef SNS_MEMMGR_PROFILE_ON
#define SUMMARY_DUMP(pool_id)   summary_pools(pool_id)
void summary_pools(OI_UINT8 pool_id);
#define DETAIL_DUMP(pool_id)     detail_pools(pool_id)
void detail_pools(OI_UINT8 pool_id);

#endif /* SNS_MEMMGR_PROFILE_ON */

#endif /* MEMMGR_DEBUG */

/*
 * OI_StaticMalloc
 *
 * Allocates memory from the static allocation pool. This is data that is
 * required for the stack to function and will never explicitly freed.
 * 
 * The memory allocated will be zero'd before returning to caller.
 */

#ifdef MEMMGR_DEBUG

#define OI_StaticMalloc(size) _OI_StaticMalloc_Dbg((size), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__))

void* _OI_StaticMalloc_Dbg(OI_INT32 size, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);

#else

#define OI_StaticMalloc(size) _OI_StaticMalloc(size)
void* _OI_StaticMalloc(OI_INT32 size);

#endif /* MEMMGR_DEBUG */


/*
 * OI_Free
 *
 * Returns a memory block to the memory pool and makes it available for
 * subsequent allocations. Fails with an assertion error if the memory was not
 * allocated by OI_Malloc or if the memory block has become corrupt.
 *
 * OI_FreeIf
 *
 * Passed a reference to a pointer to a dynamic memory block. If the value
 * referenced is non-null, frees the memory and sets the referenced value to
 * null.
 */

#ifdef MEMMGR_DEBUG

#define OI_Free(block) _OI_Free_Dbg((block), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__))
void _OI_Free_Dbg(void *block, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);

#define OI_FreeIf(p) \
    do { \
        _OI_FreeIf_Dbg(*(p), OI_CURRENT_MODULE, __FILE__, (OI_UINT16)(__LINE__)); \
        *(p) = NULL; \
    } while (0)
void _OI_FreeIf_Dbg(const void *blockRef, OI_UINT8 module, OI_CHAR *fname, OI_UINT16 lineNum);

#else

#define OI_Free(block) _OI_Free(block)
void _OI_Free(void *block);

#define OI_FreeIf(p)  do { _OI_FreeIf(*(p)); *(p) = NULL; } while (0)
void _OI_FreeIf(const void *blockRef);

#endif /* MEMMGR_DEBUG */

#endif /* USE_NATIVE_MALLOC */


/**
 * OI_MEMMGR_Check, OI_MEMMGR_CheckSize
 *
 * For debugging only
 *
 * Checks the memory address is a valid memory block address and that has been
 * allocated (i.e. not on the free list) and checks that the contents at that
 * memory block has not been corrupted.
 * 
 * Returns TRUE if the address and contents are ok, FALSE otherwise.
 *
 * OI_MEMMGR_CheckSize also checks that the allocated storage for the memory block
 * is >= Size.
 *
 * Intended for use with the ASSERT macro, for example:
 *
 *     OI_ASSERT(OI_MEMMGR_Check(ptr));
 *     OI_ASSERT(OI_MEMMGR_CheckSize(buffer, 96));
 */

#ifdef MEMMGR_DEBUG

OI_BOOL OI_MEMMGR_Check(void const *Block);

OI_BOOL OI_MEMMGR_CheckSize(void const *Block,
                            OI_UINT32 Size);

/**
 * How much dynamic memory is currently allocated
 */
OI_UINT32 OI_MEMMGR_CurrentDynamicAllocation(void);

/*
 * OI_MaxAllocBlock
 *
 * Returns the size of the largest memory block currently available for
 * allocation by a OI_Malloc
 */
OI_UINT32 OI_MaxAllocBlock(void);


/*
 * OI_MinAllocBlock
 *
 * Returns the size of the smallest memory block currently available for
 * allocation by a OI_Malloc
 */
OI_UINT32 OI_MinAllocBlock(void);

#else

#define OI_MEMMGR_Checksum(B)        (0)

#define OI_MEMMGR_Check(B)        (TRUE)

#define OI_MEMMGR_CheckSize(B, S) (TRUE)

#define OI_MEMMGR_CurrentDynamicAllocation()  (0)

#define OI_MaxAllocBlock() (0)

#define OI_MinAllocBlock() (0)

#endif /* MEMMGR_DEBUG */

#ifndef OI_REENTRANT
void mallocMutexInit(void);
void mallocMutexLock(void);
void mallocMutexUnlock(void);
#endif


#ifdef __cplusplus
}
#endif

/**@}*/

/*****************************************************************************/
#endif /* _MEMMGR_H */

