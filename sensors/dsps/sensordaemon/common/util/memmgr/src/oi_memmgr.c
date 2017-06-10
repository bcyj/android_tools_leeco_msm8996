#define __SNS_MODULE__ OI_MODULE_MEMMGR
/*============================================================================
  @file oi_memmgr.c

  @brief
  Implements the sensors memory manager libraries.


  DEPENDENCIES: 

  *********************************************************************************
  $AccuRev-Revision: 8/20 $
  Copyright 2002 Open Interface North America, Inc. All rights reserved.
  *********************************************************************************

Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
  ============================================================================*/

/*============================================================================
  EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/core/sensors/dsps/common/main/latest/util/memmgr/src/oi_memmgr.c#9 $
  $DateTime: 2012/01/12 13:59:33 $

  when       who    what, where, why 
  ---------- --- -----------------------------------------------------------
  2012-01-12 br  Fragmented heap memory region into two regions
  2011-11-23 br  deleted featurization by USE_NATIVE_MEMCPY
  2011-11-22 br  Cleared comments

  ============================================================================*/
#include "oi_common.h"
#include "oi_debug.h"
#include "oi_std_utils.h"
#include "oi_memmgr.h"
#include "oi_memprof.h"

//#include "oi_debugcontrol.h"
#include "oi_modules.h"

#include "oi_support_init.h"
#include "oi_dump.h"

//#include "oi_init_flags.h"

#include "oi_assert.h"

#include "sns_common.h"
#include "sns_osa.h"

#if defined(SNS_MEMMGR_PROFILE_ON)
#include "sns_debug_str.h"
#include "sns_debug_api.h"
#endif


/*
 * This global variable allows any BM3 function to verify that the current process does
 * in fact own the stack token.
 *
 * Initialize to TRUE for those platforms which do not choose to manipulate this
 * variable from within stackwrapper token handling.
 */
 
OI_BOOL OI_StackTokenHeld = TRUE;




/*
 * Use OI_Printf instead of OI_DBGPRINT if we are not compiled for debugging.
 */
#ifndef OI_DEBUG
#undef OI_DBGPRINT
#undef OI_DBGPRINT2
#define OI_DBGPRINT(s)    OI_Printf s
#define OI_DBGPRINT2(s)   OI_Printf s
#endif


/*
 * Comment out the whole file if we are using the native malloc
 */
#ifndef USE_NATIVE_MALLOC


static OS_EVENT *mallocMutex;

#define LOCK_INIT {                                                     \
    uint8_t err;                                                        \
    mallocMutex = sns_os_mutex_create((uint8_t)SNS_MEMMGR_MUTEX, &err); \
  }

#define LOCK_DEINIT {                                             \
    uint8_t err = 0;                                              \
    sns_os_mutex_del(mallocMutex, 0 , &err);                      \
  }

#define LOCK {                                  \
    uint8_t err;                                \
    sns_os_mutex_pend(mallocMutex, 0, &err);    \
  }

#define UNLOCK sns_os_mutex_post(mallocMutex);

/************************************************************************************

  Memory Heap

 ************************************************************************************/
/*
 *  All memory is managed by the memory manager resides in a heap which is
 *  passed to the mm in the initialization call.  Memory is staticMalloc'd from
 *  the heap and never returned.  Dynamic memory allocation is also from the
 *  heap, but via the pool mechanism.
 *
 *  The heap is managed with a 'current' and 'end' pointers.  The current
 *  pointer points at the next available memory location in the heap.  The end
 *  pointer points at the first memory location past the end of the heap.
 */

/************************************************************************************

  Memory Pools

 ************************************************************************************/

/*
 * A sanity check - no sane person would ever allocate 100 different size pools.
 */

#define MEMMGR_MAX_POOL_COUNT 100


/*
 * Forward declaration of ALLOC_BLOCK
 */

typedef struct _ALLOC_BLOCK ALLOC_BLOCK;

#ifdef MEMMGR_DEBUG

/*
 * The guard string helps to detect certain classes of memory bounds overrun
 * errors. It also helps to identify the start of memory blocks when debugging
 * raw memory.  The guard string length must be a multiple of 4 bytes to
 * maintain proper memory alignment of the ALLOC_BLOCK structure.
 */
static const OI_BYTE GUARD_STRING[] = "!Qualcomm!!";

static OI_BOOL CheckGuardString(ALLOC_BLOCK const *blockPtr);

void *last_alloc;   /* the last block allocated */

void *last_free;    /* the last block freed */

/*
 * Add extra to end of each block to test for overshoot
 */
#define DEBUG_EXTRA 4

/*
 * Verify that allocated memory extends at least size bytes beyond addr.
 */
#define CHECK_EXTENT(addr, size)  OI_ASSERT(OI_MEMMGR_CheckSize(addr, size))

#else

#define CHECK_EXTENT(addr, size)
static OI_BYTE debug_out = 0;

/*
 * No Overshoot detection in Release Builds
 */
#define DEBUG_EXTRA 0

#endif /* MEMMGR_DEBUG */


/*
 * Type ALLOCBLOCK
 *
 * Raw memory blocks are cast to this type when linking and unlinking from the
 * free list. If MEMMGR_DEBUG is TRUE additional fields are allocated for
 * profiling and error checking.
 *
 * The actual allocated size is used for error checking when clearing or copying
 * memory and is also used to tune memory allocation for embedded applications.
 *
 * The Guard field is used to detect memory corruption errors due to bounds
 * errors. A fairly large guard field is more reliable at catching such errors
 * and use of an identifiable string makes it easier to debug other memory
 * allocation errors.
 */


struct _ALLOC_BLOCK {
#ifdef MEMMGR_DEBUG
    struct {
        OI_BYTE   guard[sizeof(GUARD_STRING)]; /* Detects memory corruption */
        OI_UINT32 allocSize;                   /* Actual allocation size */
        OI_CHAR  *fname;                       /* File name of allocator */
        OI_CHAR  *freeFile;                    /* File name of last freer */
        OI_UINT16 lineNum;                     /* File line number of allocator */
        OI_UINT16 freeLine;                    /* File line number of last freer */
        OI_BYTE   free;                        /* Has the block been freed? */
        OI_UINT8  module;                      /* Module that last Malloc'd block */
        OI_UINT8  alignment[2];                /* (Unused) Force 32 bit alignment on 16 bit platforms */
        ALLOC_BLOCK *priorPool;                /* Prior Pool location for Guard String coruption */
    } MemDebug;
#endif
#ifdef SNS_MEMMGR_PROFILE_ON
    struct {
        OI_UINT8  alloc_module;
        OI_UINT32 timestamp;
        OI_UINT8  owner;
        OI_UINT32 req_size;
        OI_BYTE   free;
    } MemProf;
    ALLOC_BLOCK *usedNext;
#endif
    ALLOC_BLOCK *next;     /* Link to next block in free list */
};


