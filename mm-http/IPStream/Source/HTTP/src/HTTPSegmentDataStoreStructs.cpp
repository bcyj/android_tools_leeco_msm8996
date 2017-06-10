/************************************************************************* */
/**
 * HTTPSegmentDataStoreStructs.cpp
 * @brief implementation of the HTTPSegmentDataStoreStructs.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/src/HTTPSegmentDataStoreStructs.cpp#18 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "HTTPHeapManager.h"
#include "HTTPSegmentDataStoreStructs.h"
#include "SourceMemDebug.h"
#include "qtv_msg.h"

#include "string.h"

namespace video {

/**
 * c'tor.
 */
HttpSegmentDataStoreBase::HttpSegmentDataStoreBase()
{
  ResetBase();
  m_pDataLock = NULL;
  MM_CriticalSection_Create(&m_pDataLock);
}

/**
 * d'tor
 */
HttpSegmentDataStoreBase::~HttpSegmentDataStoreBase()
{
  if(m_pDataLock)
  {
    MM_CriticalSection_Release(m_pDataLock);
    m_pDataLock = NULL;
  }
}

/**
 * Reset base class member variables.
 */
void HttpSegmentDataStoreBase::ResetBase()
{
  m_Key = 0xffffffffffffffffLL;
  m_NumBytesDownloaded = 0;
  m_bIsFullyDownloaded = false;
  m_bIsPurgable = true;
  m_nStartOffset = -1;
}

/**
 * c'tor. Also calls base class's ResetBase.
 */
HttpSegmentDataStoreHeap::HttpSegmentDataStoreHeap() :
  m_pHeapManager(NULL),
  m_ChunkSize(HTTPHeapManager::GetChunkSize())
{
  for (int i = 0; i < MAX_MEMORY_UNITS; ++i)
  {
    m_HeapMemArray[i] = NULL;
  }

  Reset();
}

/**
 * Pass the ptr heap manager instance to the segment heap.
 */
void HttpSegmentDataStoreHeap::SetHeapManager(HTTPHeapManager *pHeapManager)
{
  m_pHeapManager = pHeapManager;
}

/**
 * d'tor
 */
HttpSegmentDataStoreHeap::~HttpSegmentDataStoreHeap()
{

}

/**
 * Reset segment info.
 */
void HttpSegmentDataStoreHeap::Reset()
{
  ResetBase();
  QTV_NULL_PTR_CHECK(m_pHeapManager, RETURN_VOID);

  // Release memory back to the heap manager for this segment.
  for (int i = 0; i < MAX_MEMORY_UNITS; ++i)
  {
    if (m_HeapMemArray[i])
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "Reset() Deallocate %p", (void*)m_HeapMemArray[i]);

      m_pHeapManager->Deallocate(m_HeapMemArray[i]);
      m_HeapMemArray[i] = NULL;
    }
    else
    {
      break;
    }
  }
}

/**
 * Get ptr to buffer to write to for the segment.
 */
