/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_SIGNAL_H
#define _VTEST_SIGNAL_H

#include "OMX_Core.h"

namespace vtest {

/**
 * @brief Class for sending signals to threads
 *
 * Signals behave similarly to binary (not counting) semaphores.
 */
class Signal {

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

    /**
     * @brief Broadcast a signal
     */
    OMX_ERRORTYPE Broadcast();

private:
    void *m_pSignal;
};

}

#endif // #ifndef _VTEST_SIGNAL_H