/*
 * The size of a block header (with debugging off this should be zero)
 */

#define BLOCK_HEADER_SIZE  (sizeof(ALLOC_BLOCK) - sizeof(ALLOC_BLOCK*))


/*
 * Macro to get the start of an alloc block
 */

#define GET_ALLOC_BLOCK(b) ((ALLOC_BLOCK*) ((OI_BYTE *) (b) - BLOCK_HEADER_SIZE))



/*
 * Type MEMPOOL
 *
 * A memory pool is a linked list of memory blocks of the same size. The
 * FreeList pointer is a pointer to a linked list of unallocated ALLOCBLOCKS.
 *
 * In production use OI_Malloc will always allocate the first block in the free
 * list, and OI_Free will always add a freed block to the head of the list. For
 * debugging we mix things up to help flush out memory allocation bugs.
 */

typedef struct {
    OI_BYTE     *LAddr;    /* Lowest memory address allocated to this pool */
    OI_BYTE     *HAddr;    /* Highest memory address allocated to this pool */
    ALLOC_BLOCK *freeList; /* Pointer to first free memory allocation block */
#ifdef MEMMGR_DEBUG
    OI_UINT32   poolSize;  /* Allocated size for every block in this pool */
#endif /* MEMMGR_DEBUG */
#ifdef SNS_MEMMGR_PROFILE_ON
    OI_UINT32   pool_size;
    OI_UINT16   high_watermark;
    OI_UINT16   num_allocated_block;
    OI_UINT16   num_max_block;
#endif /* SNS_MEMMGR_PROFILE_ON */
} MEM_POOL;

#ifdef SNS_MEMMGR_PROFILE_ON
typedef struct {
  ALLOC_BLOCK *list;
  OI_UINT16    num_block;
} ALLOC_POOL;
#endif

/*
 * The pool table is array of memory pools of various sizes as specified in the
 * pool configuration array. In a production system MEMMGR_MAX_POOL_COUNT should
 * be equal the exact number of bytes specified in the pool configuration table
 */

static MEM_POOL PoolTable[MEMMGR_MAX_ALLOWED_POOL_CNT];

/*
 * The heap is sets of statically allocated contiguous block of memory. All memory
 * blocks are allocated from the heap. Same size blocks in memory pools are
 * allocated contiguously. This means that we can tell which pool a memory block
 * is allocated from by comparing the memory block address to the low and high
 * bounds of entries in the Pool_Table.
 */

#ifdef MEMMGR_DEBUG
/*
 * Test if an address references data allocated from the pool heap
 */
#define IS_DYNAMIC_ALLOC(a) (((OI_UINT32*)(a) >= OI_memmgrHeap_frag0) && ((OI_UINT32*)(a) < (OI_memmgrHeap_frag0 + MEMMGR_TOTAL_HEAP_FRAG0_SIZE)))

static OI_UINT32 MEMMGR_ValidateAllocBlock(void const *block);
static OI_UINT32 MEMMGR_ValidateFreeBlock(ALLOC_BLOCK *blockPtr, OI_UINT32 poolsize);
#endif


/*
 * Test if an address references data allocated from a specific pool
 */

#define IS_FROM_POOL(a, p) ((((const OI_BYTE*)(a) >= PoolTable[p].LAddr)) && ((const OI_BYTE*)(a) < PoolTable[p].HAddr))


/*
 * Number of pools is established by pool configuragion parameter passed to OI_Memory_Manager_Init
 */

static OI_UINT16 PoolCount;


/*
 * The pool configuration for the pool heap as passed in to OI_MEMMGR_Init
 */

static const OI_MEMMGR_POOL_CONFIG *PoolConfiguration;


/**
 * Pointer to an OOM callback function. This is called when OOM, to try to clean
 * up memory.
 */
OI_LOWMEM_CB LowmemCb = NULL;

//sean OI_AssertFail
void OI_AssertFail(char* file, int line, char* reason)
{
      UNREFERENCED_PARAMETER(file);
      UNREFERENCED_PARAMETER(line);
      UNREFERENCED_PARAMETER(reason);

	DBG_PRINT1(("%d of %s : %s\n", line, file, reason));
} 

/**
 * Log error handler - indicate module and line number of error
 */
void OI_LogError(OI_MODULE module, OI_INT lineno, OI_STATUS status)
{
    UNREFERENCED_PARAMETER(module);
    UNREFERENCED_PARAMETER(lineno);
    UNREFERENCED_PARAMETER(status);

    /* Send error info to console */
    DBG_PRINT1(("\nOI_SLOG_ERROR: %d,%d,%d\n", module, lineno, status));
}
//sean OI_LogError
/*
 * OI_MEMMGR_Init
 *
 * Allocates blocks for each of the memory pools as specified in the pool
 * configuration parameter. The pool configuration is an array of tuples that
 * specify the block size for memory blocks in a pool and the number of blocks
 * in the pool. All blocks for all pools are allocated from a statically
 * allocated pool heap.
 */

uint32_t PoolFreeCnt[MEMMGR_MAX_POOL_COUNT];

#ifdef SNS_MEMMGR_PROFILE_ON
static ALLOC_POOL AllocatedPoolTable[MEMMGR_MAX_POOL_COUNT];
#endif

