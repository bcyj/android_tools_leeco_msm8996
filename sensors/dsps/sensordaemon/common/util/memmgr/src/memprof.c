/**
 * @file
 * @internal
 *
 * functions to record the use of memory by the Memory Manager
 *
 */

/**********************************************************************************
  $AccuRev-Revision: 344/4 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/
#define __SNS_MODULE__ OI_MODULE_MEMMGR

//#include "oi_core_common.h"
//#include "oi_osinterface.h"
#include "oi_memmgr.h"
#include "oi_memprof.h"
#include "oi_utils.h"
#include "oi_assert.h"

/**
 * variables for use by the memmgr profiler
 */

typedef struct {
    OI_UINT dynamicSize;
    OI_UINT staticSize;
    OI_UINT dynamicSizeMax;
} MODULE_MEMORY;


/*
 * Entire file is ifdef'd out if we are not doing memory profiling.
 */
#ifdef MEMMGR_PROFILE


static MODULE_MEMORY moduleMemUsage[OI_MODULE_UNKNOWN + 1];


/*
 * The currentUses total requested size for dynamic memory.
 */

static OI_UINT32 dynamicRequestSize;
static OI_UINT32 dynamicRequestCount;

/*
 * The largest total of requested memory.
 */

static OI_UINT32 dynamicHighWaterMark;
static OI_UINT32 dynamicHighCount;

/**
 * The total size of the memory allocated by OI_StaticMalloc.
 */

static OI_UINT32 staticMemSize;


/**
 * Tracks actual allocations to provide a workable pool configuration.
 */
typedef struct {
    OI_UINT16 sizeL;      /* lower bound of pool size */
    OI_UINT16 sizeH;      /* upper bound of pool size */
    OI_UINT8 currentUses; /* current allocation count */
    OI_UINT8 maxUses;     /* high-water mark of allocations of this size */
} ALLOC_TRACKER;


#ifdef  SMALL_RAM_MEMORY
#define MAX_POOLS      32
#else
#define MAX_POOLS      1024
#endif

static ALLOC_TRACKER pools[MAX_POOLS];

static OI_UINT8 MasterPool[MAX_POOLS];
static OI_UINT8 PromotePool[MAX_POOLS];

/* Round size up to nearest 4 byte boundary */
/* Minimum size is 4 bytes, as implemented by oi_memmgr.c */
#define ROUND_SIZE(s)  ( (s) ? ((OI_UINT16) (((s) + 3) & ~(0x3))) : 4)

#ifndef _WIN32
#define NEAR(x, y) \
    (((x) >= (y)) ? (((x) - (y)) <= ((y) >> 1)) : (((y) - (x)) <= ((x) >> 1)))
#endif /* _WIN32 */


#define SIZE_IN_RANGE(sz, p)  ((((sz) >= (p).sizeL)) && (((sz) <= (p).sizeH) || NEAR((sz), (p).sizeH)))


static OI_INT poolCount;

#ifdef MEMMGR_DEBUG
static OI_INT debug_overhead;
#endif




OI_STATUS OI_MemProf_Init(OI_INT overhead)
{
    OI_MemZero(MasterPool, sizeof(MasterPool));
    OI_MemZero(PromotePool, sizeof(PromotePool));
    OI_MemZero(moduleMemUsage, sizeof(moduleMemUsage));
    OI_MemZero(pools, sizeof(pools));

#ifdef MEMMGR_DEBUG
    debug_overhead = overhead;
#endif

    dynamicRequestSize = 0;
    dynamicRequestCount = 0;
    dynamicHighWaterMark = 0;
    dynamicHighCount = 0;
    staticMemSize = 0;
    poolCount = 0;
    return OI_OK;
}


void OI_MemProf_StaticMalloc(OI_INT32 size,
                             OI_UINT8 module,
                             OI_CHAR *filename)
{
    UNREFERENCED_PARAMETER(filename);

    moduleMemUsage[module].staticSize += ROUND_SIZE(size);

    /* Total */
    staticMemSize += ROUND_SIZE(size);
}