bool HttpSegmentDataStoreHeap::GetBuffer(byte* &pBuf, int64& nBufSize)
{
  bool rslt = false;
  pBuf = NULL;
  QTV_NULL_PTR_CHECK(m_pHeapManager, false);

  // Locate the unit array idx from the numbytes downloaded. If at unit
  // boundary allocate from Heap Manager.
  int64 arrayIdx = m_NumBytesDownloaded / m_ChunkSize;

  if (arrayIdx >= MAX_MEMORY_UNITS)
  {
    // non-recoverable error
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "HttpSegmentDataStoreHeap::GetBuffer() maxDownloadCap %lld bytes for segment reached",
      m_NumBytesDownloaded);
  }
  else
  {
    int64 numDownloadedInLogicalUnit = m_NumBytesDownloaded % m_ChunkSize;

    if (0 == numDownloadedInLogicalUnit)
    {
      // need to allocate memory
      if (NULL == m_HeapMemArray[arrayIdx])
      {
        m_HeapMemArray[arrayIdx] = m_pHeapManager->Allocate();
      }

      if (NULL == m_HeapMemArray[arrayIdx])
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "GetBuffer() Failed to allocate memory from heap manager");
        nBufSize = 0;
      }
      else
      {
        QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_LOW,
                      "GetBuffer() Allocate '%p'", (void*)m_HeapMemArray[arrayIdx]);

        pBuf = m_HeapMemArray[arrayIdx];
        nBufSize = m_ChunkSize;
        rslt = true;
      }
    }
    else
    {
      // memory is available, nBufSize carries the max available space
      int64 maxAvailable = m_ChunkSize - numDownloadedInLogicalUnit;

      pBuf = m_HeapMemArray[arrayIdx] + numDownloadedInLogicalUnit;
      nBufSize = maxAvailable;

      rslt = true;
    }
  }

  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "GetBuffer() result %d, pBuf %p, nBufSize %lld",
                rslt, (void*)pBuf, nBufSize);

  return rslt;
}

/**
 * Commit the info associated with the data write on the buffer
 * obtained via GetBuffer for the segment.
 */
bool HttpSegmentDataStoreHeap::CommitBuffer(byte* pBuf, int64 nNumWritten)
{
  bool rslt = false;

  int64 arrayIdx = m_NumBytesDownloaded / m_ChunkSize;

  if (arrayIdx >= MAX_MEMORY_UNITS)
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "HttpSegmentDataStoreHeap::CommitBuffer() maxDownloadCap %lld bytes for segment reached",
      m_NumBytesDownloaded);
  }
  else
  {
    int64 numDownloadedInLogicalUnit = m_NumBytesDownloaded % m_ChunkSize;

    byte *pWritePtr = m_HeapMemArray[arrayIdx] + numDownloadedInLogicalUnit;

    if (true == m_bIsFullyDownloaded)
    {
      QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                    "CommitBuffer() Segment %d is already downloaded", (int)m_Key);
    }
    else
    {
      if (pWritePtr != pBuf)
      {
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                      "CommitBuffer() pBuf %p is not equal to write ptr %p",
                      (void*)pBuf, (void*)pWritePtr);
      }
      else
      {
        if (nNumWritten > m_ChunkSize - numDownloadedInLogicalUnit)
        {
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "CommitBuffer() nNumWritten %lld larger than max allowed %lld",
            nNumWritten, m_ChunkSize - numDownloadedInLogicalUnit);
        }
        else
        {
          m_NumBytesDownloaded += nNumWritten;
          rslt = true;
        }
      }
    }
  }

  return rslt;
}

/**
 * Read at most nBufSize bytes at an offset nOffset from the
 * start of the segment.
 */
