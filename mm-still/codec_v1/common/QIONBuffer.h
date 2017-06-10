/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QIONBUFFER_H__
#define __QIONBUFFER_H__

#include "QIBuffer.h"
#include <linux/msm_ion.h>

/*===========================================================================
 * Class: QIONBuffer
 *
 * Description: This class represents the buffer allocated in ION memory
 *
 *
 * Notes: none
 *==========================================================================*/
class QIONBuffer : public QIBuffer {

public:

  /** QIONCacheOpsType:
   *  CACHE_INVALIDATE: invalidate cache
   *  CACHE_CLEAN: clean cache
   *  CACHE_CLEAN_INVALIDATE: Both clean and invalidate cache
   *
   *  Type of cache operation
   *
   **/
  typedef enum _QIONCacheOpsType {
    CACHE_INVALIDATE,
    CACHE_CLEAN,
    CACHE_CLEAN_INVALIDATE,
  } QIONCacheOpsType;

  /** New:
   *
   *  2 phase constructor for ion buffer
   **/
  static QIONBuffer *New(uint32_t aLength, bool aCached = true);

  /** ~QIONBuffer:
   *
   *  virtual destructor
   **/
  virtual ~QIONBuffer();

  /** DoCacheOps:
   *
   *  cache operation for the ION buffers
   **/
  static int DoCacheOps(QIONBuffer *aIONBuffer, QIONCacheOpsType aType);

private:

  /** QIONBuffer:
   *
   *  private constructor
   **/
  QIONBuffer();

  /** OpenClient:
   *
   *  opens ion client
   **/
  int OpenClient();

  /** DoMmap:
   *
   *  mmap the buffers
   **/
  int DoMmap();

  /** CloseClient:
   *
   *  close the ion client
   **/
  void CloseClient();

  /** DoUnmap:
   *
   *  unmap the buffers
   **/
  void DoUnmap();

private:

  /** mAllocIon:
   *
   *  ion allocation data
   **/
  struct ion_allocation_data mAllocIon;

  /** mFdIonMap:
   *
   *  ion fd map data
   **/
  struct ion_fd_data mFdIonMap;

  /** mIONFd:
   *
   *  ion fd
   **/
  int mIONFd;

  /** mCached:
   *
   *  Flag to indicate if the buffer is cached
   **/
  bool mCached;
};

#endif //__QIONBUFFER_H__
