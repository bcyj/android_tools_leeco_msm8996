/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#include "QICommon.h"
#include "QIONBuffer.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/android_pmem.h>
#include <string.h>
#include <errno.h>

#include <linux/msm_ion.h>

#define PAGE_SZ 4096

/*===========================================================================
 * Function: OpenClient
 *
 * Description: open the ion client
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QIONBuffer::OpenClient()
{
  uint32_t flags = (mCached) ? O_RDONLY : (O_RDONLY | O_SYNC);
  mIONFd = open("/dev/ion", flags);
  if(mIONFd < 0) {
    QIDBG_ERROR("%s:%d] Ion open failed %s\n",
      __func__, __LINE__, strerror(errno));
    return QI_ERR_GENERAL;
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: DoMmap
 *
 * Description: MMap the buffers
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QIONBuffer::DoMmap()
{
  int lrc = 0;
  struct ion_handle_data lHandleData;

  mAllocIon.flags = (mCached) ? ION_FLAG_CACHED : 0;
  mAllocIon.heap_id_mask = (0x1 << ION_IOMMU_HEAP_ID);
  mAllocIon.align = PAGE_SZ;

  /* to make it page size aligned */
  mAllocIon.len = (mLength + (PAGE_SZ-1)) & (~(PAGE_SZ-1));
  QIDBG_LOW("%s:%d] ionfd %d len %d", __func__, __LINE__,
    mIONFd, mAllocIon.len);
  lrc = ioctl(mIONFd, ION_IOC_ALLOC, &mAllocIon);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Ion alloc failed %s\n",
      __func__, __LINE__, strerror(errno));
    goto ION_ALLOC_FAILED;
  }

  mFdIonMap.handle = mAllocIon.handle;
  lrc = ioctl(mIONFd, ION_IOC_SHARE, &mFdIonMap);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Ion map failed %s\n",
      __func__, __LINE__, strerror(errno));
    goto ION_MAP_FAILED;
  }
  mFd = mFdIonMap.fd;
  mAddr = (uint8_t *)mmap(NULL,
    mAllocIon.len,
    PROT_READ  | PROT_WRITE,
    MAP_SHARED,
    mFd,
    0);

  if (mAddr == MAP_FAILED) {
    QIDBG_ERROR("%s:%d] Ion map failed %s\n",
      __func__, __LINE__, strerror(errno));
    goto ION_MAP_FAILED;
  }
  QIDBG_MED("%s:%d] addr %p len %d alloc_len %d fd %d",
    __func__, __LINE__,
    mAddr, mLength, mAllocIon.len, mFd);
  return QI_SUCCESS;

ION_MAP_FAILED:
  lHandleData.handle = mFdIonMap.handle;
  lrc = ioctl(mIONFd, ION_IOC_FREE, &lHandleData);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Ion free failed %s\n",
      __func__, __LINE__, strerror(errno));
  }

ION_ALLOC_FAILED:
  return QI_ERR_GENERAL;
}

/*===========================================================================
 * Function: CloseClient
 *
 * Description: close the ION client
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIONBuffer::CloseClient()
{
  if (mIONFd > 0) {
    close(mIONFd);
  }
}

/*===========================================================================
 * Function: DoUnmap
 *
 * Description: unmap the buffers
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIONBuffer::DoUnmap()
{
  int lrc = 0;
  struct ion_handle_data lHandleData;

  if (mIONFd <= 0) {
    return;
  }
  QIDBG_MED("%s:%d] addr %p len %d alloc_len %d fd %d",
    __func__, __LINE__,
    mAddr, mLength, mAllocIon.len, mFd);

  lrc = munmap(mAddr, mAllocIon.len);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] munmap failed %s\n",
      __func__, __LINE__, strerror(errno));
  }
  close(mFdIonMap.fd);

  lHandleData.handle = mFdIonMap.handle;
  ioctl(mIONFd, ION_IOC_FREE, &lHandleData);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Ion free failed %s\n",
      __func__, __LINE__, strerror(errno));
  }
}

/*===========================================================================
 * Function: QIONBuffer
 *
 * Description: QIONBuffer constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIONBuffer::QIONBuffer()
: QIBuffer(NULL, 0)
{
  mIONFd = -1;
  memset(&mAllocIon, 0x0, sizeof(struct ion_allocation_data));
  memset(&mFdIonMap, 0x0, sizeof(struct ion_fd_data));
}

/*===========================================================================
 * Function: ~QIONBuffer
 *
 * Description: QIONBuffer destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIONBuffer::~QIONBuffer()
{
  /*unmap the buffer*/
  DoUnmap();

  /*close the ion client*/
  CloseClient();

}

