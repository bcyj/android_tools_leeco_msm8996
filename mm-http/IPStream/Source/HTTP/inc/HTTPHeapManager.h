#ifndef _HTTP_HEAP_MANAGER_H_
#define _HTTP_HEAP_MANAGER_H_

/************************************************************************* */
/**
 * HTTPHeapManager.h
 * @brief Header file for HTTPHeapManager. The class defines a wrapper for
 *   system heap allocation for http segment data.
 *  (i) It ensures that system heap allocation is performed infrequently.
 *  (ii) The managed heap does not suffer from fragementation. This is
 *      achieved by providing only fixed sized memory chunks to the client
 *      upon request which causes fixed chunk deallocations as well from the
 *      client. So, there is no need of any algorithm to find a suitable
 *      memory chunk upon request as all free chunks are of the same size.
 *  (iii) It should also handle temporary spikes in memory requests well.
 *        The strategy is to find a free chunk is to find a free chunk from
 *        a heap allocation unit that is the most heavily used.
 *
 *  The max allocatable limit of heap memory is divided into equal sized
 *  allocatable units each of which corresponds to an operating system
 *  'malloc' ofr 'free'. Each allocatable unit is futhur divided into logical
 *  fixed-sized units (chunks) of memory that are privided to the client
 *  upon request. Each logical unit of memory is pre-prepended by 8 bytes
 *  meta-data to identify the the allocatable unit from which it originated.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPHeapManager.h#8 $
$DateTime: 2012/07/12 04:09:16 $
$Change: 2584374 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPHeapManager.h
** ======================================================================= */

#include "IPStreamSourceUtils.h"
#include "StreamQueue.h"
#include "assert.h"

/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

namespace video
{

class HTTPHeapManager
{
public:
  // Meta data at the start of each memory unit to identify the location.
  static const int LOGICAL_UNIT_META_DATA_SZ = 8;

  // memory chunk size provided to client upon request.
  static const int LOGICAL_UNIT_MEM_SZ = 15 * 1024;

  // operating system alloc size =
  //  (LOGICAL_UNIT_MEM_SZ * NUM_LOGICAL_UNITS_PER_ALLOC)
  static const int NUM_LOGICAL_UNITS_PER_ALLOC = 75;

  // Max number of heap allocations.
  static const int MAX_REGIONS = 33;

  /**
   * c'tor
   */
  HTTPHeapManager();

  /**
   * d'tor
   */
  ~HTTPHeapManager();

  /**
   * Return the chunk size supported by the heap manager. This
   * is a constant.
   */
  static int64 GetChunkSize();

  /**
   * Get the max space that can be allocated from the heap.
   */
  static int GetMaxAllocatableSpace();

  /**
   * Get the max remaining space that the heap manager will try to
   * allocate.
   */
  int GetMaxAvailableSpace();

  /**
   * Provide a fixed sized chunk of memory to the client.
   */
  byte* Allocate();

  /**
   * Return a a fixed sized chunk of memory back to
   * HTTPHeapManager.
   */
  void Deallocate(byte* pMem);

  // Debugging.
  void Print();

private:
  HTTPHeapManager(const HTTPHeapManager&);
  HTTPHeapManager& operator=(const HTTPHeapManager&);

  /**
   * LogicalUnit represents a memory unit that is provided to the
   * HTTPHeapManager client.
   */
  class LogicalUnit
  {
  public:
    StreamQ_link_type m_link;

    /**
     * c'tor
     */
    LogicalUnit();

    /**
     * d'tor
     */
    ~LogicalUnit();

    /**
     * Called when this instance is no longer associated with heap
     * memory eg initially or when all logical units associated with
     * a contigious region of heap memory have been deallocated.
     */
    void Clear();

    /**
     * Called when this instance is associated with a part of heap
     * memory. ie when there is a heap allocation for a HeapAllocationUnit
     * in which case all LogicalUnits assocaited with it need to be
     * 'set'.
     */
    void Set(byte *pBuf, uint16 logicalUnitIdx);

    /**
     * Return pointer to memory associatied with this instance. Will
     * be null if not associated with heap memory.
     */
    byte *GetPtr();

    /**
     * Return the array idx in m_LogicalUnitArray which this
     * instance corresponds to. Will be 0xffff if not associated
     * with heap memory.
     */
    uint16 GetLogicalUnitIdx() const;

  private:
    // When non-null, points to the start of heap memory represented by this
    // instance. When null, not associated with any heap memory. It is null
    // only when it is in the free-pool and there is no heap allocation
    // associated with the free-pool. If any element in the free-pool is not
    // associated with heap memory, then all elements in the free-pool are not
    // associated with heap memory.
    byte *m_pStartAddr;

    // Represents the index (key) of this logical unit in the
    // m_LogicalUnitArray defined in HeapAllocationUnit.
    uint16 m_LogicalUnitIdx;
  };

  /**
   * Represents a contigious region of heap memory obtained by a
   * QTV_Malloc.
   */
  class HeapAllocationUnit
  {
  public:
    /**
     * c'tor.
     *  Initialize member variables. Construct the free-pool
     *  consisting of a list of LogicalUnits that are not associated
     *  with heap memory.
     */
    HeapAllocationUnit();

    /**
     * d'tor
     */
    ~HeapAllocationUnit();

    /**
     * Free the contigious heap memory associated with this instance
     * and reset member variables. All LogicalUnits will be in the
     * free-pool and not associated with any heap memory.
     */
    void Clear();

    /**
     * Return a unit of heap memory associated with a LogicalUnit to
     * the client. If there is heap memory associated with this
     * instance, will allocate heap memory, associate LogicalUnits
     * with it, and return memory associated with a LogicalUnit.
     */
    bool Allocate(byte*& pMem, uint16& rLogicalUnitIdx);

    /**
     * Return a logicalUnit back to a HeapAllocationUnit. The
     * 'logicalUnitIdx' identified the array idx in
     * m_LogicalUnitArray. If all LogicalUnits associated with this
     * instance are deallocated, then the heap memory associated
     * with this instance is returned to the operating system. All
     * logicalUnits are then 'clear'ed so that they are not
     * associaed with heap memory.
     */
    void Deallocate(uint16 logicalUnitIdx);

    // Debugging
    void Print();

  private:
    /**
     * Return the number of elements in the free pool.
     */
    int GetFreeCount();

    // Associated with memory which can be returned on an 'Allocate' if
    // m_pHeapAddr is non-null.
    StreamQ_type m_FreePool;

    // Associated with memory that is in-use by the client.
    StreamQ_type m_UsedPool;

    // non-null when heap malloc was done for this instance. Then, all
    // LogicalUnits should be properly 'set'.
    byte *m_pHeapAddr;

    // Each unit represents a unit that can be provided to the client. As a
    // whole, represents the entire contigious memory associated with m_pHeapAddr.
    LogicalUnit m_LogicalUnitArray[NUM_LOGICAL_UNITS_PER_ALLOC];

    friend class HTTPHeapManager;
  };

  // An array for which each element represents a contigious region of
  // heap memory.
  HeapAllocationUnit m_HeapAllocationUnitArray[MAX_REGIONS];

  const int64 m_ChunkSize;
  MM_HANDLE m_pDataLock;
};

} // end namespace video

#endif
