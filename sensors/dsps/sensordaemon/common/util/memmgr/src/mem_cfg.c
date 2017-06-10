
/*============================================================================
  @file mem_cfg.c

  @brief
  This contains the configuration for the memory manager.

  <br><br>

  DEPENDENCIES: None.

  Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ============================================================================*/

/*============================================================================
  EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. Please
  use ISO format for dates.

  $Header: //source/qcom/qct/core/sensors/dsps/apps/la/main/latest/sensordaemon/util/memmgr/src/mem_cfg.c#3 $
  $DateTime: 2011/11/06 13:53:34 $

  when       who    what, where, why
  ---------- --- -----------------------------------------------------------
  2012-02-23 pd  Increase memory by adding two 5K blocks for Gyro Tap
  2012-01-13 jhh Apply fragmented heap memory region implemetatoin in apps
  2011-12-06 jh  Increase size of 512B and 1kB memory pool
  2011-11-03 jtl Split file to apps processor. Increased heap size.
  2011-09-02 sd  Recofigured the memory blocks to support multiple requests
  2011-07-26 br  Recofigured the memory blocks to support multiple requests
  2011-07-12 br  Shuffled the memory blocks to enable init to succeed
  2011-07-01 sj  Increase heap memory by 2kb as SMGR allocs were failing
  2011-06-11 sd  Increase Heap Memory for 2kb which is needed when power features are enabled
  2011-05-05 rk  Changed the max memory blocks for 128bytes to fit into 18KB
  2011-05-05 rk  reduced the heap memory size from 25KB to 18KB
  2011-04-27 br  increased heap memory size

  ============================================================================*/



/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#ifndef USE_NATIVE_MALLOC
#include "mem_cfg.h"
#include "oi_assert.h"

/** The heap is declared to be an array of elements of type OI_UINT32, with
 *  a size of @ref MEMMGR_TOTAL_HEAP_SIZE. OI_UINT32 elements ensure proper
 *  byte alignment on all platforms.
 */
OI_UINT32 OI_memmgrHeap_frag0[MEMMGR_TOTAL_HEAP_FRAG0_SIZE/sizeof(OI_UINT32)];

/**
 *  All memory allocated by the Memory Manager comes from a single heap. This includes
 *  statically allocated memory and the memory pools. This structure configures the heap,
 *  setting a pointer to the heap and establishing the heap size. This structure is
 *  eventually sent to OI_MEMMGR_Init() by way of OI_CONFIG_MEMMGR.
 */
const OI_MEMMGR_HEAP_CONFIG    heapConfig[] =
{
    {(void*)OI_memmgrHeap_frag0,      /**< pointer to the fraged heap */
    MEMMGR_TOTAL_HEAP_FRAG0_SIZE}    /**< size of the heap */
};

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
#define NUM_16_BYTE_BLOCKS  16    /* 256B */ 
#define NUM_24_BYTE_BLOCKS  32    /* 768B */
#define NUM_32_BYTE_BLOCKS  28    /* 896B */
#define NUM_64_BYTE_BLOCKS  46    /* 2KB + 896B */
#define NUM_128_BYTE_BLOCKS 64    /* 8KB */
#define NUM_256_BYTE_BLOCKS 15    /* 3.8KB */
#define NUM_512_BYTE_BLOCKS 24    /* 12KB */
#define NUM_1024_BYTE_BLOCKS 18   /* 18KB */
#define NUM_2048_BYTE_BLOCKS 20   /* 40KB */
#define NUM_5120_BYTE_BLOCKS 2    /* 10KB */
#define NUM_12288_BYTE_BLOCKS 1   /* 12KB */

const OI_MEMMGR_POOL_CONFIG memmgrPoolTable[] =
{
    {NUM_16_BYTE_BLOCKS,    16,        0},
    {NUM_24_BYTE_BLOCKS,    24,        0},
    {NUM_32_BYTE_BLOCKS,    32,        0},
    {NUM_64_BYTE_BLOCKS,    64,        0},
    {NUM_128_BYTE_BLOCKS,   128,       0},
    {NUM_256_BYTE_BLOCKS,   256,       0},
    {NUM_512_BYTE_BLOCKS,   512,       0},
    {NUM_1024_BYTE_BLOCKS,  1024,      0},
    {NUM_2048_BYTE_BLOCKS,  2048,      0},
    {NUM_5120_BYTE_BLOCKS,  5120,      0},
    {NUM_12288_BYTE_BLOCKS, 12288,     0},
    {0, 0}
};

/*  This single structure is used for passing all memory configuration
 *  information to OI_MEMMGR_Init().
 */
const OI_CONFIG_MEMMGR  oi_default_config_MEMMGR =
{
    &memmgrPoolTable[0],
    (const OI_MEMMGR_HEAP_CONFIG*)&heapConfig[0],
};
#endif /* USE_NATIVE_MALLOC */
