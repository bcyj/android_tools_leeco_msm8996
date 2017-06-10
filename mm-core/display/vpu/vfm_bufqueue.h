/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
    This file declares buf queue class
*****************************************************************************/

#ifndef VFM_BUFQUEUE_H
#define VFM_BUFQUEUE_H

#include <sys/types.h>
#include <linux/videodev2.h>
#include "alloc_controller.h"

#define MAX_BUFQ_BUFFERS (64)

namespace vpu {

typedef struct v4l2_buffer* v4l2bufptr;

class BufQ {
public:
    BufQ(const int32_t numBufs, const int32_t enableDebugLogs);

    virtual ~BufQ();

    v4l2bufptr getNextEmptyBuf();

    v4l2bufptr getEmptyBuf(uint32_t bufIdx);

    v4l2bufptr getEmptyBuf(unsigned long userptr);

    v4l2bufptr getNextUnassignedEmptyBuf();

    v4l2bufptr getBufPtr(const int32_t bufIdx);

    v4l2bufptr getNextAssignedEmptyBuf();

    private_handle_t* getPvtHandle(const int32_t bufIdx);

    void storeEmptyBuf(v4l2bufptr);

    void storeEmptyBuf(const uint32_t bufIdx);

    void assignUsrPtr(v4l2bufptr, unsigned long userptr);

    void assignPvtHandle(private_handle_t* hnd, const int32_t bufIdx);

    void markWithConsumer(const int32_t bufIdx);

    //setDim (int32_t width, int32_t height) {mWd = width; mHt = height; }
    //setPixFmt (int32_t pixFmt) { mPixFmt = pixFmt; }

private:
    /* Buffer states */
    enum {
        EMPTY,          //Buffer is EMPTY and it is with BufQ
        WITH_PRODUCER,  //Buffer is with producer - typically player
        FILLED,         //Buffer is filled and is with BufQ
        WITH_CONSUMER,  //Buffer is with consumer - typically driver
    };

    struct bufObj_t {
        v4l2bufptr          bufPtr;
        private_handle_t*   hnd;
        int32_t             bufState;
    };

    bufObj_t bufPool[MAX_BUFQ_BUFFERS];

    int32_t mNumBufs;
    Mutex   mLock;
    int32_t logAll() { return (mDebugLogs >= 2); }
    int32_t isDebug() { return (mDebugLogs >= 1); }
    int32_t mDebugLogs;
};

/*****************************************************************************
class OutBufPool:
 Maintains a pool of buffer objects and provides methods to allocate,
 deallocate ion buffers.
*****************************************************************************/
class OutBufPool {
public:
    OutBufPool(const int32_t enableDebugLogs);
    virtual ~OutBufPool();

    status_t allocBufs(const int32_t numBufs, const OutputCfg&);
    v4l2bufptr getEmptyBuf(const uint32_t idx);
    void* getBufDataPtr(const uint32_t bufIdx);
    int32_t getBufferMemorySize() { return mBufferMemorySize; }
private:
    struct bufObj_t {
        v4l2bufptr          bufPtr;
        gralloc::alloc_data   data;
    };
    status_t allocateIonBuffers();
    status_t deallocateIonBuffers();
    void assignUsrPtr(v4l2bufptr bufPtr, int32_t fd);

    bufObj_t bufPool[MAX_BUFQ_BUFFERS];

    uint32_t mNumBufs;
    Dim     mStride;
    int32_t mBufferMemorySize;
    gralloc::IAllocController*  mAllocController;
    gralloc::IMemAlloc*         mMemAlloc;
    int32_t logAll() { return (mDebugLogs >= 2);}
    int32_t isDebug() {return (mDebugLogs >= 1);}
    int32_t mDebugLogs;
};
}; //namespace VPU

#endif /* end of include guard: VFM_BUFQUEUE_H */
