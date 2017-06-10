/************************************************************************* */
/**
 * HTTPHeapManager.cpp
 * @brief Source file for HTTPHeapManager. The class defines a wrapper for
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

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPHeapManager.cpp#10 $
$DateTime: 2012/07/16 09:24:33 $
$Change: 2594762 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "IPStreamSourceUtils.h"
#include "HTTPHeapManager.h"
#include "qtv_msg.h"
#include "SourceMemDebug.h"

namespace video
{

/**
 * c'tor
 */
HTTPHeapManager::HTTPHeapManager() :
  m_ChunkSize(HTTPHeapManager::GetChunkSize())
{
  m_pDataLock = NULL;
  MM_CriticalSection_Create(&m_pDataLock);
}

/**
 * d'tor
 */
HTTPHeapManager::~HTTPHeapManager()
{
  if(m_pDataLock)
  {
    MM_CriticalSection_Release(m_pDataLock);
    m_pDataLock = NULL;
  }
}

int64 HTTPHeapManager::GetChunkSize()
{
  return (int64)LOGICAL_UNIT_MEM_SZ;
}

/**
 * Get the max space that can be allocated from the heap.
 */
int HTTPHeapManager::GetMaxAllocatableSpace()
{
  return MAX_REGIONS * NUM_LOGICAL_UNITS_PER_ALLOC * LOGICAL_UNIT_MEM_SZ;
}

/**
 * Get the max remaining space that the heap manager will try to
 * allocate.
 */
int HTTPHeapManager::GetMaxAvailableSpace()
{
  int freeCount = 0;
  MM_CriticalSection_Enter(m_pDataLock);

  for (int i = 0; i < MAX_REGIONS; ++i)
  {
    freeCount += m_HeapAllocationUnitArray[i].GetFreeCount();
  }
  MM_CriticalSection_Leave(m_pDataLock);

  return freeCount * LOGICAL_UNIT_MEM_SZ;
}

/**
 * Provide a fixed sized chunk of memory to client.
 */
byte* HTTPHeapManager::Allocate()
{
  MM_CriticalSection_Enter(m_pDataLock);
  // Strategy to find a memory is to re-use LogicalUnits from the most
  // heavily used HeapAllocationUnit. So, if there is a temporary spike in
  // memory usage, over time complete HeapAllocationUnits will become free
  // again resulting in memory returned back to the operating system.
  byte *pMem = NULL;

  int freePoolIdx = 0xffff;
  int minFreePoolCount = 0xffff;

  for (int i = 0; i < MAX_REGIONS; ++i)
  {
    int freePoolCount = m_HeapAllocationUnitArray[i].GetFreeCount();

    if ((freePoolCount > 0) && (freePoolCount < minFreePoolCount))
    {
      minFreePoolCount = freePoolCount;
      freePoolIdx = i;
    }
  }

  if ((freePoolIdx >= 0) && (freePoolIdx < MAX_REGIONS))
  {
    uint16 minorKey = 0xffff;

    if (true == m_HeapAllocationUnitArray[freePoolIdx].Allocate(pMem, minorKey))
    {
      if (pMem && minorKey < NUM_LOGICAL_UNITS_PER_ALLOC)
      {
        uint16 majorKey = (uint16)freePoolIdx;

        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
          "HTTPHeapManager::Allocate() Chosen Unit majorKey %d, minorKey %d, pMem %p",
          majorKey, minorKey, (void*)pMem);

        // Write meta-data into the start of the buffer so that the corresponding
        // heap region can be identified on a deallocate().
        char *pMajorKey = (char *)&majorKey;
        char *pMinorKey = (char *)&minorKey;

        pMem[0] = 'M';
        pMem[1] = 'J';
        pMem[2] = *pMajorKey;
        pMem[3] = *(pMajorKey + 1);

        pMem[4] = 'M';
        pMem[5] = 'I';
        pMem[6] = *pMinorKey;
        pMem[7] = *(pMinorKey + 1);

        pMem += LOGICAL_UNIT_META_DATA_SZ;
      }
      else
      {
        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "Unexpected null pMem %p or minorKey %d not < than %d",
                      (void*)pMem, minorKey, NUM_LOGICAL_UNITS_PER_ALLOC);
      }
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "Allocate() Failed to allocate a logical unit for majorKey %d",
        freePoolIdx);
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Allocate() failed. No free-pool available");
  }

  MM_CriticalSection_Leave(m_pDataLock);

  return pMem;
}

