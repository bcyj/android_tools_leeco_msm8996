/*===========================================================================
                          V i d e o   W r a p p e r
                   f o r   C   M e m o r y   R o u t i n e s

*//** @file  VideoMemory.h
   This module defines routines to track and debug memory related usage

   Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc.
   All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "MMMalloc.h"

/*#ifdef FEATURE_VIDEO_MEMORY_DEBUG
#include "VideoDebugMsg.h"
#endif
#include "VideoCriticalSection.h"
#include "Windows.h"
*/
/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
// Pradnya
#undef FEATURE_VIDEO_MEMORY_DEBUG

#ifdef FEATURE_VIDEO_MEMORY_DEBUG
typedef struct {
  const void   *pointer;
  unsigned int  bytes;
  const char   *file;
  unsigned int  line;
} VideoMallocType;

#define SIZE_OF_MEM_DEBUG_ARRAY 1000
VideoMallocType VideoMallocTable[SIZE_OF_MEM_DEBUG_ARRAY] = {0};

/* Current and maximum byte counts can only be kept accurately if free can
 * determine how much memory has been free'd.
 */
static unsigned int  VideoMallocCurrBytes    = 0;
static unsigned int  VideoMallocMaxBytes     = 0;
static unsigned int  VideoMallocTotalBytes   = 0;

/*
 * Handle to the critical section to get streamline the calls to add and
 * remove entries to VideoMallocTable
 */
static MM_HANDLE memDebugLock = 0;

/*
 * Refernce count to the number to init and release calls
 */
static unsigned int numMemDebugLockRefs = 0;
#endif // FEATURE_VIDEO_MEMORY_DEBUG

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

#ifdef FEATURE_VIDEO_MEMORY_DEBUG

/*
 *  Helper function that adds a mem alloc entry to the VideoMallocTable.
 *
 * @param[in] pMemory - pointer to the memory
 * @param[in] size - memory size requested
 * @param[in] file - the file name for debugging
 * @param[in] line - line number for debugging
 */
static void MM_MallocAddEntry
(
  const void *pMemory,
  unsigned int size,
  const char *file,
  unsigned int line
)
{
  int i;
  /* If memory was allocated, store the information into the mem_pointer_array
   * table.
   */
  if( pMemory != NULL )
  {
    for( i = 0; i < SIZE_OF_MEM_DEBUG_ARRAY; i++ )
    {
      if( VideoMallocTable[i].pointer == NULL )
      {
        /* Found an empty spot, so store the data */
        VideoMallocTable[i].pointer = pMemory;
        VideoMallocTable[i].bytes   = size;
        VideoMallocTable[i].file    = file;
        VideoMallocTable[i].line    = line;
        break;
      }
    }
    /* Error checking */
    if( i >= SIZE_OF_MEM_DEBUG_ARRAY )
    {
      MM_MSG_PRIO3( MM_DEBUG, MM_PRIO_ERROR,
                       "VIDEO_Memory Unable to store malloc info: [%s:%d] %d bytes",
                       file, line, size );
    }

    /* Increment the number of bytes currently allocated */
    VideoMallocCurrBytes   += size;
    /* Increment the total number of bytes currently allocated */
    VideoMallocTotalBytes   += size;
    /* Keep track of the watermark of bytes allocated */
    if ( VideoMallocMaxBytes < VideoMallocCurrBytes )
    {
      VideoMallocMaxBytes = VideoMallocCurrBytes;
    }
  }
}

/*
 * Helper function that finds a mem alloc entry in the VideoMallocTable given a
 * previously added/allocated memory pointer.
 *
 * @param[in] pMemory - pointer to the memory to match
 *
 * @return Pointer to the VideoMallocType element if found else NULL
 */
static VideoMallocType* MM_MallocFindEntry(const void *pMemory)
{
  unsigned int i;

  if( pMemory != NULL )
  {
    /* Search through the table to find the entry corresponding to the pointer */
    for( i = 0; i < SIZE_OF_MEM_DEBUG_ARRAY; i++ )
    {
      if( VideoMallocTable[i].pointer == pMemory )
      {
        /* Found the entry.  Return it. */
        return &VideoMallocTable[i];
      }
    }
  }

  /* Error reporting */
  return NULL;
}

/*
 * Helper function that removes a mem alloc entry to the qtv_malloc_table given a
 * previously added/allocated memory pointer.  In addition keeps track of total
 * allocated byte counts.
 *
 * @param[in] pEntry - pointer to the VideoMallocType to be reset
 */
