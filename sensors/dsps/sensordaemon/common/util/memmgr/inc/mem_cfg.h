#ifndef MEMMGR_CONFIG
#define MEMMGR_CONFIG

/*============================================================================
  @file mem_cfg.h

  @brief
  This contains the configuration for the memory manager.

  <br><br>

  DEPENDENCIES: None.

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ============================================================================*/

/*============================================================================
  EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/core/sensors/dsps/apps/la/main/latest/sensordaemon/util/memmgr/inc/mem_cfg.h#3 $ 
  $DateTime: 2011/11/21 19:08:56 $

  when       who    what, where, why 
  ---------- --- -----------------------------------------------------------
  2012-02-23 pd  Increase memory by adding two 5K blocks for Gyro Tap
  2012-01-13 jhh Apply fragmented heap memory region implemetatoin in apps
  2011-12-06 jh  Increase size of 512B and 1kB memory pool
  2011-11-03 jtl Split file to apps processor. Increased heap size.
  2011-07-01 sj  Increase heap memory by 2kb as SMGR allocs were failing  
  2011-06-11 sd  Increase Heap Memory for 2kb which is needed when power features are enabled
  2011-04-27 agk Included oi_string.h to remove a compiler warning.
  2011-04-27 br  increased heap memory size
  2011-02-28 jtl Added header.
  2011-05-05 rk  reduced the heap memory size from 25KB to 18KB to help fit into TCM

  ============================================================================*/


/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#include "oi_stddefs.h"
#include "oi_support_init.h"
#include "oi_memmgr.h"
#include "oi_string.h"


/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

#ifndef USE_NATIVE_MALLOC

#define MEMMGR_HEAP_FRAG_CNT              1
#define MEMMGR_MAX_ALLOWED_POOL_CNT       11

#if defined(MEMMGR_DEBUG) || defined(OI_DEBUG)
    #define MEMMGR_TOTAL_HEAP_FRAG0_SIZE  (122*1024)
#elif defined(SNS_MEMMGR_PROFILE_ON)
    #define MEMMGR_TOTAL_HEAP_FRAG0_SIZE  (118*1024)
#else
    #define MEMMGR_TOTAL_HEAP_FRAG0_SIZE  (110*1024)
#endif

/*============================================================================
 * Global Data Definitions
 ============================================================================*/

/**************************************************
  Memory Manager pool table

    This file defines settings for the Memory Manager heap and memory pool configuration:
        - Small platforms, typically embedded platforms with limited RAM

    These default settings may not be appropriate for a specific application. For those cases,
    the Memory Manager configuration can be defined somewhere else, in which case this file
    will not be linked in.

 **************************************************/

/** The heap is declared to be an array of elements of type OI_UINT32, with
 *  a size of @ref MEMMGR_TOTAL_HEAP_SIZE. OI_UINT32 elements ensure proper
 *  byte alignment on all platforms.
 */
extern OI_UINT32 OI_memmgrHeap_frag0[MEMMGR_TOTAL_HEAP_FRAG0_SIZE/sizeof(OI_UINT32)];

/**
 *  All memory allocated by the Memory Manager comes from a single heap. This includes
 *  statically allocated memory and the memory pools. This structure configures the heap,
 *  setting a pointer to the heap and establishing the heap size. This structure is
 *  eventually sent to OI_MEMMGR_Init() by way of OI_CONFIG_MEMMGR.
 */
extern  const OI_MEMMGR_HEAP_CONFIG    heapConfig[];

/** This array is a table of pairs that indicate how the Memory Manager will partition
    the portion of the heap from which it dynamically allocates memory.
    A memory pool is a linked list of memory blocks of the same size. Each entry                                  
    in this table configures one memory pool: the first field indicates the number of blocks
    that will be allotted in a pool; the second field indicates the size of the blocks in this pool.
    For example, the entry {100, 64} indicates that a pool of 100 64-byte blocks will be set aside.

    Note:
    - The last entry must always be a pair of zeroes.
    - The block sizes must be multiples of 4.
    - The pool table must be ordered such that block sizes are in increasing order.
*/
extern const OI_MEMMGR_POOL_CONFIG memmgrPoolTable[];

/*  This single structure is used for passing all memory configuration
 *  information to OI_MEMMGR_Init().
 */
extern const OI_CONFIG_MEMMGR  oi_default_config_MEMMGR;

#endif /* USE_NATIVE_MALLOC */

#endif
