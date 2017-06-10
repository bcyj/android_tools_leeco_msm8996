/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_BUFFER_MANAGER_H
#define _VTEST_BUFFER_MANAGER_H

#include <vector>
#include "OMX_Core.h"
#include "vtest_ComDef.h"
#include "OMX_Component.h"
#include "OMX_QCOMExtns.h"
#include "vtest_Mutex.h"

using std::vector;

namespace vtest {

class ISource;

struct PortBufferCapability
{
    OMX_BOOL bAllocateBuffer;
    OMX_BOOL bUseBuffer;
    ISource *pSource;
    OMX_U32 ePortIndex;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_U32 nMinBufferSize;
    OMX_U32 nMinBufferCount;
    OMX_U32 nExtraBufferCount;
    OMX_U32 nBufferUsage;
    /* Use same buffer pool for both ports,
     * this can only work if all the options are same
     * and the node can allocate buffers on both ports
     * These checks are enforced by the buffermanager */
    OMX_BOOL bUseSameBufferPool;
};

struct Node {
    ISource *pSource;
    OMX_U32 ePortIndex;
    PortBufferCapability sPortCaps;
};

struct BufferPool {
    Mutex *pLock;
    Node *pAllocateNode;
    Node *pUseNode;
    BufferInfo *pBuffers;
    OMX_U32 nWidth;
    OMX_U32 nHeight;
    OMX_U32 nBufferCount;
    OMX_U32 nBufferSize;
    OMX_U32 nBufferUsage;
};

class BufferManager {

public:

    BufferManager();
    ~BufferManager();

    OMX_ERRORTYPE GetBuffers(ISource *pNode,
            OMX_U32 ePortIndex, BufferInfo **pBuffers, OMX_U32 *nBufferCount);
    OMX_ERRORTYPE GetBuffer(ISource *pNode, OMX_U32 ePortIndex,
            OMX_BUFFERHEADERTYPE *pHeader, BufferInfo **pBuffer);

    OMX_ERRORTYPE FreeBuffers(ISource *pNode, OMX_U32 ePortIndex);
    OMX_ERRORTYPE SetupBufferPool(ISource *pNodeSrc, ISource *pNodeSink);
private:

    BufferPool* GetBufferPool(ISource *pNode1, ISource *pNode2);
    BufferPool* GetBufferPool(ISource *pNode, OMX_U32 ePortIndex);
    BufferPool* GetBufferPool(ISource *pNode, BufferInfo *pBuffer);

    vector<BufferPool*> m_pBufferPools;
};

}

#endif // #ifndef _VTEST_BUFFER_MANAGER_H
