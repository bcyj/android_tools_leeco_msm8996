/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
    This file defines buf queue class. BufQ holds 'slots' for buffer pointers.
    BufQ book keeps a mapping of 'fd' buffer from client to v4l2 buffer object
    of the driver.
*****************************************************************************/

#include "vfm_defs.h"
#include "vfm_utils.h"
#include "vfm_vpuinterface.h"
#include "vfm_bufqueue.h"
#include <linux/msm_ion.h>
namespace vpu {

BufQ::BufQ(const int32_t numBufs, const int32_t enableDebugLogs)
    : mNumBufs(numBufs),
      mDebugLogs(enableDebugLogs)
{
    ALOGD_IF(enableDebugLogs, "%s: E", __func__);
    if(numBufs > MAX_BUFQ_BUFFERS){
        ALOGE("%s: numBufs requested: %d is large. Limiting to: %d",
            __func__, numBufs, MAX_BUFQ_BUFFERS);
        mNumBufs = MAX_BUFQ_BUFFERS;
    }
    /* All slots in the array are initialized */
    for(int32_t i = 0; i < MAX_BUFQ_BUFFERS; i++){
        v4l2bufptr buffer   = (v4l2bufptr) malloc(sizeof(struct v4l2_buffer));
        memset(buffer, 0, sizeof(struct v4l2_buffer));
        buffer->m.planes    = (struct v4l2_plane*)
                    malloc(sizeof(struct v4l2_plane[VIDEO_MAX_PLANES]));
        memset(buffer->m.planes, 0,
            sizeof(struct v4l2_plane[VIDEO_MAX_PLANES]));
        buffer->index       = i;

        bufPool[i].bufPtr   = buffer;
        bufPool[i].bufState = EMPTY;
    }
}

BufQ::~BufQ()
{
    ALOGD_IF(isDebug(), "%s: E", __func__);
    for(int32_t i = 0; i < MAX_BUFQ_BUFFERS; i++){
        if(bufPool[i].bufPtr){
            free(bufPool[i].bufPtr->m.planes);
            free(bufPool[i].bufPtr);
            bufPool[i].bufPtr = NULL;
        }
    }
}

//Returns the next empty buffer irrespective of whether it is assigned to a
//specific userptr/fd or not
v4l2bufptr BufQ::getNextEmptyBuf()
{
    v4l2bufptr bufPtr = NULL;
    Mutex::Autolock autoLock(mLock);

    for(int32_t i = 0; i < mNumBufs; i++){
        if(EMPTY == bufPool[i].bufState){
            bufPool[i].bufState = WITH_PRODUCER;
            return bufPool[i].bufPtr;
        }
    }
    return bufPtr;
}

//Returns the empty buffer that matches the buffer index
//Returns NULL if such ID does not exist or if it is not empty
v4l2bufptr BufQ::getEmptyBuf(uint32_t bufIdx)
{
    v4l2bufptr bufPtr = NULL;
    Mutex::Autolock autoLock(mLock);

    for(int32_t i = 0; i < mNumBufs; i++){
        if(bufPool[i].bufPtr->index == bufIdx){
            if(EMPTY != bufPool[i].bufState){
                ALOGE("%s: Buffer %d is not empty, but still requested!",
                      __func__, bufIdx);
                return NULL;
            }else {
                bufPool[i].bufState = WITH_PRODUCER;
                return bufPool[i].bufPtr;
            }
        }
    }
    ALOGD_IF(isDebug(), "%s: Buffer id %d is not in the list",
        __func__, bufIdx);
    return bufPtr;
}

//Returns the empty buffer that has the matching userptr
//Returns NULL if such buffer is not empty or if the userptr does not exit
v4l2bufptr BufQ::getEmptyBuf(unsigned long userptr)
{
    v4l2bufptr bufPtr = NULL;
    Mutex::Autolock autoLock(mLock);

    for(int32_t i = 0; i < mNumBufs; i++){
        if(bufPool[i].bufPtr->m.planes[0].m.userptr == userptr){
            if(EMPTY != bufPool[i].bufState){
                ALOGE("%s: Buffer %d is not empty, but still requested!",
                       __func__, bufPool[i].bufPtr->index);
                return NULL;
            }else {
                bufPool[i].bufState = WITH_PRODUCER;
                return bufPool[i].bufPtr;
            }
        }
    }
    ALOGD_IF(isDebug(), "%s: New buffer fd %d assigning now",
        __func__, (int)userptr);
    return bufPtr;
}

//Returns the empty buf which is not yet assigned to any userptr
v4l2bufptr BufQ::getNextUnassignedEmptyBuf()
{
    v4l2bufptr bufPtr = NULL;
    Mutex::Autolock autoLock(mLock);

    for(int32_t i = 0; i < mNumBufs; i++){
        if((0 == bufPool[i].bufPtr->m.planes[0].m.userptr) &&
                    (EMPTY == bufPool[i].bufState)){
            bufPool[i].bufState = WITH_PRODUCER;
            return bufPool[i].bufPtr;
        }
    }
    ALOGD_IF(isDebug(), "%s: No unassigned empty buffer ", __func__);
    return bufPtr;
}

//Returns the empty buf from among the 'fd-assigned' buffer slots
v4l2bufptr BufQ::getNextAssignedEmptyBuf()
{
    v4l2bufptr bufPtr = NULL;
    Mutex::Autolock autoLock(mLock);

    for(int32_t i = 0; i < mNumBufs; i++){
        if((0 != bufPool[i].bufPtr->m.planes[0].m.userptr) &&
                    (EMPTY == bufPool[i].bufState)){
            bufPool[i].bufState = WITH_PRODUCER;
            return bufPool[i].bufPtr;
        }
    }
    ALOGD_IF(isDebug(), "%s: No assigned empty buffer ", __func__);
    /* Should not reach this point unless producer is extremely fast as compared
     * to the consumer */
    for(int32_t i = 0; i < mNumBufs; i++){
        ALOGD_IF(isDebug(), "%s: bufSlot: %d, fd:%d, state: %d",
            __func__, i, (int)bufPool[i].bufPtr->m.planes[0].m.userptr,
            bufPool[i].bufState);
    }
    return bufPtr;
}

//Store empty buffer in the pool
void BufQ::storeEmptyBuf(v4l2bufptr bufToStore)
{
    Mutex::Autolock autoLock(mLock);
    for(int32_t i = 0; i < mNumBufs; i++){
        if(bufPool[i].bufPtr == bufToStore){
            bufPool[i].bufState = EMPTY;
            ALOGD_IF(logAll(), "%s: slot: %d marked empty",
                        __func__, i);
            return;
        }
    }
    ALOGD_IF(isDebug(), "%s: This buffer: %p is not in the list ",
        __func__, bufToStore);
    return;
}

//Store empty buffer in the pool
void BufQ::storeEmptyBuf(const uint32_t bufIdx)
{
    Mutex::Autolock autoLock(mLock);
    for(int32_t i = 0; i < mNumBufs; i++){
        if(bufPool[i].bufPtr->index == bufIdx){
            if(EMPTY == bufPool[i].bufState){
                ALOGE("%s: Buffer %d is already empty. Freeing again",
                      __func__, bufIdx);
            }
            bufPool[i].bufState = EMPTY;
            ALOGD_IF(logAll(), "%s: slot: %d marked empty",
                        __func__, i);
            return;
        }
    }
    ALOGD_IF(isDebug(), "%s: Buffer id %d is not in the list",
        __func__, bufIdx);
    return;
}

/*
    Returns the v4l2 buffer pointer corresponing to the requested index
    Does not change the state of the buffer
*/
v4l2bufptr BufQ::getBufPtr(const int32_t bufIdx)
{
    v4l2bufptr bufPtr = NULL;
    Mutex::Autolock autoLock(mLock);
    ALOGD_IF(logAll(), "%s: bufIdx: %d", __func__, bufIdx);

    for(int32_t i = 0; i < mNumBufs; i++){
        if(bufPool[i].bufPtr->index == (uint32_t)bufIdx){
            return bufPool[i].bufPtr;
        }
    }
    ALOGE("%s: bufIdx: %d not found", __func__, bufIdx);
    return bufPtr;
}

/*
    Returns the private handle corresponing to the requested index
    Does not change the state of the buffer
*/
private_handle_t* BufQ::getPvtHandle(const int32_t bufIdx)
{
    private_handle_t* handle = NULL;
    Mutex::Autolock autoLock(mLock);
    ALOGD_IF(logAll(), "%s: bufIdx: %d", __func__, bufIdx);

    for(int32_t i = 0; i < mNumBufs; i++){
        if(bufPool[i].bufPtr->index == (uint32_t) bufIdx){
            return bufPool[i].hnd;
        }
    }
    ALOGE("%s: bufIdx: %d not found", __func__, bufIdx);
    return handle;
}

void BufQ::assignUsrPtr(v4l2bufptr bufPtr, unsigned long userptr)
{
    Mutex::Autolock autoLock(mLock);
    if(bufPtr){
        for(int32_t j = 0; j < VIDEO_MAX_PLANES; j++){
            bufPtr->m.planes[j].m.userptr = userptr;
        }
    }
    return;
}

void BufQ::assignPvtHandle(private_handle_t* hnd, const int32_t bufIdx)
{
    Mutex::Autolock autoLock(mLock);
    ALOGD_IF(logAll(), "%s: bufIdx: %d", __func__, bufIdx);

    if(hnd && (bufIdx >= 0 && bufIdx < mNumBufs)){
        bufPool[bufIdx].hnd = hnd;
    }else{
        ALOGE("%s: Error. hnd:%p bufIdx: %d", __func__, hnd, bufIdx);
    }
    return;
}

void BufQ::markWithConsumer(const int32_t bufIdx)
{
    Mutex::Autolock autoLock(mLock);
    ALOGD_IF(logAll(), "%s: bufIdx: %d", __func__, bufIdx);
    for(int32_t i = 0; i < mNumBufs; i++){
        if(bufPool[i].bufPtr->index == (uint32_t)bufIdx){
            if(WITH_CONSUMER == bufPool[i].bufState){
                ALOGE("%s: Warning: Buffer: %d is already with consumer",
                        __func__, bufIdx);
            }
            bufPool[i].bufState = WITH_CONSUMER;
            return;
        }
    }
}
/*****************************************************************************
class OutBufPool
*****************************************************************************/
#define ION_NODE "/dev/ion"

OutBufPool::OutBufPool(const int32_t enableDebugLogs)
    : mAllocController(NULL),
      mDebugLogs(enableDebugLogs)
{
    ALOGD_IF(isDebug(), "%s: E", __func__);
    mAllocController = gralloc::IAllocController::getInstance();
    if(NULL == mAllocController){
        ALOGE("%s: gralloc::IAllocController::getInstance failed", __func__);
    }
    if (mAllocController){
        mMemAlloc = mAllocController->getAllocator(
                                    private_handle_t::PRIV_FLAGS_USES_ION);
        if(NULL == mMemAlloc){
            ALOGE("%s: mAllocController->getAllocator failed", __func__);
        }
    }
}

OutBufPool::~OutBufPool(){
    ALOGD_IF(isDebug(), "%s: E", __func__);

    deallocateIonBuffers();
    for(uint32_t i = 0; i < mNumBufs; i++){
        if(bufPool[i].bufPtr){
            free(bufPool[i].bufPtr->m.planes);
            free(bufPool[i].bufPtr);
            bufPool[i].bufPtr = NULL;
        }
    }
}

/* allocates numBufs of ion buffers and v4l2_buffer structures */
status_t OutBufPool::allocBufs(int32_t numBufs, const OutputCfg& outCfg){
    ALOGD_IF(isDebug(), "%s: E numBufs: %d outCfg: %s", __func__, numBufs,
                        outCfg.dump().string());

    status_t err = NO_ERROR;

    mNumBufs = numBufs;
    /* Allocate v4l2_buffer structures */
    for(int32_t i = 0; i < numBufs; i++){
        v4l2bufptr buffer   = (v4l2bufptr)malloc(sizeof(struct v4l2_buffer));
        memset(buffer, 0, sizeof(struct v4l2_buffer));
        buffer->m.planes    =(struct v4l2_plane*)
                        malloc(sizeof(struct v4l2_plane[VIDEO_MAX_PLANES]));
        memset(buffer->m.planes, 0,
            sizeof(struct v4l2_plane[VIDEO_MAX_PLANES]));
        buffer->index       = i;

        bufPool[i].bufPtr   = buffer;
    }

    VpuIntf::getBufferSize(outCfg.sDim, outCfg.pixFmt, mBufferMemorySize,
                                mStride);
    mBufferMemorySize = align(mBufferMemorySize, 4096);
    ALOGD_IF(isDebug(), "%s: 4096 aligned memsize: %d", __func__,
                            mBufferMemorySize);

    err = allocateIonBuffers();

    return err;
}

/* Private methods of OutBufPool */
/* allocates ion buffers */
status_t OutBufPool::allocateIonBuffers()
{
    status_t err = NO_ERROR;
    int32_t result;

    ALOGD_IF(isDebug(), "%s: E", __func__);
    for(uint32_t i = 0; i < mNumBufs; i++){
        bufObj_t* bufObj = &bufPool[i];

        bufObj->data.base = 0;
        bufObj->data.fd = -1;
        bufObj->data.offset = 0;
        bufObj->data.size = mBufferMemorySize;
        bufObj->data.align = getpagesize();
        bufObj->data.uncached = true;
        int allocFlags = GRALLOC_USAGE_PRIVATE_IOMMU_HEAP;
        int ret = mAllocController->allocate(bufObj->data, allocFlags);
        if (0 != ret) {
            ALOGE("%s: Memory allocation for %d failed", __func__, i);
            return NO_MEMORY;
        }
        /* assigns share fd in v4l2_buffer structure */
        assignUsrPtr(bufObj->bufPtr, bufObj->data.fd);
        ALOGD_IF(isDebug(), "%s: Memory for %d allocated. fd: %d, "
        "dataPtr: %p", __func__, i, bufObj->data.fd, bufObj->data.base);
    }
    return err;
}

/* deallocates ion buffers */
status_t OutBufPool::deallocateIonBuffers()
{
    status_t err = NO_ERROR;
    int ret = 0;
    ALOGD_IF(isDebug(), "%s: E", __func__);

    for(uint32_t i = 0; i < mNumBufs; i++){
        bufObj_t* bufObj = &bufPool[i];

        /* Donot call free if the fd is invalid */
        if(bufObj->data.fd < 0)
            continue;

        ret = mMemAlloc->free_buffer(bufObj->data.base, bufObj->data.size,
                                    bufObj->data.offset, bufObj->data.fd);
        if(ret){
            ALOGE("%s: Memory deallocation for %d failed", __func__, i);
        }
        else {
            ALOGD_IF(isDebug(), "%s: Memory for %d deallocated",
                        __func__, i);
        }
    }
    return err;
}

/* assigns fd to v4l2_buffer structure */
void OutBufPool::assignUsrPtr(v4l2bufptr bufPtr, int32_t fd)
{
    if(bufPtr){
        for(int32_t j = 0; j < VIDEO_MAX_PLANES; j++){
            bufPtr->m.planes[j].m.userptr = fd;
        }
    }
    return;
}

//Returns the v4l2 buffer pointer for corresponding buffer index
//Returns NULL for invalid ID
v4l2bufptr OutBufPool::getEmptyBuf(const uint32_t bufIdx)
{
    return ((bufIdx < mNumBufs) ? bufPool[bufIdx].bufPtr : NULL);
}

//Returns the datapointer corresponding to buffer index
//Returns NULL for invalid ID
void* OutBufPool::getBufDataPtr(const uint32_t bufIdx)
{
    return ((bufIdx < mNumBufs) ? bufPool[bufIdx].data.base : NULL);
}
}; //namespace vpu
