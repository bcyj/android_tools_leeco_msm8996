/* =======================================================================
                              WFDMMSourceMutex.h
DESCRIPTION

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/inc/WFDMMSourceMutex.h

========================================================================== */

#ifndef _VENC_TEST_MUTEX_H
#define _VENC_TEST_MUTEX_H

/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/
#include "OMX_Core.h"

  /**
   * @brief Mutex class
   */
  class Mutex
  {
    public:

      /**
       * @brief Constructor
       */
      Mutex();

      /**
       * @brief Destructor
       */
      ~Mutex();

      /**
       * @brief Locks the mutex
       */
      OMX_ERRORTYPE Lock();

      /**
       * @brief Unlocks the mutex
       */
      OMX_ERRORTYPE UnLock();
    private:

      void* m_pMutex;
  };


#endif // #ifndef _VENC_TEST_MUTEX_H