OI_STATUS OI_MEMMGR_Init(const OI_CONFIG_MEMMGR *config, OI_LOWMEM_CB pLowmemCb)
{
    OI_INT16  numBlocks;
    OI_INT32  blockSize;
    OI_INT32  poolSize;
    OI_INT16  pool;
    OI_INT32  poolHeapFragSize[MEMMGR_HEAP_FRAG_CNT];
    OI_INT32  i;
    OI_UINT32 curFragId;
    OI_BYTE  *poolHeapPtr;
#ifdef MEMMGR_DEBUG
    OI_INT32  prevBlockSize = 0;    /* to verify pool table ordering  */
    ALLOC_BLOCK *priorPool = NULL;

    /*
     * We want to ensure that the memory allocator always returns a pointer that
     * is aligned on a 32-bit boundary.  OI_Static_Malloc always returns a
     * 4-byte aligned block, so all we need to do is ensure that all block sizes
     * are a multiple of 4 and that the sizeof debug structure (if any) is a
     * multiple of 4.
     */

#ifdef _WIN32
    static_assert(0 == (BLOCK_HEADER_SIZE & (3)), "OI_Static_Malloc not aligned properly");
#else
    OI_ASSERT(0 == (BLOCK_HEADER_SIZE & (3)));
#endif
#endif /* MEMMGR_DEBUG */
    OI_MemZero(poolHeapFragSize, sizeof(poolHeapFragSize) );

/*
    OI_ASSERT(!OI_INIT_FLAG_VALUE(MEMMGR));
    if ( OI_INIT_FLAG_VALUE(MEMMGR) ){
        return OI_STATUS_ALREADY_INITIALIZED;
    }
*/
    LOCK_INIT;

    /*
     * Initialize the overall heap - it will be needed for subsequent allocation
     * of pool heaps.
     */

    OI_ASSERT(NULL != config->heapConfig);
    OI_ASSERT(NULL != config->heapConfig->heap);
    OI_ASSERT(0 != config->heapConfig->heapSize);

    /*
     * Establish the pool configuration that will be used
     */
    PoolConfiguration = config->poolConfig;

    /*
     * Count the pools and calculate the size of the pool heap
     */
    for (pool = 0; pool < MEMMGR_MAX_POOL_COUNT; ++pool) {
        numBlocks = PoolConfiguration[pool].numBlocks;
        blockSize = PoolConfiguration[pool].blockSize;
        PoolFreeCnt[pool] = numBlocks;
        /*
         * End of the config table is marked by a pair of zeroes.
         */
        if ((blockSize == 0) && (numBlocks == 0)) {
            PoolCount = pool;
            break;
        }

        OI_ASSERT( 0 == (blockSize & (3)));   /* must be a multiple of 4 */
        OI_ASSERT((blockSize > 0) && (numBlocks > 0));

#ifdef MEMMGR_DEBUG
        OI_ASSERT(blockSize > prevBlockSize); /* verify increasing block size order */
        prevBlockSize = blockSize;
#endif /* MEMMGR_DEBUG */

        /*
         * The pool heap space needed for a block is the block size defined for
         * this pool plus the block header (zero when debugging is off).
         */
        poolSize = blockSize + BLOCK_HEADER_SIZE + DEBUG_EXTRA;
        poolHeapFragSize[PoolConfiguration[pool].fragId] += poolSize * numBlocks;

#ifdef SNS_MEMMGR_PROFILE_ON
        AllocatedPoolTable[pool].list = NULL;
        AllocatedPoolTable[pool].num_block = 0;
#endif
    }

    /*
     * Check Dynamic memory size
     */
    for(i=0; i<MEMMGR_HEAP_FRAG_CNT; i++)
    {
      OI_ASSERT( poolHeapFragSize[i] <= ((OI_INT32) (config->heapConfig[i].heapSize)) );
      /*
      * Zero memory pool space
      */
      OI_MemZero(config->heapConfig[i].heap, config->heapConfig[i].heapSize );
    }

    /*
     * Initialize the pool tables
     */
    curFragId = 0;
    poolHeapPtr = config->heapConfig[curFragId].heap;

    for (pool = 0; pool < PoolCount; ++pool) {
        OI_INT16 blockNum;
        ALLOC_BLOCK *freeList = NULL;

        numBlocks = PoolConfiguration[pool].numBlocks;
        blockSize = PoolConfiguration[pool].blockSize;

        poolSize = blockSize + BLOCK_HEADER_SIZE + DEBUG_EXTRA;
        if (curFragId != PoolConfiguration[pool].fragId)
        {
          /* update with next heap frag */
          curFragId = PoolConfiguration[pool].fragId;
          poolHeapPtr = config->heapConfig[PoolConfiguration[pool].fragId].heap;
        }
        /*
         * Set low and high addresses for blocks in this pool.
         */
        PoolTable[pool].LAddr = poolHeapPtr;
        PoolTable[pool].HAddr = poolHeapPtr + (numBlocks * poolSize);

#ifdef MEMMGR_DEBUG
        PoolTable[pool].poolSize = poolSize;
#endif /* MEMMGR_DEBUG */

#ifdef SNS_MEMMGR_PROFILE_ON
        PoolTable[pool].pool_size = blockSize;
        PoolTable[pool].high_watermark = 0;
        PoolTable[pool].num_allocated_block = 0;
        PoolTable[pool].num_max_block = numBlocks;
#endif /* SNS_MEMMGR_PROFILE_ON */

        /*
         * Allocate space for each of the blocks in the pool and link them into
         * the free list for the pool
         */
        for (blockNum = 0; blockNum < numBlocks; ++ blockNum) {

            ALLOC_BLOCK *blockPtr = (ALLOC_BLOCK*) poolHeapPtr;

            /* Link this block into the pool's free list */
            blockPtr->next = freeList;
            freeList = blockPtr;

            poolHeapPtr += poolSize;

#ifdef MEMMGR_DEBUG
            {
                OI_INT32   i;
                OI_UINT8 *fill;

                /* Initialize the guard string */
                for (i = 0; i < sizeof(GUARD_STRING); ++i) {
                    blockPtr->MemDebug.guard[i] = GUARD_STRING[i];
                }
                blockPtr->MemDebug.priorPool = priorPool;
                priorPool = blockPtr;
                blockPtr->MemDebug.free = TRUE;
                blockPtr->MemDebug.allocSize = 0;
                /*
                 * Fill block with odd value pattern to trap usage of freed
                 * blocks. 
                 */
                fill = (OI_UINT8 *)&(blockPtr->next);
                fill += sizeof(blockPtr->next);
                for (; fill < (OI_UINT8 *)poolHeapPtr; fill++) {
                    *fill = 0x55;
                }

            }
#endif /* MEMMGR_DEBUG */
#ifdef SNS_MEMMGR_PROFILE_ON
              blockPtr->MemProf.alloc_module = SNS_DBG_NUM_MOD_IDS;
              blockPtr->MemProf.owner = SNS_DBG_NUM_MOD_IDS;
              blockPtr->MemProf.req_size = 0;
              blockPtr->MemProf.timestamp = 0;
              blockPtr->MemProf.free = TRUE;
              blockPtr->usedNext = NULL;
#endif /* SNS_MEMMGR_PROFILE_ON */
        }

        OI_ASSERT(PoolTable[pool].HAddr == poolHeapPtr);

        /*
         * Initialize free list with the last allocated block
         */
        PoolTable[pool].freeList = freeList;

    }

    /* Mark MEMMGR as initialized */
    //OI_INIT_FLAG_PUT_FLAG(TRUE, MEMMGR);


    MEMPROF_INIT( BLOCK_HEADER_SIZE ); /*Initialize after the pools are allocated */

    LowmemCb = pLowmemCb;

    return OI_OK;
}

