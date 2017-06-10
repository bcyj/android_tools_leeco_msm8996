#ifndef _OI_SUPPORT_INIT_H
#define _OI_SUPPORT_INIT_H

/*============================================================================
  @file oi_support_init.h

  @brief
  This contains the type definition for the memory manager.


  DEPENDENCIES: None.

  **********************************************************************************
  $AccuRev-Revision: 344/2 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
  **********************************************************************************


Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
  ============================================================================*/

/*============================================================================
  EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/core/sensors/dsps/common/main/latest/util/memmgr/inc/oi_support_init.h#3 $
  $DateTime: 2012/01/12 13:59:33 $

  when       who    what, where, why
  ---------- --- -----------------------------------------------------------
  2012-01-12 br  Inserted "fragId" field into OI_MEMMGR_POOL_CONFIG structure

  ============================================================================*/

#include "oi_stddefs.h"
#include "oi_status.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This global variable allows any BM3 function to verify that the current process does
 * in fact own the stack token.
 */
extern OI_BOOL OI_StackTokenHeld;

/**
 * This function initializes the core support library.
 * @ingroup Misc
 */
extern OI_STATUS OI_Support_Init(void);


/** \addtogroup MemMgr Memory Manager APIs */
/**@{*/


#ifdef USE_NATIVE_MALLOC

#define OI_MEMMGR_Init(x) (OI_OK)

#else

/*****************************************************
 * Memory Manager configuration parameters
 *****************************************************/

/**
    This structure configures one memory pool, including the
    number and size of blocks in the pool. Structures of this
    type are used as elements in an array that indicates how the
    Memory Manager will partition the portion of the heap from
    which it dynamically allocates memory. A memory pool is a
    linked list of memory blocks of the same size. The pool
    table is an array of memory pools.

    This structure configures one memory pool. The first field
    indicates the number of blocks that will be allotted in a
    pool; the second field indicates the size of the blocks in
    this pool. For example, the entry {100, 64} indicates that a
    pool of 100 64-byte blocks will be set aside. The block
    sizes must be multiples of 4.
 */
typedef struct {
    OI_UINT16 numBlocks;                /**< Number of blocks that will be allocated in this pool */
    OI_UINT32 blockSize;                /**< Size of each block in this pool */
    OI_UINT32 fragId;                   /**< The heap frag Id that this pool is belong to */
} OI_MEMMGR_POOL_CONFIG;


/**
   This structure configures the heap, setting a pointer to the
   heap and establishing the heap size. All memory allocated by
   the Memory Manager for a particular application comes from a
   single heap. This includes statically allocated memory and
   the memory pools. The structure is eventually sent to
   OI_MEMMGR_Init() by way of OI_CONFIG_MEMMGR.
 */
typedef struct  {
    void        *heap ;         /**< Pointer to the heap */
    OI_UINT32   heapSize ;      /**< Size of the heap in bytes */
} OI_MEMMGR_HEAP_CONFIG ;

/** A single structure of this type is used for passing all memory configuration
 *  information to OI_MEMMGR_Init().
 */
typedef struct {
    const OI_MEMMGR_POOL_CONFIG  *poolConfig ;       /**< Pointer to zero-terminated list of pool-configuration pairs */
    const OI_MEMMGR_HEAP_CONFIG  *heapConfig ;      /**< Pointer to heap configuration information */
} OI_CONFIG_MEMMGR ;

/**
 * Pointer to a function to handle out-of-memory conditions. A failed call to allocate
 * memory will call this function, and then try again.
 */
typedef void (*OI_LOWMEM_CB)(void);

/**
 * This function initializes the Memory Manager. See the @ref memMgr_docpage section
 * for more details.
 *
 * This function will fail if the actual memory required by the Memory Manager is
 * greater than the defined heapSize (@ref OI_MEMMGR_HEAP_CONFIG) or if there are too
 * many pools declared (more than MEMMGR_MAX_POOL_COUNT, which is defined internal
 * to the protocol stack and is currently 1000).
 *
 *   @param pConfig     a pointer to the Memory Manager configuration parameters
 *   @param pLowmemCb   pointer to a function to free memory in OOM conditions, may
 *                      be NULL to disable this functionality.
 *
 *   @return            status of Memory Manager configuration:
 *                      - OI_OK: successful
 *                      - Any other value indicates initialization failure and
 *                        is likely to be a fatal error that will prevent the
 *                        memory manager from operating properly.
 */
extern OI_STATUS OI_MEMMGR_Init(const OI_CONFIG_MEMMGR *pConfig, OI_LOWMEM_CB pLowmemCb);

/**
 * This function de-initializes the Memory Manager. See the @ref memMgr_docpage section
 * for more details.
 *
 * This function will fail if the Memory Manager is not initialized.
 *
 *   @return            status of Memory Manager configuration:
 *                      - OI_OK: successful
 *                      - Any other value indicates de-initialization failure and
 *                        is likely to be a fatal error that will prevent the
 *                        memory manager from operating properly.
 */
extern OI_STATUS OI_MEMMGR_DeInit();

#endif


/*****************************************************************************/
/**@}*/

#ifdef __cplusplus
}
#endif

#endif /* _OI_SUPPORT_INIT_H */

