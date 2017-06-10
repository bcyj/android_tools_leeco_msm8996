#ifndef _HTTP_SEGMENT_DATASTORE_STRUCTS_H_
#define _HTTP_SEGMENT_DATASTORE_STRUCTS_H_

/************************************************************************* */
/**
 * HTTPSegmentDataStoreStructs.h
 * @brief Header file for HttpSegmentDataStoreBase, HttpSegmentDataStoreHeap,
 *  HttpSegmentDataStoreFileSystem (TO DO), HttpSegmentDataStoreListElement.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPSegmentDataStoreStructs.h#14 $
$DateTime: 2013/02/11 22:51:37 $
$Change: 3346652 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPSegmentDataStoreStructs.h
** ======================================================================= */

#include "HTTPBase.h"
#include "HTTPCommon.h"
#include "HTTPDataStoreBase.h"
#include "StreamQueue.h"
#include "assert.h"
#include "HTTPHeapManager.h"

namespace video {

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

/**
 *  HttpSegmentDataStoreBase
 *    Pure virtual class for segment store. Defines interfaces
 *    for the actual derived type HttpSegmentDataStoreHeap, (and
 *    HttpSegmentDataStoreFileSystem TO DO).
 */
class HttpSegmentDataStoreBase
{
public:
  /**
   * c'tor
   */
  HttpSegmentDataStoreBase();

  /**
   * d'tor
   */
  virtual ~HttpSegmentDataStoreBase();

  /**
   * Reset segment info.
   */
  virtual void Reset() = 0;

  /**
   * Get ptr to buffer to write to for the segment.
   */
  virtual bool GetBuffer(byte* &pBuf, int64& nBufSize) = 0;

  /**
   * Commit the info associated with the data write on the buffer
   * obtained via GetBuffer for the segment.
   */
  virtual bool CommitBuffer(byte* pBuf, int64 nNumWritten) = 0;

  /**
   * Read at most nBufSize bytes at an offset nOffset from the
   * start of the segment.
   */
  virtual int64 Read(int nOffset, byte* pBuf, int nBufSize) = 0;

  /**
   * Mark the segment as fully downloaded.
   */
  virtual void SetSegmentComplete(int64 nEndOffset)=0;

  /**
   * Return the storage type heap or file system associated with
   * the segment.
   */
  virtual iHTTPBase::DataStorageType GetStorageType() const = 0;

  /**
   * Set the startoffset for the segment.
   */
  virtual void SetStartOffset(int64 nStartOffset) = 0;

  virtual void SetPurgeFlag(bool bPurge) = 0;
  virtual void SetKey(int64 nkey);
  /**
   * Check whether segment is fully downloaded.
   */
  virtual bool IsFullyDownloaded() const
  {
    bool bFullyDownloaded;
    MM_CriticalSection_Enter(m_pDataLock);
    bFullyDownloaded = m_bIsFullyDownloaded;
    MM_CriticalSection_Leave(m_pDataLock);
    return bFullyDownloaded;
  };

  virtual bool IsPurgable() const
  {
    bool bPurge;
    MM_CriticalSection_Enter(m_pDataLock);
    bPurge = m_bIsPurgable;
    MM_CriticalSection_Leave(m_pDataLock);
    return bPurge;
  }
  virtual int64 GetKey() const
  {
    int64 nKey;
    MM_CriticalSection_Enter(m_pDataLock);
    nKey = m_Key;
    MM_CriticalSection_Leave(m_pDataLock);
    return nKey;
  }
  virtual int64 GetStartOffset() const
  {
    int64 nStartOffset;
    MM_CriticalSection_Enter(m_pDataLock);
    nStartOffset = m_nStartOffset;
    MM_CriticalSection_Leave(m_pDataLock);
    return nStartOffset;
  }
  /**
   * Return the total number of bytes downloaded into this segment.
   */
  virtual int64 GetNumBytesDownloaded() const
  {
    int64 nNumBytesDownloaded;
    MM_CriticalSection_Enter(m_pDataLock);
    nNumBytesDownloaded = m_NumBytesDownloaded;
    MM_CriticalSection_Leave(m_pDataLock);
    return nNumBytesDownloaded;
  };

  /**
   * For debugging
   */
  virtual void GetPrintStr(char* printStr, int bufSize) = 0;

protected:
  /**
   * Reset base class member variables.
   */
  void ResetBase();

  // unique key for segment. key space is owned by client.
  int64 m_Key;

  // num bytes downloaded from start of segment. no 'holes' of data
  // supported currently.
  int64 m_NumBytesDownloaded;

  // true when segment is downloaded, false when download in process.
  bool m_bIsFullyDownloaded;

  //true when segment can be purged, false otherwise.
  bool m_bIsPurgable;