void OI_MemProf_Malloc(OI_INT32 size,
                       void* addr,
                       OI_UINT8 module,
                       OI_CHAR *fileName)
{
    UNREFERENCED_PARAMETER(addr);

    OI_UINT16 poolSize;
    OI_INT i;
    OI_INT j;
    OI_INT k;
    OI_INT borrow = 0;
    OI_BOOL   interesting = FALSE;

    if ( module == OI_MODULE_UNKNOWN ){
        DBG_PRINT1(("Unknown Module \"%s\" allocated %ld ", fileName, size));
    }

    moduleMemUsage[module].dynamicSize += size;
    if (moduleMemUsage[module].dynamicSize >  moduleMemUsage[module].dynamicSizeMax) {
        moduleMemUsage[module].dynamicSizeMax = moduleMemUsage[module].dynamicSize;
    }

    /* Total Size */
    if ((dynamicRequestSize += size) > dynamicHighWaterMark) {
        dynamicHighWaterMark = dynamicRequestSize;
        DBG_PRINT2(("Peak dynamic memory: %ld ", dynamicHighWaterMark));
        interesting = TRUE;
    }

    /* Total Count */
    dynamicRequestCount++;
    if (dynamicRequestCount > dynamicHighCount) {
        dynamicHighCount = dynamicRequestCount;
        DBG_PRINT2(("Peak Buffers Allocated: %ld ", dynamicHighCount));
        interesting = TRUE;
    }

    /*
     * Round size up to nearest 8 byte boundary.
     */
    poolSize = ROUND_SIZE(size);

    /* Per-pool count */
    for (i = 0; i < MAX_POOLS; ++i) {
        if (poolCount < MAX_POOLS){
            /* Force new pool if unique */
            if ( pools[i].sizeH == 0 ){
                /* If larger than largest pool, create new at end */
                pools[i].sizeL = pools[i].sizeH = poolSize;
                pools[i].maxUses = pools[i].currentUses = 1;
                MasterPool[i] = PromotePool[i] = 0;
                poolCount++;
                interesting = TRUE;
                break;
            }
            else if ( poolSize > pools[i].sizeH ){
                /* Keep Looking */
                continue;
            }
            else if (poolSize < pools[i].sizeL){
                /* Create new pool Here */
                for (j = poolCount-1; j >= i; --j) {
                    pools[j + 1] = pools[j];
                    MasterPool[j + 1] = MasterPool[j];
                    PromotePool[j + 1] = PromotePool[j];
                }
                pools[i].sizeL = pools[i].sizeH = poolSize;
                pools[i].maxUses = pools[i].currentUses = 1;
                MasterPool[i] = PromotePool[i] = 0;
                poolCount++;
                interesting = TRUE;
                break;
            }
            /* Else, stuff into equivilent sized pool */
        }
        if (i < poolCount) {
            /*
             * If this is the pool, increment the usage count.
             */
            if ( (poolSize <= pools[i].sizeH) || (i == (MAX_POOLS - 1)) ) {
                /*
                 * Check we are not running into the next pool.
                 */
                if (((i + 1) < poolCount) && poolSize >= pools[i + 1].sizeL) {
                    continue;
                }
                if ((++pools[i].currentUses) > pools[i].maxUses) {
                    pools[i].maxUses = pools[i].currentUses;
                    interesting = TRUE;
                }
                if (poolSize < pools[i].sizeL) {
                    pools[i].sizeL = poolSize;
                }
                if (poolSize > pools[i].sizeH) {
                    pools[i].sizeH = poolSize;
                }
                break;
            }
            /*
             * Pools are sorted in order of increasing size.
             */
            if (poolSize > pools[i].sizeH) {
                continue;
            }
            /*
             * Move existing pools to make room.
             */
            for (j = poolCount-1; j >= i; --j) {
                pools[j + 1] = pools[j];
                MasterPool[j + 1] = MasterPool[j];
                PromotePool[j + 1] = PromotePool[j];
            }
        }
        MasterPool[i] = PromotePool[i] = 0;
        pools[i].sizeL = pools[i].sizeH = poolSize;
        pools[i].currentUses = pools[i].maxUses = 1;
        interesting = TRUE;
        ++poolCount;
        break;
    }

    interesting = TRUE;
    if ( interesting ){
        j = 0;
        k = 0;
        for (i=(poolCount-1); i >= 0; i--){

            /* See if we have under-run on buffers */
            j += MasterPool[i];
            j -=  pools[i].currentUses;
            if ( j < 0 ){
                MasterPool[i]++;
                j++;
            }

            /* See if any Promotions can be made if pool count increases */
            k += PromotePool[i];
            k -=  pools[i].currentUses;
            if ( k < 0 ){
                DBG_PRINT2(("Borrow For: %d ", pools[i].sizeH));
                PromotePool[i]++;
                k++;
                borrow++;
            }
            else if ( (k > 0) && borrow && PromotePool[i] ){
                DBG_PRINT2(("Borrow From: %d ", pools[i].sizeH));
                PromotePool[i]--;
                borrow--;
                k--;
            }
        }
    }

	DBG_PRINT1(("\n"));

    if (i == MAX_POOLS) {
        DBG_PRINT1(("Pool usage could not be recorded for size = %d\n", poolSize));
    }

}