/**
 * Return a a fixed sized chunk of memory back to
 * HTTPHeapManager.
 */
void HTTPHeapManager::Deallocate(byte* pMem)
{
  MM_CriticalSection_Enter(m_pDataLock);

  if (pMem)
  {
    pMem -= LOGICAL_UNIT_META_DATA_SZ;

    uint16 majorKey = 0xffff, minorKey = 0xffff;
    char *pMajorKey = (char *)&majorKey;
    char *pMinorKey = (char *)&minorKey;

    if (('M' == pMem[0]) && ('J' == pMem[1]) &&
        ('M' == pMem[4]) && ('I' == pMem[5]))
    {
      pMajorKey[0] = pMem[2];
      pMajorKey[1] = pMem[3];

      pMinorKey[0] = pMem[6];
      pMinorKey[1] = pMem[7];

      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "Deallocate() majorKey %u, minorKey %u, pMem %p",
                    majorKey, minorKey, (void*)pMem);

      m_HeapAllocationUnitArray[majorKey].Deallocate(minorKey);
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "HTTPHeapManager::Deallocate Sanity chcek failed");
    }
  }
  MM_CriticalSection_Leave(m_pDataLock);

}

/**
 * Debugging
 */
void HTTPHeapManager::Print()
{
  QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                "HTTPHeapManager::Print() Available space %d:", GetMaxAvailableSpace());
  MM_CriticalSection_Enter(m_pDataLock);

  for (int i = 0; i < MAX_REGIONS; ++i)
  {
    QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                  "HTTPHeapManager::Print() HeapAllocationUnit %d:", i);
    m_HeapAllocationUnitArray[i].Print();
  }
  MM_CriticalSection_Leave(m_pDataLock);

}

/**
 * c'tor
 */
HTTPHeapManager::LogicalUnit::LogicalUnit()
{
  Clear();
}

/**
 * d'tor
 */
HTTPHeapManager::LogicalUnit::~LogicalUnit()
{

}

/**
 * Called when this instance is no longer associated with heap
 * memory eg initially or when all logical units associated with
 * a contigious region of heap memory have been deallocated.
 */
void HTTPHeapManager::LogicalUnit::Clear()
{
  m_pStartAddr = NULL;
  m_LogicalUnitIdx = 0xffff;
}

/**
 * Called when this instance is associated with a part of heap
 * memory. ie when there is a heap allocation for a HeapAllocationUnit
 * in which case all LogicalUnits assocaited with it need to be
 * 'set'.
 */
void HTTPHeapManager::LogicalUnit::Set(byte *pBuf, uint16 logicalUnitIdx)
{
  m_pStartAddr = pBuf;
  m_LogicalUnitIdx = logicalUnitIdx;
}

/**
 * Return pointer to memory associatied with this instance. Will
 * be null if not associated with heap memory.
 */
byte *HTTPHeapManager::LogicalUnit::GetPtr()
{
  return (m_pStartAddr ? m_pStartAddr : NULL);
}

/**
 * Return the array idx in m_LogicalUnitArray which this
 * instance corresponds to. Will be 0xffff if not associated
 * with heap memory.
 */
uint16 HTTPHeapManager::LogicalUnit::GetLogicalUnitIdx() const
{
  return m_LogicalUnitIdx;
}

/**
 * c'tor.
 *  Initialize member variables. Construct the free-pool
 *  consisting of a list of LogicalUnits that are not associated
 *  with heap memory.
 */
HTTPHeapManager::HeapAllocationUnit::HeapAllocationUnit() :
  m_pHeapAddr(NULL)
{
  (void)StreamQ_init(&m_FreePool);
  (void)StreamQ_init(&m_UsedPool);

  // Initialize the the free pool with NUM_LOGICAL_UNITS_PER_ALLOC elements.
  // The free and used pools can be used only if heap memory was allocated,
  // ie m_pHeapAddr is not null.
  for (int i = 0; i < NUM_LOGICAL_UNITS_PER_ALLOC; ++i)
  {
    (void)StreamQ_link(&m_LogicalUnitArray[i], &m_LogicalUnitArray[i].m_link);
    StreamQ_put(&m_FreePool, &m_LogicalUnitArray[i].m_link);
  }
}