  //startoffset in bytes for the segment.
  int64 m_nStartOffset;
  MM_HANDLE m_pDataLock;
  friend class HttpSegmentDataStoreListElement;
  friend class HttpSegmentDataStoreContainer;
};

/**
 * HttpSegmentDataStoreHeap
 *  Derived class for heap implementation for segment.
 */
class HttpSegmentDataStoreHeap : public HttpSegmentDataStoreBase
{
public:
  /**
   * c'tor. Also calls base class's ResetBase.
   */
  HttpSegmentDataStoreHeap();

  /**
   * d'tor
   */
  virtual ~HttpSegmentDataStoreHeap();

  /**
   * Pass the ptr heap manager instance to the segment heap.
   */
  void SetHeapManager(HTTPHeapManager *pHeapManager);

  /**
   * Reset segment info.
   */
  virtual void Reset();

  /**
   * Get ptr to buffer to write to for the segment.
   */
  virtual bool GetBuffer(byte* &pBuf, int64& nBufSize);

  /**
   * Commit the info associated with the data write on the buffer
   * obtained via GetBuffer for the segment.
   */
  virtual bool CommitBuffer(byte* pBuf, int64 nNumWritten);

  /**
   * Read at most nBufSize bytes at an offset nOffset from the
   * start of the segment.
   */
  virtual int64 Read(int nOffset, byte* pBuf, int nBufSize);

  /**
   * Mark the segment as fully downloaded.
   */
  virtual void SetSegmentComplete(int64 nEndOffset);

  /**
   * Set the startoffset for the segment.
   */
  virtual void SetStartOffset(int64 nStartOffset);

  virtual void SetPurgeFlag(bool bPurge);

  void DiscardData(int64 nOffset);

  /**
   * Return the storage type heap or file system associated with
   * the segment.
   */
  virtual iHTTPBase::DataStorageType GetStorageType() const;

  /**
   * For debugging
   */
  virtual void GetPrintStr(char* printStr, int bufSize);

private:
  // Max SegmentSize =
  //   (MAX_MEMORY_UNITS * HTTPHeapManager::LOGICAL_UNIT_MEM_SZ)
  static const int MAX_MEMORY_UNITS = HTTPHeapManager::MAX_REGIONS * HTTPHeapManager::NUM_LOGICAL_UNITS_PER_ALLOC;

  // Pointer to the shared HTTPHeapManager instance.
  HTTPHeapManager *m_pHeapManager;

  // An array of logical memory units to store data.
  byte* m_HeapMemArray[MAX_MEMORY_UNITS];

  // Size of a logical unit managed by the HTTPHeapManager
  const int64 m_ChunkSize;
};

/**
 * HttpSegmentDataStoreListElement
 *  Linked list element for a segment data store instance.
 */
class HttpSegmentDataStoreListElement
{
public:
  StreamQ_link_type m_link;

  /**
   * c'tor
   */
  HttpSegmentDataStoreListElement();

  /**
   * d'tor
   */
  ~HttpSegmentDataStoreListElement();

  /**
   * Reset segment info.
   */
  void Reset();

  /**
   * Set the key for the segment.
   */
  void SetKey(uint64 key);

  /**
   * Get the segment key.
   */
  int64 GetKey() const;

  /**
   * Set the startoffset for the segment.
   */
  void SetStartOffset(int64 key);

  /**
   * Get the startoffset for the segment.
   */
  int64 GetStartOffset() const;

   /**
   * Set the purge flag for the segment.
   */
  void SetPurgeFlag(bool bPurge);

  /**
   * Return true if purgable, false otherwise
   */
  bool IsPurgable();

  /**
   * Get ptr to buffer to write to for the segment.
   */
  bool GetBuffer(byte* &pBuf, int64& nBufSize);

  /**
   * Get ptr to buffer to write to for the segment.
   */
  bool CommitBuffer(byte* pBuf, int64 nNumWritten);

  /**
   * Read at most nBufSize bytes at an offset nOffset from the
   * start of the segment.
   */
  int64 Read(int nOffset, byte* pBuf, int nBufSize);

  /**
   * Mark the segment as fully downloaded.
   */
  void SetSegmentComplete(int64 nEndOffset = -1);

  /**
   * Get the number of bytes downloaded.
   */
  int64 GetNumBytesDownloaded() const;

  /**
   * Return true if segment is marked downloaded, else false.
   */
  bool IsFullyDownloaded() const;

  /**
   * Return the storage type heap or file system associated with
   * the segment.
   */
  iHTTPBase::DataStorageType GetStorageType() const;

  /**
   * For debugging
   */
  void GetPrintStr(char* printStr, int bufSize);

private:
  HttpSegmentDataStoreBase *m_pHttpSegmentDataStoreBase;
  bool bIsMemAllocatedLocally;
  friend class HttpSegmentDataStoreContainer;
};

} // end namespace video

#endif