int64 HttpSegmentDataStoreHeap::Read(int nOffset, byte* pBuf, int nBufSize)
{
  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
    "HttpSegmentDataStoreHeap::Read() nOffset %d, pBuf %p, nBufSize %d",
    nOffset, (void*)pBuf, nBufSize);

  int64 numRead = 0;

  if ((nOffset >= 0) && (nOffset < m_NumBytesDownloaded))
  {
    bool doLoop = false;

    do
    {
      doLoop = false;

      // array idx correponding to read pointer
      int readArrayIdx = nOffset / (int)m_ChunkSize;

      if (readArrayIdx >= MAX_MEMORY_UNITS)
      {
        // should never get here
        QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
          "HttpSegmentDataStoreHeap::Read() Failed readArrayIdx %d, maxUnits %d",
          readArrayIdx, MAX_MEMORY_UNITS);

        numRead = -1;
        break;
      }

      int readOffset = nOffset % (int)m_ChunkSize;
      byte *pReadPtr = m_HeapMemArray[readArrayIdx] + readOffset;

      int64 writeArrayIdx = m_NumBytesDownloaded / m_ChunkSize;

      int64 maxToRead = 0;

      if (readArrayIdx < writeArrayIdx)
      {
        maxToRead = m_ChunkSize - readOffset;
      }
      else
      {
        if (readArrayIdx == writeArrayIdx)
        {
          maxToRead = m_NumBytesDownloaded - nOffset;
        }
        else
        {
          QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
            "HttpSegmentDataStoreHeap::Read Unexpcted error readArrayIdx %d != "
            "writeArrayIdx %lld", readArrayIdx, writeArrayIdx);
        }
      }

      int64 numToRead = (nBufSize <= maxToRead ? nBufSize : maxToRead);

      if (numToRead > 0)
      {
        memcpy(pBuf, pReadPtr, (size_t)numToRead);

        QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
          "HttpSegmentDataStoreHeap::Read() numRead %lld, maxToRead %lld, "
          "readArrayIdx %d, writeArrayIdx %lld",
          numRead, maxToRead, readArrayIdx, writeArrayIdx);

        pBuf += numToRead;
        numRead += numToRead;
        nOffset += (int)numToRead;

        if ((numRead < nBufSize) &&
            (nOffset < m_NumBytesDownloaded))
        {
          // read from next memory unit.
          QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
            "HttpSegmentDataStoreHeap::Read() Read from next memory unit as "
            "numRead %lld < numDownloaded %lld, and can read %d bytes",
            numRead, m_NumBytesDownloaded, nBufSize);

          doLoop = true;
          nBufSize -= (int)numToRead;
        }
      }
      else
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                     "Read() Zero bytes to read");
      }
    }
    while (doLoop);
  }

  return numRead;
}
/**
 * @brief: Discards the data from nOffset to m_NumBytesDownloaded
   @param - nOffset - the offset from which data should be discarded
 */
void HttpSegmentDataStoreHeap::DiscardData(int64 nOffset)
{
  int startArrIdx = (int)(nOffset/m_ChunkSize);
  int endArrIdx = (int)(m_NumBytesDownloaded/m_ChunkSize);
  if((nOffset % m_ChunkSize) != 0)
  {
    startArrIdx++;
  }
  if(m_NumBytesDownloaded % m_ChunkSize == 0)
  {
    endArrIdx--;
  }
  for(int i= startArrIdx;i<=endArrIdx;i++)
  {
    if (m_HeapMemArray[i])
    {
      QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "DiscardData() Deallocate %p", (void*)m_HeapMemArray[i]);

      m_pHeapManager->Deallocate(m_HeapMemArray[i]);
      m_HeapMemArray[i] = NULL;
    }
  }
  if(m_HeapMemArray[(nOffset/m_ChunkSize)])
  {
    int writeOffset = (int)(nOffset % m_ChunkSize);
    std_memset(m_HeapMemArray[(nOffset/m_ChunkSize)]+writeOffset,0,(int)m_ChunkSize - writeOffset);
  }
}
/**
 * @brief: Mark the segment as fully downloaded. If number of bytes downloaded are greater than param
   nEndOffset then discards the extra data and updates the number of bytes downloaded.
   @param - nEndOffset - the last valid offset for the segment
 */
void HttpSegmentDataStoreHeap::SetSegmentComplete(int64 nEndOffset)
{
  MM_CriticalSection_Enter(m_pDataLock);
  QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "HttpSegmentDataStoreHeap::SetSegmentComplete start %d end %d numBytesDownloaded %lld",
                (int)m_nStartOffset, (int)nEndOffset, m_NumBytesDownloaded);
  m_bIsFullyDownloaded = true;
  if(nEndOffset >= m_nStartOffset)
  {
    nEndOffset-= m_nStartOffset;
    if((m_NumBytesDownloaded) > nEndOffset)
    {
      //Data is stored with reference to 0 offset i.e from 0 to m_NumBytesDownloaded
      //So we need to pass endoffset after subtracting startoffset.
      //Discard data from nEndOffset to m_NumBytesDownloaded
      DiscardData(nEndOffset);
      m_NumBytesDownloaded = nEndOffset;
    }
  }
  MM_CriticalSection_Leave(m_pDataLock);
}
void HttpSegmentDataStoreHeap::SetPurgeFlag(bool bPurge)
{
  MM_CriticalSection_Enter(m_pDataLock);
  m_bIsPurgable = bPurge;
  MM_CriticalSection_Leave(m_pDataLock);
  return;
}
/**
  * Set the startoffset for the segment.
  */