void OI_MemProf_Free(OI_INT32 size,
                     void* free,
                     OI_UINT8 module,
                     OI_CHAR *filename)
{
    OI_INT i;
    OI_UINT16 poolSize;

    UNREFERENCED_PARAMETER(free);
    UNREFERENCED_PARAMETER(filename);

    /* Module count */
    OI_ASSERT( (OI_INT)moduleMemUsage[module].dynamicSize >= size);
    moduleMemUsage[module].dynamicSize -= size;

    /* Total count */
    if ((OI_INT)dynamicRequestSize < size) {
        DBG_PRINT1(("dynamicRequestSize <= size (%d < %d)", (OI_INT)dynamicRequestSize, size));
    }
    dynamicRequestSize -= size;
    dynamicRequestCount--;

    poolSize = ROUND_SIZE(size);

    /* Per-pool count */
    for (i = 0; i < poolCount; ++i) {
        if ((poolSize >= pools[i].sizeL) && (poolSize <= pools[i].sizeH)) {
            if (pools[i].currentUses == 0) {
                DBG_PRINT1(("Underflow in pool %d", size));
                continue;
            }
            --pools[i].currentUses;
            break;
        }
    }
}


#ifdef MEMMGR_DEBUG


void OI_MEMMGR_Dump(void)
{
    static const OI_CHAR *spaces = "                ";
    OI_UINT16 i,j;
    OI_UINT totalPoolUse = 0;
    OI_UINT Overhead = 0;
    OI_UINT DebugOverhead;
    OI_UINT coalesce;

    DBG_PRINT2(("**********************************************\n"));
    DBG_PRINT2(("Maximum Dynamic memory pools used\n"));
    for (i = 0, j = 0; i < poolCount; ++i) {
        DBG_PRINT2(("{ %4d, %4d},\n", pools[i].maxUses,pools[i].sizeH));
        totalPoolUse +=  pools[i].maxUses * pools[i].sizeH;
        j += pools[i].maxUses;
    }
    Overhead = 12 * poolCount;
    DBG_PRINT2(("\nTotal pool usage = %d + %d = %d versus optimal = %ld\n", totalPoolUse, Overhead, (Overhead+totalPoolUse), dynamicHighWaterMark));
    DBG_PRINT2(("\nTotal pool Buffers = %d versus Max allocations = %ld\n", j, dynamicHighCount));
    DBG_PRINT2(("**********************************************\n"));

    /* if ( OI_CheckDebugControl(OI_MODULE_MEMMGR, 0, OI_DBG_MSG_PRINT_ENABLE) ) */
    {
        DBG_PRINT2(("Dynamic Pool Recommendation #1 - Promotion Allowed\n"));
        totalPoolUse = 0;
        Overhead = 0;
        for (i = 0, j = 0; i < poolCount; ++i) {
            if ( MasterPool[i] ){
                DBG_PRINT2(("{ %4d, %4d},\n", MasterPool[i],pools[i].sizeH));
                totalPoolUse +=  MasterPool[i] * pools[i].sizeH;
                j += MasterPool[i];
                Overhead++;
            }
        }

        Overhead *= 12;
        DBG_PRINT2(("\nTotal pool usage = %d + %d = %d versus optimal = %ld\n", totalPoolUse, Overhead, (Overhead+totalPoolUse), dynamicHighWaterMark));
        DBG_PRINT2(("\nTotal pool Buffers = %d versus Max allocations = %ld\n", j, dynamicHighCount));
        DBG_PRINT2(("**********************************************\n"));
        DBG_PRINT2(("Dynamic Pool Recommendation #2 - Promotion Required\n"));
        totalPoolUse = 0;
        Overhead = 0;
        for (i = 0, j = 0; i < poolCount; ++i) {
            if ( PromotePool[i] ){
                DBG_PRINT2(("{ %4d, %4d},\n", PromotePool[i],pools[i].sizeH));
                totalPoolUse +=  PromotePool[i] * pools[i].sizeH;
                j += PromotePool[i];
                Overhead++;
            }
        }

        Overhead *= 12;
        DBG_PRINT2(("\nTotal pool usage = %d + %d = %d versus optimal = %ld\n", totalPoolUse, Overhead, (Overhead+totalPoolUse), dynamicHighWaterMark));
        DBG_PRINT2(("\nTotal pool Buffers = %d versus Max allocations = %ld\n", j, dynamicHighCount));
        DBG_PRINT2(("**********************************************\n"));
    }

    DBG_PRINT2(("Highly Optimized Dynamic Pool Recommendation (Minimums)\n"));
    totalPoolUse = 0;
    Overhead = 0;
    coalesce = 0;
    for (i = 0, j = 0; i < poolCount; ++i) {
        coalesce += PromotePool[i];
        if ( coalesce ){
            if ((12 >= (coalesce * (pools[i+1].sizeH - pools[i].sizeH))) && PromotePool[i+1]){
                /* coalesce */
            }
            else if ((12 >= (coalesce * (pools[i+2].sizeH - pools[i].sizeH))) && PromotePool[i+2]){
                /* coalesce */
            }
            else{
                DBG_PRINT2(("{ %4d, %4d},\n", coalesce,pools[i].sizeH));
                totalPoolUse +=  coalesce * pools[i].sizeH;
                j += (OI_UINT16)coalesce;
                Overhead++;
                coalesce = 0;
            }
        }
    }

    DebugOverhead = 16 * Overhead; /* Extra debug OI_UINT32 in debug pools */
    Overhead *= 12;
    DBG_PRINT2(("\nTotal pool usage = %d + %d = %d versus optimal = %ld\n", totalPoolUse, Overhead, (Overhead+totalPoolUse), dynamicHighWaterMark));
    DBG_PRINT2(("\nTotal pool Buffers = %d versus Max allocations = %ld\n", j, dynamicHighCount));
    DBG_PRINT2(("**********************************************\n"));


    DBG_PRINT1(("\nActual memory usage by module\n"));
    DBG_PRINT1(("Module          Static  Dynamic  Dyn Max  Total  Max Total\n"));
    DBG_PRINT1(("------          ------  -------  -------  -----  ---------\n"));

    for (i = 0; i < OI_NUM_MODULES; i++) {
        OI_INT staticSize;
        OI_INT dynamicSize;
        OI_INT dynamicSizeMax;

        staticSize = moduleMemUsage[i].staticSize;
        dynamicSize = moduleMemUsage[i].dynamicSize;
        dynamicSizeMax = moduleMemUsage[i].dynamicSizeMax;

        /* if ( !OI_CheckDebugControl((OI_MODULE)i, 0, OI_DBG_MSG_PRINT_ENABLE)) */
        {
            /* If debugging turned on for this module, don't supress output */
            while (OI_Strcmp(OI_ModuleToString((OI_MODULE)i), OI_ModuleToString((OI_MODULE)(i + 1))) == 0) {
                staticSize += moduleMemUsage[i+1].staticSize;
                dynamicSize += moduleMemUsage[i+1].dynamicSize;
                dynamicSizeMax += moduleMemUsage[i+1].dynamicSizeMax;
                i++;
            }
        }

        if ((staticSize + dynamicSizeMax) /*|| OI_CheckDebugControl((OI_MODULE)i, 0, OI_DBG_MSG_PRINT_ENABLE)*/) {
            DBG_PRINT1(("%s%s%5d    %5d    %5d  %5d      %5d\n",
                      OI_ModuleToString((OI_MODULE)i),
                      &spaces[OI_StrLen(OI_ModuleToString((OI_MODULE)i))],
                      staticSize,
                      dynamicSize,
                      dynamicSizeMax,
                      staticSize + dynamicSize,
                      staticSize + dynamicSizeMax));
        }
    }

    if ( moduleMemUsage[OI_MODULE_UNKNOWN].staticSize || moduleMemUsage[OI_MODULE_UNKNOWN].dynamicSizeMax  ){
        DBG_PRINT1(("%s%s%5d    %5d    %5d  %5d      %5d\n",
                  "UNKNOWN",
                  &spaces[7],
                  moduleMemUsage[OI_MODULE_UNKNOWN].staticSize,
                  moduleMemUsage[OI_MODULE_UNKNOWN].dynamicSize,
                  moduleMemUsage[OI_MODULE_UNKNOWN].dynamicSizeMax,
                  moduleMemUsage[OI_MODULE_UNKNOWN].staticSize + moduleMemUsage[OI_MODULE_UNKNOWN].dynamicSize,
                  moduleMemUsage[OI_MODULE_UNKNOWN].staticSize + moduleMemUsage[OI_MODULE_UNKNOWN].dynamicSizeMax));
    }
	DBG_PRINT1(("\n"));
    DBG_PRINT2(("Current dynamic size: %6ld\n", dynamicRequestSize));
    DBG_PRINT2((  "Current dynamic bufs: %6ld\n", dynamicRequestCount));
    DBG_PRINT2((  "Max optimal dynamic:  %6ld\n", dynamicHighWaterMark));
    DBG_PRINT2((  "Max optimal buffers:  %6ld\n", dynamicHighCount));
    DBG_PRINT2((  "Total static:         %6ld\n", staticMemSize));
    DBG_PRINT2((  "Max total:            %6ld\n", (dynamicHighWaterMark + staticMemSize) ));
    DBG_PRINT2((  "Min Heap (Release):   %6ld  (Minimal pools plus overhead and static)\n",
              ( Overhead + totalPoolUse + staticMemSize) ));
    DBG_PRINT2((  "Min Heap (Debug):     %6ld  (Minimal pools plus overhead and static)\n\n",
              ( (debug_overhead * j ) + DebugOverhead + totalPoolUse + staticMemSize) ));

}

#endif /* MEMMGR_DEBUG */

#endif /* MEMMGR_PROFILE */