/**
 * d'tor
 */
HTTPHeapManager::HeapAllocationUnit::~HeapAllocationUnit()
{
  // move all elements in the free-pool to the used-pool.
  LogicalUnit *pLogicalUnit = (LogicalUnit *)StreamQ_get(&m_UsedPool);

  while (pLogicalUnit)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "~HeapAllocationUnit() LEAK: '%p' not free'd",
                  (void*)pLogicalUnit->GetPtr());

    pLogicalUnit = (LogicalUnit *)StreamQ_get(&m_UsedPool);
  }

  if (NUM_LOGICAL_UNITS_PER_ALLOC == GetFreeCount())
  {
    Clear();
  }
}

/**
 * Free the contigious heap memory associated with this instance
 * and reset member variables. All LogicalUnits will be in the
 * free-pool and not associated with any heap memory.
 */
void HTTPHeapManager::HeapAllocationUnit::Clear()
{
  // for sanity check.
  static const int SUM_KEYS =
    (NUM_LOGICAL_UNITS_PER_ALLOC - 1) * NUM_LOGICAL_UNITS_PER_ALLOC / 2;

  if (m_pHeapAddr)
  {
    // Reset all elements in the free-pool.
    LogicalUnit *pLogicalUnit = (LogicalUnit *)StreamQ_check(&m_FreePool);

    int sumKeys = 0;

    while (pLogicalUnit)
    {
      sumKeys += pLogicalUnit->GetLogicalUnitIdx();
      pLogicalUnit->Clear();

      pLogicalUnit =
        (LogicalUnit *)StreamQ_next(&m_FreePool, &pLogicalUnit->m_link);
    }

    if (SUM_KEYS == sumKeys)
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "HeapAllocationUnit::Clear() Free'd %p", (void*)m_pHeapAddr);

      // Free heap memory associated with this unit.
      QTV_Free(m_pHeapAddr);
      m_pHeapAddr = NULL;
    }
    else
    {
      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "Unexpected sumKeys %d not equal to %d", sumKeys, SUM_KEYS);
    }
  }
}

/**
 * Return a unit of heap memory associated with a LogicalUnit to
 * the client. If there is heap memory associated with this
 * instance, will allocate heap memory, associate LogicalUnits
 * with it, and return memory associated with a LogicalUnit.
 */
bool HTTPHeapManager::HeapAllocationUnit::Allocate(byte*& pMem,
                                                   uint16& rLogicalUnitIdx)
{
  pMem = NULL;

  if (NULL == m_pHeapAddr)
  {
    static const int logicalUnitSize =
      (HTTPHeapManager::LOGICAL_UNIT_META_DATA_SZ +
       HTTPHeapManager::LOGICAL_UNIT_MEM_SZ);

    static const int allocSize =
      HTTPHeapManager::NUM_LOGICAL_UNITS_PER_ALLOC * logicalUnitSize;

    int usedPoolCnt = StreamQ_cnt(&m_UsedPool);

    if (0 == usedPoolCnt)
    {
      m_pHeapAddr = (byte *)QTV_Malloc(allocSize * sizeof(byte));

      if (NULL != m_pHeapAddr)
      {
        // Set up the free pool.
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                      "HeapAllocationUnit::Allocate() Heap allocation %d bytes %p",
                      allocSize, (void*)m_pHeapAddr);

        // reset the free pool elements using the newly allocated memory.
        LogicalUnit *pLogicalUnit =
          (LogicalUnit *)StreamQ_check(&m_FreePool);

        uint16 rLogicalUnitIdx = 0;
        byte *pBuf = m_pHeapAddr;

        while (pLogicalUnit)
        {
          pLogicalUnit->Set(pBuf, rLogicalUnitIdx);
          pLogicalUnit = (LogicalUnit *)StreamQ_next(
            &m_FreePool, &pLogicalUnit->m_link);

          pBuf += logicalUnitSize;
          ++rLogicalUnitIdx;
        }
      }
    }
    else
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "HeapAllocationUnit::Allocate Unexpected non zero used pool count%d",
        usedPoolCnt);
    }
  }

  if (NULL == m_pHeapAddr)
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "Allocate() Heap allocation failed");
  }
  else
  {
    LogicalUnit *pLogicalUnit = (LogicalUnit *)StreamQ_get(&m_FreePool);

    if (pLogicalUnit)
    {
      pMem = pLogicalUnit->GetPtr();
      if (pMem)
      {
        rLogicalUnitIdx = pLogicalUnit->GetLogicalUnitIdx();
        StreamQ_put(&m_UsedPool, &pLogicalUnit->m_link);
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "HeapAllocationUnit::Allocate Unexpected null pMem");
      }
    }
  }
  return (pMem ? true : false);
}