void HttpSegmentDataStoreHeap::SetStartOffset(int64 nStartOffset)
{
  MM_CriticalSection_Enter(m_pDataLock);
  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "HttpSegmentDataStoreListElement::Setting start offset to %lld",nStartOffset);
  m_nStartOffset = nStartOffset;
  MM_CriticalSection_Leave(m_pDataLock);
  return;
}
/**
 * Return the storage type heap or file system associated with
 * the segment.
 */
iHTTPBase::DataStorageType HttpSegmentDataStoreHeap::GetStorageType() const
{
  return iHTTPBase::HEAP;
}

/**
 * For debugging
 */
void HttpSegmentDataStoreHeap::GetPrintStr(char* printStr, int bufSize)
{
  int numUnitsInUse = 0;
  for (int i = 0; i < MAX_MEMORY_UNITS; ++i)
  {
    if (NULL == m_HeapMemArray[i])
    {
      break;
    }

    ++numUnitsInUse;
  }

  std_strlprintf(printStr, bufSize, "{%d,%lld,?%d,storage %d,numUnits %d}-",
                 (int)m_Key, m_NumBytesDownloaded, m_bIsFullyDownloaded,
                 GetStorageType(), numUnitsInUse);
}

/**
 * c'tor
 */
HttpSegmentDataStoreListElement::HttpSegmentDataStoreListElement() :
  m_pHttpSegmentDataStoreBase(NULL)
{

}

/**
 * d'tor
 */
HttpSegmentDataStoreListElement::~HttpSegmentDataStoreListElement()
{
}

/**
 * Reset segment info.
 */
void HttpSegmentDataStoreListElement::Reset()
{
  m_pHttpSegmentDataStoreBase->Reset();
}
void HttpSegmentDataStoreBase::SetKey(int64 key)
{
   MM_CriticalSection_Enter(m_pDataLock);
   m_Key = key;
   MM_CriticalSection_Leave(m_pDataLock);
}
/**
 * Set the key for the segment.
 */
void HttpSegmentDataStoreListElement::SetKey(uint64 key)
{
  if ((int64)0xffffffffffffffffLL != m_pHttpSegmentDataStoreBase->GetKey())
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "HttpSegmentDataStoreListElement::SetKey Key was already initialized");
  }

  m_pHttpSegmentDataStoreBase->SetKey(key);
}

/**
 * Get the segment key.
 */
int64 HttpSegmentDataStoreListElement::GetKey() const
{
  int64 nKey;

  nKey = (m_pHttpSegmentDataStoreBase
          ? m_pHttpSegmentDataStoreBase->GetKey() : 0xffffffffffffffffLL);
  return nKey;

}
/**
  * Set the startoffset for the segment.
  */
void HttpSegmentDataStoreListElement::SetStartOffset(int64 nStartOffset)
{
  if(m_pHttpSegmentDataStoreBase)
  {
    m_pHttpSegmentDataStoreBase->SetStartOffset(nStartOffset);
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "HttpSegmentDataStoreListElement::Unexpected Null data store base");
  }
}

/**
  * Get the startoffset for the segment.
  */
int64 HttpSegmentDataStoreListElement::GetStartOffset() const
{
  int64 nStartOffset;
  nStartOffset = (m_pHttpSegmentDataStoreBase
          ? m_pHttpSegmentDataStoreBase->GetStartOffset() : -1);

  return nStartOffset;
}