/*===========================================================================
 * Function: DoCacheOps
 *
 * Description: Static Function to perform cache operation for the ION
 *             buffer
 *
 * Input parameters:
 *   aIONBuffer - pointer to the ION buffer
 *   aType - cache operation type
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QIONBuffer::DoCacheOps(QIONBuffer *aIONBuffer, QIONCacheOpsType aType)
{
  int lrc = QI_SUCCESS;
  uint32_t lCmd = ION_IOC_CLEAN_INV_CACHES;
  struct ion_custom_data mCustomData;
  struct ion_flush_data mFlushData;
  memset(&mFlushData, 0, sizeof(struct ion_flush_data));
  memset(&mCustomData, 0, sizeof(struct ion_custom_data));

  switch (aType) {
  case CACHE_INVALIDATE:
    lCmd = ION_IOC_CLEAN_CACHES;
    break;
  case CACHE_CLEAN:
    lCmd = ION_IOC_INV_CACHES;
    break;
  default:
  case CACHE_CLEAN_INVALIDATE:
    lCmd = ION_IOC_CLEAN_INV_CACHES;
    break;
  }
  if (aIONBuffer->mIONFd <= 0) {
    QIDBG_ERROR("%s:%d] Invalid ION fd %d", __func__, __LINE__,
      aIONBuffer->mIONFd);
    return lrc;
  }

  mFlushData.vaddr = aIONBuffer->Addr();
  mFlushData.fd = aIONBuffer->Fd();
  mFlushData.handle =  aIONBuffer->mFdIonMap.handle;
  mFlushData.length = aIONBuffer->Length();

  mCustomData.cmd = lCmd;
  mCustomData.arg = (unsigned long)&mFlushData;

  lrc = ioctl(aIONBuffer->mIONFd, ION_IOC_CUSTOM, &mCustomData);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] failed with errno %s\n", __func__, __LINE__,
    strerror(errno));
    lrc = QI_ERR_GENERAL;
  } else
    QIDBG_MED("%s:%d] success %d", __func__, __LINE__, aIONBuffer->mCached);

  return lrc;
}

/*===========================================================================
 * Function: New
 *
 * Description: 2 phase constructor for heap buffer
 *
 * Input parameters:
 *   aLength - length of the buffer
 *
 * Return values:
 *   pointer to QIONBuffer object
 *
 * Notes: none
 *==========================================================================*/
QIONBuffer *QIONBuffer::New(uint32_t aLength, bool aCached)
{
  int lrc = QI_SUCCESS;
  QIONBuffer *lBuffer = new QIONBuffer();
  if (NULL == lBuffer) {
    return NULL;
  }

  lBuffer->mLength = aLength;
  lBuffer->mCached = aCached;
  lrc = lBuffer->OpenClient();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] open client failed\n", __func__, __LINE__);
    goto error;
  }

  lrc = lBuffer->DoMmap();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] mmap failed\n", __func__, __LINE__);
    goto error;
  }

  return lBuffer;

error:
  if (lBuffer) {
    delete lBuffer;
    lBuffer = NULL;
  }
  return NULL;
}