/*
 * OI_MEMMGR_DeInit
 * Clean up any resources
 */
OI_STATUS OI_MEMMGR_DeInit()
{
    LOCK_DEINIT;
    return OI_STATUS_SUCCESS;
}

/*
 * OI_Malloc
 *
 * Searches through the pool list to find a pool with block size >= the
 * requested size. If the best fit pool is empty, goes to the next pool. Assumes
 * that pools in the pool table are sorted by size.
 *
 * If a suitable pool is found, the block at the head of the free list is
 * removed from the free list and returned.
 */

#ifndef MEMMGR_DEBUG
/* Non-Debug version */

void* _OI_Malloc(OI_UINT8 module, OI_UINT32 size)
{
    OI_INT16 pool;
    ALLOC_BLOCK *blockPtr;
    OI_UINT32 usize = size;
    void *retval;
#if SNS_DEBUG > 0
    OI_UINT32 largest_available = 0;
#endif /* SNS_DEBUG */
    OI_INT32 retry_cnt = 0;

    UNREFERENCED_PARAMETER(module);
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));

 retry_malloc:
    LOCK;
    for (pool = 0; pool < PoolCount; ++pool) {
        if (usize <= PoolConfiguration[pool].blockSize) {
            blockPtr = PoolTable[pool].freeList;
            if (blockPtr != NULL) {
                PoolFreeCnt[pool]--;
                
                PoolTable[pool].freeList = blockPtr->next;
                /*
                 * Allocated memory starts where the next pointer used to be
                 */
                retval = (void*) &(blockPtr->next);
                UNLOCK;
                /* Poor mans memprof --> print out details of memory allocation */
                if (debug_out) DBG_PRINT1(("\r+%d %x\n", usize, retval));

#ifdef SNS_MEMMGR_PROFILE_ON
                PoolTable[pool].num_allocated_block++;
                if(PoolTable[pool].high_watermark < PoolTable[pool].num_allocated_block)
                {
                  PoolTable[pool].high_watermark = PoolTable[pool].num_allocated_block;
                }

                blockPtr->MemProf.free = FALSE;
                blockPtr->MemProf.alloc_module = module;
                blockPtr->MemProf.owner = module;
                blockPtr->MemProf.req_size = size;
                blockPtr->MemProf.timestamp = 0;//sns_em_get_timestamp();

                blockPtr->usedNext = AllocatedPoolTable[pool].list;
                AllocatedPoolTable[pool].list = blockPtr;
                AllocatedPoolTable[pool].num_block++;
#endif

                return retval;
            }
        } 
#if SNS_DEBUG > 0
        else {
            /* Poor mans memprof --> remember largest available block size */
            if (PoolTable[pool].freeList) {
                largest_available = PoolConfiguration[pool].blockSize;
            }
        }
#endif /* SNS_DEBUG */
    }
    UNLOCK;
    

    /* Poor mans memprof --> print out details of memory failure */
    if (debug_out){
        DBG_PRINT1(("\r+%d 88888888\n", usize));
    }

    if( retry_cnt == 0 &&
        LowmemCb != NULL )
    {
      retry_cnt++;
      LowmemCb();
      goto retry_malloc;
    }

    return 0;
}


#else /* MEMMGR_DEBUG */

/* Debug version */

