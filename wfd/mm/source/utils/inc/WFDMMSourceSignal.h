/* =======================================================================
                              WFDMMSourceSignal.h
DESCRIPTION

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/inc/WFDMMSourceSignal.h

========================================================================== */

#ifndef _VENC_TEST_SIGNAL_H
#define _VENC_TEST_SIGNAL_H

/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/
#include "OMX_Core.h"


  /**
   * @brief Class for sending signals to threads
   *
   * Signals behave similarly to binary (not counting) semaphores.
   */
  class Signal
  {
    public:

      /**
       * @brief Constructor
       */
      Signal();

      /**
       * @brief Destructor
       */
      ~Signal();

    public:

      /**
       * @brief Set a signal
       */
      OMX_ERRORTYPE Set();

      /**
       * @brief Wait for signal to be set
       *
       * @param nTimeoutMillis Milliseconds before timeout. Specify 0 for infinite.
       */
      OMX_ERRORTYPE Wait(OMX_S32 nTimeoutMillis);

    private:

      void* m_pSignal;
  };
#endif // #ifndef _VENC_TEST_SIGNAL_H
