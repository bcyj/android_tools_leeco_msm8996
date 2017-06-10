/* =======================================================================
                              WFDMMSourcePmem.cpp
DESCRIPTION

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/src/WFDMMSourcePmem.cpp

========================================================================== */

/*========================================================================
  Include Files
 ==========================================================================*/
#include "WFDMMSourcePmem.h"
#include "MMDebugMsg.h"

#include <string.h>

// pmem include files
/*
extern "C"
{
   #include "pmem.h"
}
*/


  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Pmem::Pmem(OMX_S32 nBuffers)
    : m_nBuffers(nBuffers),
    m_nBuffersAlloc(0),
    m_pBufferInfo(new venc_pmem[nBuffers])
  {
    if (m_pBufferInfo != NULL)
    {
      memset(m_pBufferInfo, 0, sizeof(struct venc_pmem) * nBuffers);
    }
    // do nothing
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Pmem::~Pmem()
  {
    if (m_pBufferInfo != NULL)
    {
      delete [] m_pBufferInfo;
    }
  }

  OMX_ERRORTYPE Pmem::pmem_alloc(struct venc_pmem *pBuf, int size)
  {
    struct pmem_region region;

    //QC_OMX_MSG_HIGH("Opening pmem files with size 0x%x...",size,0,0);
    pBuf->fd = open("/dev/pmem_adsp", O_RDWR);

    if (pBuf->fd < 0) {
      //QC_OMX_MSG_ERROR("error could not open pmem device");
      return OMX_ErrorInsufficientResources;
    }

    pBuf->offset = 0;
    pBuf->size = (size + 4095) & (~4095);

    /* QC_OMX_MSG_HIGH("Allocate pmem of size:0x%x, fd:%d \n", pBuf->size, pBuf->fd, 0); */
    pBuf->virt = mmap(NULL, pBuf->size, PROT_READ | PROT_WRITE,
        MAP_SHARED, pBuf->fd, 0);

    //QC_OMX_MSG_HIGH("Allocate pmem of size:0x%x, fd:%d  virt:%p\n", pBuf->size, pBuf->fd, pBuf->virt);
    if (pBuf->virt == MAP_FAILED) {
   //   QC_OMX_MSG_ERROR("error mmap failed with size:%d",size);
      close(pBuf->fd);
      pBuf->fd = -1;
      return OMX_ErrorInsufficientResources;
    }

    return OMX_ErrorNone;
  }

  OMX_ERRORTYPE Pmem::pmem_free(struct venc_pmem* pBuf)
  {
    //QC_OMX_MSG_HIGH("Free pmem of size:0x%x, fd:%d \n",pBuf->size, pBuf->fd, 0);
    close(pBuf->fd);
    pBuf->fd = -1;
    munmap(pBuf->virt, pBuf->size);
    pBuf->offset = 0;
    pBuf->phys = pBuf->virt = NULL;
    return OMX_ErrorNone;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Pmem::Allocate(OMX_U8 ** ppBuffer,
      OMX_S32 nBytes)
  {
    void *pVirt;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    static const OMX_S32 PMEM_ALIGN = 4096;
    static const OMX_U32 PMEM_ALIGN_MASK = 0xFFFFF000;
    int nBytesAlign;

    if (ppBuffer != NULL && nBytes > 0)
    {
      if (m_nBuffersAlloc < m_nBuffers)
      {
        OMX_S32 i = 0;
        struct venc_pmem* pBuffer = NULL;

        // look for a free info structure
        while (pBuffer == NULL)
        {
          if (m_pBufferInfo[i].virt == NULL)
          {
            pBuffer = &m_pBufferInfo[i];
          }
          ++i;
        }

        if (pBuffer != NULL)
        {
          nBytesAlign = ((int)nBytes + 4095) & (~4095);
          pmem_alloc(pBuffer, nBytesAlign);

          pVirt = pBuffer->virt;
          if (pVirt != NULL)
          {
            /* pInfo->pVirt = (OMX_U8*) (((OMX_U32) (pVirt + PMEM_ALIGN)) & PMEM_ALIGN_MASK);
               pInfo->pVirtBase = pBuf->virt;
               pInfo->nBytes = nBytes;
               pInfo->nBytesAlign = nBytesAlign; */

            MM_MSG_PRIO3(MM_GENERAL,MM_PRIO_HIGH,"Allocate buffer: fd:%d, offset:%d, pVirt=%p \n", pBuffer->fd,
                pBuffer->offset, pBuffer->virt);
            // --susan
            *ppBuffer = (OMX_U8 *)pBuffer;
            /* *ppBuffer = (OMX_U8 *)pInfo->virt; */
            ++m_nBuffersAlloc;
          }
          else
          {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"pmem alloc failed");
            result = OMX_ErrorUndefined;
          }
        }
        else
        {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"error finding buffer");
          result = OMX_ErrorUndefined;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"ran out of buffers");
        result = OMX_ErrorUndefined;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"bad params");
      result = OMX_ErrorBadParameter;
    }
    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Pmem::Free(OMX_U8* pBuffer)
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;
    if (pBuffer != NULL)
    {
      OMX_S32 i = 0;
      struct venc_pmem* pInfo = NULL;

      // look for a free info structure
      while (i < m_nBuffers )
      {
        if (&m_pBufferInfo[i] == (struct venc_pmem *)pBuffer)
        {
          pInfo = &m_pBufferInfo[i];
          break;
        }
        ++i;
      }

      if (pInfo != NULL)
      {
        pmem_free(pInfo);
        --m_nBuffersAlloc;
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"error finding buffer");
        result = OMX_ErrorUndefined;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"bad params");
      result = OMX_ErrorBadParameter;
    }
    return result;
  }