/**
  * Set the purge flag for the segment.
  */
void HttpSegmentDataStoreListElement::SetPurgeFlag(bool bPurge)
{
  if(m_pHttpSegmentDataStoreBase)
  {
    m_pHttpSegmentDataStoreBase->SetPurgeFlag(bPurge);
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                  "HttpSegmentDataStoreListElement::Setting purge flag to %d",bPurge);
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                 "HttpSegmentDataStoreListElement::Unexpected Null data store base");
  }
  return;

}

/**
  * Return true if purgable, false otherwise
  */
bool HttpSegmentDataStoreListElement::IsPurgable()
{
  bool bPurge;

  bPurge= (m_pHttpSegmentDataStoreBase
          ? m_pHttpSegmentDataStoreBase->IsPurgable() : true);
  return bPurge;
}
/**
 * Get ptr to buffer to write to for the segment.
 */
bool HttpSegmentDataStoreListElement::GetBuffer(byte* &pBuf, int64& nBufSize)
{
  return (m_pHttpSegmentDataStoreBase
          ? m_pHttpSegmentDataStoreBase->GetBuffer(pBuf, nBufSize)
          : false);
}

/**
 * Get ptr to buffer to write to for the segment.
 */
bool HttpSegmentDataStoreListElement::CommitBuffer(byte* pBuf, int64 nNumWritten)
{
  return (m_pHttpSegmentDataStoreBase
          ? m_pHttpSegmentDataStoreBase->CommitBuffer(pBuf, nNumWritten)
          : false);
}

/**
 * Read at most nBufSize bytes at an offset nOffset from the
 * start of the segment.
 */
int64 HttpSegmentDataStoreListElement::Read(int nOffset, byte* pBuf, int nBufSize)
{
  QTV_NULL_PTR_CHECK(pBuf, 0);
  QTV_NULL_PTR_CHECK(m_pHttpSegmentDataStoreBase, 0);
  return m_pHttpSegmentDataStoreBase->Read(nOffset, pBuf, nBufSize);
}

/**
 * Mark the segment as fully downloaded.
 */
void HttpSegmentDataStoreListElement::SetSegmentComplete(int64 nEndOffset)
{
  QTV_NULL_PTR_CHECK(m_pHttpSegmentDataStoreBase, RETURN_VOID);
  if(nEndOffset >= 0)
  {
    m_pHttpSegmentDataStoreBase->SetSegmentComplete(nEndOffset);
  }
  return;
}

/**
 * Get the number of bytes downloaded.
 */
int64 HttpSegmentDataStoreListElement::GetNumBytesDownloaded() const
{
  return (m_pHttpSegmentDataStoreBase
          ? m_pHttpSegmentDataStoreBase->m_NumBytesDownloaded
          : 0);
}

/**
 * Return true if segment is marked downloaded, else false.
 */
bool HttpSegmentDataStoreListElement::IsFullyDownloaded() const
{
  bool bFullyDownloaded;

  bFullyDownloaded = (m_pHttpSegmentDataStoreBase ? m_pHttpSegmentDataStoreBase->IsFullyDownloaded() : false);
  return bFullyDownloaded;

}

/**
 * Return the storage type heap or file system associated with
 * the segment.
 */
iHTTPBase::DataStorageType HttpSegmentDataStoreListElement::GetStorageType() const
{
  return m_pHttpSegmentDataStoreBase->GetStorageType();
}

/**
 * For debugging
 */
void HttpSegmentDataStoreListElement::GetPrintStr(char* printStr, int bufSize)
{
  if (m_pHttpSegmentDataStoreBase)
  {
    m_pHttpSegmentDataStoreBase->GetPrintStr(printStr, bufSize);
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
                 "HttpSegmentDataStoreListElement::Print() "
                 "m_pHttpSegmentDataStoreBase uninitialized");
  }
}

} // end namespace video