static void MM_MallocDeleteEntry(VideoMallocType *pEntry)
{
  if( pEntry != NULL )
  {
    /* Decrement the number of bytes allocated by Qtv */
    VideoMallocCurrBytes -= pEntry->bytes;

    /* Clear out the old contents.  Must set 'pointer' field to NULL to let it
     * be reused. */
    memset( pEntry, 0, sizeof(*pEntry) );
  }
}

/*
 * Walk through the array of currently valid allocations.  Report via diag
 * a summary of the items, and information about each one.
 *
 * Outputs one message for the number of allocated items and the total
 * allocated size.  Then outputs N indented messages for each still-allocated
 * item.
 */
static void MM_DumpAllocated(void)
{
  int index;

  MM_MSG_PRIO( MM_STATISTICS, MM_PRIO_ERROR, "*****************************************" );
  MM_MSG_PRIO2( MM_STATISTICS, MM_PRIO_ERROR,
                   "Video_DumpAllocated: VIDEO_Memory total=%d, max=%d",
                   VideoMallocTotalBytes, VideoMallocMaxBytes );

  for( index = 0; index < SIZE_OF_MEM_DEBUG_ARRAY; index++ )
  {
    if(VideoMallocTable[index].pointer != NULL)
    {
      MM_MSG_PRIO4( MM_STATISTICS, MM_PRIO_ERROR,
                       "Leak detected - [%s:%d] ptr=%x size=%d",
                       VideoMallocTable[index].file, VideoMallocTable[index].line,
                       VideoMallocTable[index].pointer, VideoMallocTable[index].bytes );
      Video_MallocDeleteEntry(&VideoMallocTable[index]);
    }
  }
  MM_MSG_PRIO( MM_STATISTICS, MM_PRIO_ERROR, "*****************************************" );

  VideoMallocCurrBytes    = 0;
  VideoMallocMaxBytes     = 0;
  VideoMallocTotalBytes   = 0;
}

#endif // FEATURE_VIDEO_MEMORY_DEBUG

/**
 * This function mallocs the memory of requested size.
 *
 *  This function should not be called directly, instead, call video_Malloc().
 *
 * @param[in] size - memory size requested
 * @param[in] file - the file name for debugging
 * @param[in] line - line number for debugging
 *
 * @return void* a pointer to memory
 */
void * MM_malloc(size_t size, const char *file, int line)
{
  #ifdef FEATURE_VIDEO_MEMORY_DEBUG
  if (memDebugLock)
  {
    void *pMemory = NULL;
    pMemory = malloc(size);
    video_CriticalSection_Enter(memDebugLock);
    if(pMemory)
    {
      Video_MallocAddEntry(pMemory, size, file, line);
    }
    video_CriticalSection_Leave(memDebugLock);
    return pMemory;
  }
  else
  {
    return malloc(size);
  }
  #else
  (void)file;
  (void)line;
  return malloc(size);
  #endif //FEATURE_VIDEO_MEMORY_DEBUG
}

/*
 * This function reallocs the memory for requested size and
 * returns the new pointer.
 *
 * This function should not be called directly, instead, call video_Realloc().
 *
 * @param[in] pMemory - pointer to old memory
 * @param[in] file - the file name for debugging
 * @param[in] line - line number for debugging
 *
 * @return void* a pointer to memory
 */
void * MM_realloc(void *pOldMemory , size_t size, const char *file, int line)
{
  #ifdef FEATURE_VIDEO_MEMORY_DEBUG
  if (memDebugLock)
  {
    void *pMemory = NULL;
    VideoMallocType *pEntry;
    video_CriticalSection_Enter(memDebugLock);
    pEntry = Video_MallocFindEntry( pOldMemory );
    /* Error checking */
    if( pEntry == NULL )
    {
      MM_MSG_PRIO4( MM_DEBUG, MM_PRIO_ERROR,
                       "VIDEO_Memory Unable to find malloc on realloc: [%s:%d] ptr=%d, %d bytes",
                       file, line, pOldMemory, size );
    }
    else
    {
      Video_MallocDeleteEntry(pEntry);
      pMemory = realloc(pOldMemory, size);
      if(pMemory)
      {
        Video_MallocAddEntry(pMemory, size, file, line);
      }
    }
    video_CriticalSection_Leave(memDebugLock);
    return pMemory;
  }
  else
  {
    return realloc(pOldMemory, size);
  }
  #else
  (void)file;
  (void)line;
  return realloc(pOldMemory, size);
  #endif //FEATURE_VIDEO_MEMORY_DEBUG
}

/**
 * This function frees the memory allocated using qtv_malloc or
 * qtv_free
 *
 *  This function should not be called directly, instead, call video_Free().
 *
 * @param[in] pMemory - pointer to the memory
 * @param[in] file - the file name for debugging
 * @param[in] line - line number for debugging
 */