/**
 * Return a logicalUnit back to a HeapAllocationUnit. The
 * 'logicalUnitIdx' identified the array idx in
 * m_LogicalUnitArray. If all LogicalUnits associated with this
 * instance are deallocated, then the heap memory associated
 * with this instance is returned to the operating system. All
 * logicalUnits are then 'clear'ed so that they are not
 * associaed with heap memory.
 */
void HTTPHeapManager::HeapAllocationUnit::Deallocate(uint16 logicalUnitIdx)
{
  bool isFound = false;

  // locate the element in the used pool corresponding to minor key.
  LogicalUnit *pLogicalUnit = (LogicalUnit *)StreamQ_check(&m_UsedPool);

  while (pLogicalUnit)
  {
    if (pLogicalUnit->GetLogicalUnitIdx() == logicalUnitIdx)
    {
      isFound = true;
      break;
    }

    pLogicalUnit =
      (LogicalUnit *)StreamQ_next(&m_UsedPool, &pLogicalUnit->m_link);
  }

  if (isFound)
  {
    StreamQ_delete(&pLogicalUnit->m_link);
    StreamQ_put(&m_FreePool, &pLogicalUnit->m_link);
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "HeapAllocationUnit::Deallocate Falied to find logicalUnitIdx %d",
      logicalUnitIdx);
  }

  if (NUM_LOGICAL_UNITS_PER_ALLOC == GetFreeCount())
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "HeapAllocationUnit::Deallocate() minorKey %d. All logicalunits free",
      logicalUnitIdx);
    Clear();
  }
}

/**
 * Debugging
 */
void HTTPHeapManager::HeapAllocationUnit::Print()
{
  static char tmpStr[20];
  static char tmpBuf[500];

  QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                "HeapAllocationUnit::Print() #logical units %u/%u (free/use)",
                StreamQ_cnt(&m_FreePool), StreamQ_cnt(&m_UsedPool));

  LogicalUnit *pLogicalUnit = (LogicalUnit *)StreamQ_check(&m_FreePool);

  tmpBuf[0] = '\0';
  std_strlcat(tmpBuf, "FreePool: ", sizeof(tmpBuf));

  while (pLogicalUnit)
  {
    std_strlprintf(tmpStr, sizeof(tmpStr),
                   "*[%u,%p]", pLogicalUnit->GetLogicalUnitIdx(),
                   (void*)pLogicalUnit->GetPtr());

    std_strlcat(tmpBuf, tmpStr, sizeof(tmpBuf));

    pLogicalUnit =
      (LogicalUnit *)StreamQ_next(&m_FreePool, &pLogicalUnit->m_link);
  }

  QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "%s", tmpBuf);

  pLogicalUnit = (LogicalUnit *)StreamQ_check(&m_UsedPool);

  tmpBuf[0] = '\0';
  std_strlcat(tmpBuf, "UsedPool: ", sizeof(tmpBuf));

  while (pLogicalUnit)
  {
    std_strlprintf(tmpStr, sizeof(tmpStr),
                   "*[%u,%p]", pLogicalUnit->GetLogicalUnitIdx(),
                   (void*)pLogicalUnit->GetPtr());

    std_strlcat(tmpBuf, tmpStr, sizeof(tmpBuf));

    pLogicalUnit =
      (LogicalUnit *)StreamQ_next(&m_UsedPool, &pLogicalUnit->m_link);
  }

  QTV_MSG_SPRINTF_PRIO_1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH, "%s", tmpBuf);
}

/**
 * Return the number of elements in the free pool.
 */
int HTTPHeapManager::HeapAllocationUnit::GetFreeCount()
{
  return StreamQ_cnt(&m_FreePool);
}

} // end namespace video
