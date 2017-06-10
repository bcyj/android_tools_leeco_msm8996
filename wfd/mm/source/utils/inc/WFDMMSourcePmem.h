/* =======================================================================
                              WFDMMSourcePmem.h
DESCRIPTION

Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/inc/WFDMMSourcePmem.h

========================================================================== */

#ifndef _VENC_TEST_PMEM_H
#define _VENC_TEST_PMEM_H

/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/
#include "OMX_Core.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <linux/msm_q6venc.h>
#include <linux/android_pmem.h>
#include "WFDMMSourceDebug.h"
#include <sys/mman.h>


  /**
   * @brief Class for allocating / deallocating pmem buffers.
   *
   * Underlying implementation is hidden. Optionally allocate a single pool of
   * memory and slice into multiple buffers.
   */
  class Pmem
  {
    public:

      /**
       * @brief Constructor
       */
      Pmem(OMX_S32 nBuffers);

      /**
       * @brief Destructor
       */
      ~Pmem();

    public:

      OMX_ERRORTYPE Allocate(OMX_U8** ppBuffer,
          OMX_S32 nBytes);

      OMX_ERRORTYPE Free(OMX_U8* pBuffer);

    private:

      Pmem() {} // don't allow default constructor
      OMX_ERRORTYPE pmem_alloc(struct venc_pmem *pBuf, int size);
      OMX_ERRORTYPE pmem_free(struct venc_pmem *pBuf);

    private:

      OMX_S32 m_nBuffers;
      OMX_S32 m_nBuffersAlloc;
      struct venc_pmem* m_pBufferInfo;

  };

#endif // #ifndef _VENC_TEST_PMEM_H