void MM_free(void *pMemory, const char *file, int line)
{
  #ifdef FEATURE_VIDEO_MEMORY_DEBUG
  if (memDebugLock)
  {
    VideoMallocType *pEntry;
    video_CriticalSection_Enter(memDebugLock);
    pEntry = Video_MallocFindEntry( pMemory );
    /* Error checking */
    if( pEntry == NULL )
    {
      VIDEO_MSG_PRIO3( VIDEO_DEBUG, VIDEO_PRIO_ERROR,
                       "VIDEO_Memory Unable to find malloc on free: [%s:%d] ptr=%d",
                       file, line, pMemory );
    }
    else
    {
      Video_MallocDeleteEntry(pEntry);
      free(pMemory);
    }
    video_CriticalSection_Leave(memDebugLock);
  }
  else
  {
    free(pMemory);
  }
  #else
  free(pMemory);
  (void)file;
  (void)line;
  #endif // FEATURE_VIDEO_MEMORY_DEBUG

  return;
}

/*
 * Allocate memory from the heap.  This function replaces the standard C++ 'new'
 * operator in order to track memory usage by Qtv.
 *
 * This function should not be called directly, instead, call video_New()
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
)
{
  #ifdef FEATURE_VIDEO_MEMORY_DEBUG
  if( pMemory != NULL )
  {
    if (memDebugLock)
    {
      video_CriticalSection_Enter(memDebugLock);
      Video_MallocAddEntry(pMemory, size, file, line);
      video_CriticalSection_Leave(memDebugLock);
    }
  }
  #else
  (void)size;
  (void)file;
  (void)line;
  #endif // FEATURE_VIDEO_MEMORY_DEBUG

  return(pMemory);
}

/*
 * Free memory back to the heap to be reused later.  This function assumes that
 * the memory has already been deleted.  This function just keeps track of
 * information for debugging memory errors.
 *
 * This function should not be called directly, instead, call video_Delete()
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
)
{
  #ifdef FEATURE_VIDEO_MEMORY_DEBUG
  if (memDebugLock)
  {
    VideoMallocType *pEntry;
    video_CriticalSection_Enter(memDebugLock);
    pEntry = Video_MallocFindEntry( pMemory );
    /* Error checking */
    if( pEntry == 0 )
    {
      VIDEO_MSG_PRIO3( VIDEO_DEBUG, VIDEO_PRIO_ERROR,
                       "VIDEO_Memory Unable to find malloc on free: [%s:%d] ptr=%d, %d bytes",
                       file, line, pMemory );
    }
    else
    {
      Video_MallocDeleteEntry(pEntry);
    }
    video_CriticalSection_Leave(memDebugLock);
  }
  #else
  (void)pMemory;
  (void)file;
  (void)line;
  #endif // FEATURE_VIDEO_MEMORY_DEBUG
}


/*
 * Initialize the information needed to start logging the memory details
 *
 * @return zero when successfull else failure
 */
int MM_Memory_InitializeCheckPoint( void )
{
  #ifdef FEATURE_VIDEO_MEMORY_DEBUG
  if (numMemDebugLockRefs == 0 )
  {
    if ( MM_CriticalSection_Create(&memDebugLock) == 0 )
    {
      numMemDebugLockRefs = 1;
    }
    else
    {
      return 1;
    }
  }
  else if (memDebugLock)
  {
    MM_CriticalSection_Enter(memDebugLock);
    numMemDebugLockRefs++;
    MM_CriticalSection_Leave(memDebugLock);
  }
  return 0;
  #else
  return 1;
  #endif // FEATURE_VIDEO_MEMORY_DEBUG
}

/*
 * Releases a memory check point and prints leaks if any
 *
 * @return zero when successfull else failure
 */
int MM_Memory_ReleaseCheckPoint( void )
{
  #ifdef FEATURE_VIDEO_MEMORY_DEBUG
  VIDEO_HANDLE memDebugLockCopy = NULL;
  if (memDebugLock)
  {
    MM_CriticalSection_Enter(memDebugLock);

    if (numMemDebugLockRefs == 1)
    {
      //Back up memDebugLock, NULL it out and release
      //the copy (note that memDebugLock would not have
      //been added to VideoMallocTable when created)
      Video_DumpAllocated();
      numMemDebugLockRefs = 0;
      memDebugLockCopy = memDebugLock;
      memDebugLock = NULL;
      MM_CriticalSection_Leave(memDebugLockCopy);
      MM_CriticalSection_Release(memDebugLockCopy);
    }
    else
    {
      --numMemDebugLockRefs;
      MM_CriticalSection_Leave(memDebugLock);
    }
  }
  return 0;
  #else
  return 1;
  #endif // FEATURE_VIDEO_MEMORY_DEBUG
}