void* _OI_Malloc_Dbg(OI_UINT32  size,
                     OI_UINT8 module,
                     OI_CHAR *fname, OI_UINT16 lineNum)
{
    OI_INT16 pool;
    ALLOC_BLOCK *blockPtr;
    OI_UINT32 usize = size;
    void *retval;

    /* DBG_PRINT2(("%s@%d - malloc(%ld)\n", fname, lineNum, size)); */
    OI_ASSERT(OI_StackTokenHeld);
    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    LOCK;

    for (pool = 0; pool < PoolCount; ++pool) {
        if (usize <= PoolConfiguration[pool].blockSize) {
            blockPtr = PoolTable[pool].freeList;
            if (blockPtr != NULL) {
                PoolTable[pool].freeList = blockPtr->next;
                if( MEMMGR_ValidateFreeBlock(blockPtr, PoolConfiguration[pool].blockSize) != 0 ){
                    /* Additional dump of data if we are about to ASSERT */
                    DBG_PRINT1(("ERROR! - Block used after it was freed:\n"));
                    DBG_PRINT1(("Corruption at byte: %d\n", MEMMGR_ValidateFreeBlock(blockPtr, PoolConfiguration[pool].blockSize)));
                    DBG_PRINT1(("Prior Alloc: %s@%d OI_Malloc(%d)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
                    DBG_PRINT1(("Prior Free: %s@%d OI_Free(%lx)\n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine,(OI_INTPTR)&(blockPtr->next)));
                    //DBG_PRINT1(("Cur Data: %@\n", &(blockPtr->next), 16));
                    DBG_PRINT1(("New Alloc: %s@%d OI_Malloc(%d)\n",fname,lineNum,size));
                    OI_ASSERT_FAIL("OI_Malloc: block validation failed\n");
                }

                blockPtr->MemDebug.allocSize = size;  /* Record actual allocation size */
                blockPtr->MemDebug.free = FALSE;      /* This block is no longer free */
                blockPtr->MemDebug.fname = fname;     /* allocator's filename */
                blockPtr->MemDebug.lineNum = lineNum; /* allocator's line number */
                blockPtr->MemDebug.module = module;
                MEMPROF_MALLOC(size, (void*) &(blockPtr->next), module, fname);

                if ( !IS_DYNAMIC_ALLOC((void*) &(blockPtr->next))){
                    DBG_PRINT1(("Alloc Error\n%s@%d OI_Malloc(%d)\n",fname,lineNum,size));
                }
                last_alloc = ((void*) &(blockPtr->next));

                /*
                 * Allocated memory starts where the next pointer used to be
                 */
                retval = (void*) &(blockPtr->next);
                UNLOCK;
                {
                    /*
                     * Write the entire block (not just the allocated size).
                     * Note that OI_Memset cannot do this because it checks the
                     * allocated size of the block.
                     */
                  OI_UINT32 *p = (OI_UINT32 *)((OI_UINT8 *) retval + PoolConfiguration[pool].blockSize + DEBUG_EXTRA);
                    while (p != (OI_UINT32 *)retval) {
                        *(--p) = 0x77777777;
                    }
                }
                /* OI_MEMMGR_Dump(); */ //Dump the memory analysis whenever malloc
                return retval;
            }
        }
    }

    OI_MEMMGR_DumpUsedBlocks();
    OI_MEMMGR_Dump();
    UNLOCK;
    DBG_PRINT1(("Malloc(%d) failed for %s@ %d\n", size, fname, lineNum));
    return 0;
}

#endif /* MEMMGR_DEBUG */


static void Free(void *block)
{
    ALLOC_BLOCK *blockPtr;
    OI_INT16 pool;
#ifdef SNS_MEMMGR_PROFILE_ON
    ALLOC_BLOCK *curPtr;
    ALLOC_BLOCK *prevPtr;
    OI_INT16 count;
#endif

    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    OI_ASSERT(block != NULL);

#ifdef MEMMGR_DEBUG
    OI_ASSERT(IS_DYNAMIC_ALLOC(block));
#endif /* MEMMGR_DEBUG */


    blockPtr = GET_ALLOC_BLOCK(block);

    if (block != &(blockPtr->next)) {
         OI_ASSERT_FAIL("OI_Free: pointer was not returned by OI_Malloc");
    }

    for (pool = 0; pool < PoolCount; ++pool) {
        if (IS_FROM_POOL(block, pool)) {

#ifdef SNS_MEMMGR_PROFILE_ON
            PoolTable[pool].num_allocated_block--;
            blockPtr->MemProf.free = TRUE;
            blockPtr->MemProf.alloc_module = SNS_DBG_NUM_MOD_IDS;
            blockPtr->MemProf.owner = SNS_DBG_NUM_MOD_IDS;
            blockPtr->MemProf.req_size = 0;
            blockPtr->MemProf.timestamp = 0;

 
            count = 0;
            prevPtr = AllocatedPoolTable[pool].list;
            curPtr = AllocatedPoolTable[pool].list;

            while(count < AllocatedPoolTable[pool].num_block)
            {
              if(curPtr->MemProf.free == TRUE)
              {
                // Last element
                if(count + 1 == AllocatedPoolTable[pool].num_block)
                {
                  prevPtr->usedNext = NULL;
                }
                // First element
                else if(prevPtr == curPtr)
                {
                  AllocatedPoolTable[pool].list = curPtr->usedNext;
                  curPtr->usedNext = NULL;
                }
                else
                {
                  prevPtr->usedNext = curPtr->usedNext;
                  curPtr->usedNext = NULL;
                }

                AllocatedPoolTable[pool].num_block--;
                break;
              } 
              count++;
              prevPtr = curPtr;
              curPtr = curPtr->usedNext;                
            }

#endif

            blockPtr->next = PoolTable[pool].freeList;
            PoolTable[pool].freeList = blockPtr;
            PoolFreeCnt[pool]++;
            return;
        }
    }
     OI_ASSERT_FAIL("OI_Free: memory not allocated by OI_Malloc");
}


/*
 * OI_Free
 *
 * Returns a memory block to the pool heap. First determines which pool the block
 * belongs by comparing the address of the block to be freed against the address
 * ranges for the pools. The block is then linked into the free list for the
 * block and becoming the first free block in the list.
 */

#ifndef MEMMGR_DEBUG

void _OI_Free(void *block)
{
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    if (debug_out) DBG_PRINT1(("\r-%x\n", block));
    LOCK;
    Free(block);
    UNLOCK;
}

#else /* MEMMGR_DEBUG */

void _OI_Free_Dbg(void *block,
                  OI_UINT8 module,
                  OI_CHAR *fname,
                  OI_UINT16 lineNum)
{
    OI_UINT32 position; /* test for writing past end of block */

    /* DBG_PRINT2(("%s@%d - free(%lx)\n", fname, lineNum, block)); */
    OI_ASSERT(OI_StackTokenHeld);
    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    LOCK;
    last_free = block;
    if (!IS_DYNAMIC_ALLOC(block)) {
        DBG_PRINT1(("Block Not Dynamic\n%s@%d OI_Free(%lx)\n", fname, lineNum, (OI_INTPTR)block));
        OI_ASSERT(block != NULL);
    }
    position = MEMMGR_ValidateAllocBlock(block);
    Free(block);
    {
        ALLOC_BLOCK *blockPtr = GET_ALLOC_BLOCK(block);
        OI_UINT8 alloc_by = blockPtr->MemDebug.module;
        OI_UINT8 freed_by = module;
        OI_UINT32 blockSize = 0;
        //OI_UINT32 *start = ((OI_UINT32 *)(void*)&blockPtr->next) + 1; /* skip past next since it was already set by Free() */
        OI_UINT32 *start = (OI_UINT32*)((OI_BYTE*)&blockPtr->next + sizeof(blockPtr->next));
        OI_UINT32 *end;

        /* Check for prior freeing before checking overshoot, because data changes */
        if ( blockPtr->MemDebug.free ){
            DBG_PRINT1(("ERROR! - Block Freed Twice\n%s@%d OI_Free(%lx)\n",fname, lineNum, (OI_INTPTR)block));
            DBG_PRINT1(("Prior Free: %s@%d \n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine));
            DBG_PRINT1(("Prior Alloc: %s@%d OI_Malloc(%d)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
            OI_ASSERT(!blockPtr->MemDebug.free);
        }

        /* overshooting must be checked prior to actual freeing and memory fill with 55 */
        if ( position ){
            DBG_PRINT1(("ERROR! - Block overshot prior to being freed at byte %d\n", position));
            DBG_PRINT1(("This Free: %s@%d OI_Free(%lx)\n",fname, lineNum, (OI_INTPTR)block));
            DBG_PRINT1(("Prior Alloc: %s@%d OI_Malloc(%d)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
            DBG_PRINT1(("Prior Free: %s@%d \n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine));
            OI_ASSERT(!position);
        }

        /*
         * Fill freed block with odd value pattern to trap usage of freed
         * blocks. Since block sizes must be 4-byte multiples, use UINT32 pointer arithmetic.
         */
        for (position = 0; position < PoolCount; ++position) {
            if (IS_FROM_POOL(block, position)) {
                blockSize = PoolConfiguration[position].blockSize;
                break;
            }
        }

        end = start + ((blockSize + DEBUG_EXTRA)/ sizeof (OI_UINT32)) - 1; /* block size includes next */
        while (start < end) {
            *start++ = 0x55555555;
        }

        OI_ASSERT(CheckGuardString(blockPtr));

        blockPtr->MemDebug.freeFile = fname;
        blockPtr->MemDebug.freeLine = lineNum;
        blockPtr->MemDebug.free = TRUE;

        /* Always use the alloc_by module so we don't get out of sync. */
        MEMPROF_FREE(blockPtr->MemDebug.allocSize, block, blockPtr->MemDebug.module, blockPtr->MemDebug.fname);

        if ((alloc_by != OI_MODULE_MEMMGR) && (freed_by != OI_MODULE_MEMMGR)) {
            if (alloc_by != freed_by) {
                /*
                 * The following is DBGPRINT rather than LOG_ERROR because
                 * mismatch of allocating module and freeing module is at times
                 * a normal occurrence
                 */
                DBG_PRINT1(("Memory allocated by %s (line %d) was freed by %s (line %d)\n", blockPtr->MemDebug.fname, blockPtr->MemDebug.lineNum, fname, lineNum));
            }
        }
    }
    UNLOCK;
    /* OI_MEMMGR_Dump(); */ //Dump memory analysis whenever Free
}

#endif /* MEMMGR_DEBUG */


/*
 * OI_FreeIf
 *
 * Calls free if the block is not null
 */

#ifndef MEMMGR_DEBUG
void _OI_FreeIf(const void *block)
{
    OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));

    if (block != NULL) {
        OI_Free((void *)block);
    }
}
#else /* MEMMGR_DEBUG */
void _OI_FreeIf_Dbg(const void *block,
                    OI_UINT8 module,
                    OI_CHAR *fname,
                    OI_UINT16 lineNum)
{
    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));

    if (block != NULL) {
        _OI_Free_Dbg((void *)block, module, fname, lineNum);
    }
}
#endif /* MEMMGR_DEBUG */

void* _OI_Realloc(OI_UINT8 new_mod, void *block)
{
#ifdef SNS_MEMMGR_PROFILE_ON
  ((ALLOC_BLOCK *)block)->MemProf.owner = new_mod;
  ((ALLOC_BLOCK *)block)->MemProf.timestamp = 0;
#else
  UNREFERENCED_PARAMETER(new_mod);
#endif
  return block;
}


/*
 * OI_MemCopy
 */

void OI_MemCopy(void *To, void const *From, OI_UINT32 Size)
{
    /*
     * Do not OI_ASSERT(initialized) this function can be called before the
     * memory manager is initialized.
     */
    OI_ASSERT((To != NULL) || (Size == 0));
    OI_ASSERT((From != NULL) || (Size == 0));

    CHECK_EXTENT(To, Size);
    CHECK_EXTENT(From, Size);

    if (From < To) {
        /* copy starting at tail */
        while (Size > 0) {
            --Size;
            ((OI_BYTE*) To)[Size] = ((const OI_BYTE*) From)[Size];
        };
    } else {
        OI_UINT32 i;
        /* From > To ... copy starting at head */
        for (i = 0; i < Size; ++i) {
            ((OI_BYTE*) To)[i] = ((const OI_BYTE*) From)[i];
        }
    }
}


/*
 * OI_MemSet
 */

void OI_MemSet(void *Block, OI_UINT8 Val, OI_UINT32 Size)
{
    OI_UINT32 i;

    /*
     * Do not OI_ASSERT(initialized) this function can be called before the
     * memory manager is initialized.
     */
    OI_ASSERT((Block != NULL) || (Size == 0));
    CHECK_EXTENT(Block, Size);

    for (i = 0; i < Size; ++i) {
        ((OI_BYTE*) Block)[i] = Val;
    }
}


/*
 * OI_MemZero
 */

void OI_MemZero(void *Block, OI_UINT32 Size)
{
    OI_UINT32 i;

    /*
     * Do not OI_ASSERT(initialized) this function can be called before the
     * memory manager is initialized.
     */
    OI_ASSERT((Block != NULL) || (Size == 0));

    /* OI_MemZero is used to init memory not in the pool */
    /* CHECK_EXTENT(Block, Size); */

    for (i = 0; i < Size; ++i) {
        ((OI_BYTE*) Block)[i] = 0;
    }
}


/*
 * OI_MemCmp
 */

OI_INT OI_MemCmp(void const *s1, void const *s2, OI_UINT32 n) {

    OI_UINT32 i;

    /*
     * Do not OI_ASSERT(initialized) this function can be called before the
     * memory manager is initialized.
     */
    OI_ASSERT((s1 != NULL) && (s2 != NULL));
    CHECK_EXTENT(s1, n);
    CHECK_EXTENT(s2, n);

    for (i = 0; i < n; i++) {
        if ( ((const OI_BYTE *)s1)[i] < ((const OI_BYTE *)s2)[i]) return -1;
        if ( ((const OI_BYTE *)s1)[i] > ((const OI_BYTE *)s2)[i]) return 1;
    }

    return 0;
}


#ifdef SNS_MEMMGR_PROFILE_ON

void summary_pools(OI_UINT8 pool_id)
{
  SNS_PRINTF_STRING_ID_LOW_0(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_LINE_DIVIDER);
  if(pool_id == POOL_ALL)
  {
    OI_UINT8 pool;
    for(pool = 0; pool < POOL_ALL; pool++)
    {
      SNS_PRINTF_STRING_ID_LOW_2(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_HIGH_WATER_MARK,
	                               PoolTable[pool].pool_size,
                                   PoolTable[pool].high_watermark);
      SNS_PRINTF_STRING_ID_LOW_2(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_BLOCK_SUMMARY,
                                   PoolTable[pool].num_allocated_block,
                                   PoolTable[pool].num_max_block);
    }
  }
  else
  {
    SNS_PRINTF_STRING_ID_LOW_2(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_HIGH_WATER_MARK,
	                             PoolTable[pool_id].pool_size,
                                 PoolTable[pool_id].high_watermark);
    SNS_PRINTF_STRING_ID_LOW_2(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_BLOCK_SUMMARY,
                                 PoolTable[pool_id].num_allocated_block,
                                 PoolTable[pool_id].num_max_block);
  }
}
void detail_pools(OI_UINT8 pool_id)
{
  ALLOC_BLOCK *curPtr;
  OI_UINT8 count = 1;

  if(pool_id == POOL_ALL)
  {
    OI_UINT8 pool;
    for(pool = 0; pool < POOL_ALL; pool++)
    {
      count = 1;

      curPtr = AllocatedPoolTable[pool].list->usedNext;
      SNS_PRINTF_STRING_ID_LOW_0(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_LINE_DIVIDER);
      while(curPtr->usedNext != NULL)
      {
        SNS_PRINTF_STRING_ID_LOW_3(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_BLOCK_DETAIL1,
                                     count,
                                     PoolTable[pool].pool_size,
                                     AllocatedPoolTable[pool].list->MemProf.timestamp);
        SNS_PRINTF_STRING_ID_LOW_3(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_BLOCK_DETAIL2,
                                     AllocatedPoolTable[pool].list->MemProf.alloc_module,
                                     AllocatedPoolTable[pool].list->MemProf.owner,
                                     AllocatedPoolTable[pool].list->MemProf.req_size);

        count++;
        curPtr = curPtr->usedNext;
      }

    }
  }
  else
  {
    curPtr = AllocatedPoolTable[pool_id].list->usedNext;
    SNS_PRINTF_STRING_ID_LOW_0(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_LINE_DIVIDER);
    while(curPtr->usedNext != NULL)
    {
      SNS_PRINTF_STRING_ID_LOW_3(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_BLOCK_DETAIL1,
                                   count,
                                   PoolTable[pool_id].pool_size,
                                   AllocatedPoolTable[pool_id].list->MemProf.timestamp);
      SNS_PRINTF_STRING_ID_LOW_3(SNS_DBG_MOD_DSPS_SMGR, DBG_SMGR_BLOCK_DETAIL2,
                                   AllocatedPoolTable[pool_id].list->MemProf.alloc_module,
                                   AllocatedPoolTable[pool_id].list->MemProf.owner,
                                   AllocatedPoolTable[pool_id].list->MemProf.req_size);

      count++;
      curPtr = curPtr->usedNext;
    }
  }
}

#endif


/* **************************************************************************
 *
 * Debug and checking functions that are only used if MEMMGR_DEBUG is defined
 *
 * **************************************************************************/
#ifdef MEMMGR_DEBUG

/*
 * OI_MaxAllocBlock
 *
 * Searches through the pool list to find the smallest available block of
 * memory. First checks that a pool's free list is not empty then checks the
 * size of the blocks in that pool.
 */

OI_UINT32 OI_MaxAllocBlock(void)
{
    OI_UINT16 pool;
    OI_UINT32 maxSize = 0;

    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    LOCK;
    for (pool = 0; pool < PoolCount; ++pool) {
        if (PoolTable[pool].freeList != NULL) {
            if (PoolConfiguration[pool].blockSize > maxSize) {
                maxSize = PoolConfiguration[pool].blockSize;
            }
        }
    }
    UNLOCK;

    return maxSize;
}


/*
 * OI_MinAllocBlock
 *
 * Searches through the pool list to find the smallest available block of memory.
 * First checks that a pool's free list is not empty then checks the size of the
 * blocks in that pool.
 */

OI_UINT32 OI_MinAllocBlock(void)
{
    OI_UINT16 pool;
    OI_UINT32 minSize = OI_INT32_MAX;

    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    LOCK;
    for (pool = 0; pool < PoolCount; ++pool) {
        if (PoolTable[pool].freeList != NULL) {
            if (PoolConfiguration[pool].blockSize < minSize) {
                minSize = PoolConfiguration[pool].blockSize;
            }
        }
    }
    UNLOCK;
    return minSize;
}


static OI_BOOL CheckGuardString(ALLOC_BLOCK const *blockPtr)
{
    OI_UINT32 i;

    for (i = 0; i < sizeof(GUARD_STRING); ++i) {
        if (GUARD_STRING[i] != blockPtr->MemDebug.guard[i]) {
            DBG_PRINT1(("CheckGuardString failed"));
            if (blockPtr->MemDebug.priorPool) {
                blockPtr = blockPtr->MemDebug.priorPool;
                DBG_PRINT1(("Prior Block now free: %d\n",blockPtr->MemDebug.free));
                DBG_PRINT1(("Prior Last Alloc: %s@%d OI_Malloc(%d)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
                DBG_PRINT1(("Prior Last Free: %s@%d OI_Free(%lx)\n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine,(OI_INTPTR)&(blockPtr->next)));
            }
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * OI_MEMMGR_Check
 *
 * Checks that if the block was dynamically allocated that the guard string is
 * intact. Always returns TRUE for statically allocated memory.
 */

OI_BOOL OI_MEMMGR_Check(void const *Block)
{
    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    if (IS_DYNAMIC_ALLOC(Block)) {
        return CheckGuardString(GET_ALLOC_BLOCK(Block));
    }
    return TRUE; /* not dynamically allocated memory */
}


/* Validate this alloc'd block hasn't overshot its valid range */
static OI_UINT32 MEMMGR_ValidateAllocBlock(void const *block)
{
    OI_UINT8 const *check;
    OI_UINT32 index;
    OI_UINT32 blockSize = 0;
    ALLOC_BLOCK *blockPtr = GET_ALLOC_BLOCK(block);

    for (index = 0; index < PoolCount; ++index) {
        if (IS_FROM_POOL(block, index)) {
            blockSize = PoolConfiguration[index].blockSize;
            break;
        }
    }

    check = block;
    for( index = blockPtr->MemDebug.allocSize; index < (blockSize + DEBUG_EXTRA); index++ ){
        if ( check[index] != 0x77 ){
            return index;
        }
    }

    /* Looks good */
    return 0;
}

/* Validate that this free block hasn't been written to */
static OI_UINT32 MEMMGR_ValidateFreeBlock(ALLOC_BLOCK *blockPtr, OI_UINT32 poolsize)
{
    OI_UINT8 *check;
    OI_UINT32 invalid;

    if( !blockPtr->MemDebug.free ||                                         /* block must be marked free */
        (!IS_DYNAMIC_ALLOC(blockPtr->next) && (blockPtr->next != NULL))){   /* next block must be dynamic */
        return 1;
    }

    /* Verify next block is properly alligned */
    if( ((OI_INTPTR)(blockPtr->next)) & 0x03 ){
        return 3;
    }


    /* Verify that all bytes in block still have Free pattern */
    check = (OI_UINT8 *) &blockPtr->next;
    for (invalid = sizeof(void*); invalid < (poolsize + DEBUG_EXTRA); invalid++){
        if ( check[invalid] != 0x55 ){
            return invalid;
        }
    }

    /* Looks good */
    return 0;
}

/*
 * OI_MEMMGR_CheckSize
 *
 * For dynamically allocated memory, checks that there are at least "Size" bytes
 * between addr and the end of the appropriate allocation block.
 *
 * Always returns TRUE for statically allocated or stack memory.
 */

OI_BOOL OI_MEMMGR_CheckSize(void const *addr,
                            OI_UINT32 Size)
{
    ALLOC_BLOCK *blockPtr = NULL;
    OI_UINT32 poolSize;
    OI_UINT32 poolNum;
    OI_UINT16 pool;

    /*
     * There are a couple of situations during stack initialization that this
     * function is called before the memory manager has been initializted.
     */
/*
	if (!OI_INIT_FLAG_VALUE(MEMMGR)) {
        return TRUE;
    }
*/
    if (!IS_DYNAMIC_ALLOC(addr)) {
        return TRUE;
    }
    /*
     * Figure out which block this address is in
     */
    for (pool = 0; pool < PoolCount; ++pool) {
        if (IS_FROM_POOL(addr, pool)) {
            poolSize = PoolTable[pool].poolSize;
            poolNum = ((OI_INTPTR) addr - (OI_INTPTR) PoolTable[pool].LAddr) / poolSize;
            blockPtr = (ALLOC_BLOCK*) (PoolTable[pool].LAddr + poolSize * poolNum);
            break;
        }
    }
    OI_ASSERT(blockPtr != NULL);
    OI_ASSERT(CheckGuardString(blockPtr));

    /* Print out debugging info prior to assert fail if this block has been freed */
    if (blockPtr->MemDebug.free){
        DBG_PRINT1(("ERROR! - Block Checked after it was freed:\n"));
        DBG_PRINT1(("Prior Alloc: %s@%d OI_Malloc(%d)\n",blockPtr->MemDebug.fname,blockPtr->MemDebug.lineNum,blockPtr->MemDebug.allocSize));
        DBG_PRINT1(("Prior Free: %s@%d OI_Free(%lx)\n",blockPtr->MemDebug.freeFile,blockPtr->MemDebug.freeLine,(OI_INTPTR)&(blockPtr->next)));
        OI_ASSERT_FAIL("OI_MEMMGR_CheckSize: block free");
    }

    /*
     * Reduce available space by the offset of addr relative to blockPtr
     */
    if (Size <= (blockPtr->MemDebug.allocSize - ((const OI_BYTE*) addr - (const OI_BYTE*) &(blockPtr->next)))) {
        return TRUE;
    }
    DBG_PRINT1(("CheckSize failed - requested size %d, actual size %d, file %s line %d\n",
                 Size, blockPtr->MemDebug.allocSize, blockPtr->MemDebug.fname, blockPtr->MemDebug.lineNum));
    return FALSE;
}


/*
 * OI_MEMMGR_DumpPools
 *
 * Prints out the current pool allocation for the pool heap
 */
void OI_MEMMGR_DumpPools(void)
{
    OI_INT16 pool;

    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    DBG_PRINT1(("Heap pool dump"));

    LOCK;
    for (pool = 0; pool < PoolCount; ++pool) {
        ALLOC_BLOCK *blockPtr = PoolTable[pool].freeList;
        OI_INT16 count = 0;
        OI_UINT32 maxSize = 0;
        while (blockPtr != NULL) {
            ++count;
            if (blockPtr->MemDebug.allocSize > maxSize) {
                maxSize = blockPtr->MemDebug.allocSize;
            }
            blockPtr = blockPtr->next;
        }
        DBG_PRINT1(("  [%lx .. %lx] alloc size %d, max size %d, free %d\n",
                     (OI_INTPTR) PoolTable[pool].LAddr,
                     (OI_INTPTR) PoolTable[pool].HAddr,
                     PoolConfiguration[pool].blockSize, maxSize, count));

    }
    UNLOCK;
}

/*
 * OI_MEMMGR_DumpUsedBlocks
 *
 * Prints out all blocks that are currently in use
 */

void OI_MEMMGR_DumpUsedBlocks(void)
{
	OI_INT16 poolNum;

    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    DBG_PRINT1(("MemHeap dump of all used blocks\n"));
    DBG_PRINT1(("-----------------------------------------------\n"));

    LOCK;
    /*
     * for each pool, examine each block in the pool. If it is not free, print
     * out all the information about the block.
     */
    for (poolNum = 0; poolNum < PoolCount; ++poolNum) {
        MEM_POOL        *pPool = &PoolTable[poolNum];  /* ptr to pool */
        ALLOC_BLOCK     *pBlock;                       /* ptr to block */
        OI_UINT16       freeCount = 0;
        OI_UINT16       usedCount = 0;

        pBlock = (ALLOC_BLOCK*)(pPool->LAddr);         /* point at first block in this pool */
        while (pBlock < (ALLOC_BLOCK*)pPool->HAddr) {  /* while still within this pool */
            if (pBlock->MemDebug.free) {
                ++freeCount;
            } else {
                ++usedCount;
                DBG_PRINT1(("   Malloc(%d) at %s@%d\n",
                          pBlock->MemDebug.allocSize,
                          pBlock->MemDebug.fname,
                          pBlock->MemDebug.lineNum));
            }
            pBlock = (ALLOC_BLOCK*)(((OI_INTPTR)pBlock) + pPool->poolSize);
        }
        DBG_PRINT1(("[pool %4d]  used:%4d free:%4d\n",
                   PoolConfiguration[poolNum].blockSize, usedCount, freeCount));
        DBG_PRINT1(("-----------------------------------------------\n"));
    }
    UNLOCK;
}


OI_UINT32 OI_MEMMGR_CurrentDynamicAllocation(void)
{
    OI_INT16    curPool;
    OI_UINT32   freeBlocksThisPool;
    OI_UINT32   allocThisPool;
    ALLOC_BLOCK *pFree;
    OI_UINT32   totalAlloc = 0;

    //OI_ASSERT(OI_INIT_FLAG_VALUE(MEMMGR));
    LOCK;
    for (curPool = 0; curPool < PoolCount; ++curPool) {
        // count number of blocks in the free list
        freeBlocksThisPool = 0;
        pFree = PoolTable[curPool].freeList;
        while (NULL != pFree) {
            ++freeBlocksThisPool;
            pFree = pFree->next;
        }
        // allocated memory from this pool = (total_blocks - free_blocks) * blocksize
        allocThisPool = (PoolConfiguration[curPool].numBlocks - freeBlocksThisPool) * PoolConfiguration[curPool].blockSize;
        totalAlloc += allocThisPool;
    }
    UNLOCK;
    return(totalAlloc);
}

#endif /* MEMMGR_DEBUG */

#endif /* USE_NATIVE_MALLOC */

/*****************************************************************************/
