/**
 * @file
 * @internal  
 *
 * functions to record the use of memory by the Memory Manager
 *
 */


/**********************************************************************************
  $AccuRev-Revision: 370/2 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/

#ifndef _MEMPROF_H
#define _MEMPROF_H

#include "oi_stddefs.h"

/** \addtogroup MemMgr_Internal */
/**@{*/

/*
 * Cannot do memory profiling if using native malloc.
 */
#ifdef USE_NATIVE_MALLOC
#undef MEMMGR_PROFILE
#endif


#ifdef MEMMGR_PROFILE

#define MEMPROF_INIT(x)                 OI_MemProf_Init(x)
#define MEMPROF_STATIC_MALLOC(s, m, f)  OI_MemProf_StaticMalloc(s, m, f)
#define MEMPROF_MALLOC(s, a, m, f)      OI_MemProf_Malloc(s, a, m, f)
#define MEMPROF_FREE(s, a, m, f)        OI_MemProf_Free(s, a, m, f)

OI_STATUS OI_MemProf_Init(OI_INT debug_overhead);


void OI_MemProf_StaticMalloc(OI_INT32 size,
                             OI_UINT8 module,
                             OI_CHAR *fileName);


void OI_MemProf_Malloc(OI_INT32 size,
                       void* addr,
                       OI_UINT8 module,
                       OI_CHAR *fileName);


void OI_MemProf_Free(OI_INT32 size,
                     void* addr,
                     OI_UINT8 module,
                     OI_CHAR *filename);

#else

#define MEMPROF_INIT(x)
#define MEMPROF_STATIC_MALLOC(s, m, f)
#define MEMPROF_MALLOC(s, a, m, f)
#define MEMPROF_FREE(s, a, m, f)

#endif /* MEMMG_PROFILE */


#endif /* _MEMPROF_H */

/**@}*/
