/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_SLEEPER_H
#define _VTEST_SLEEPER_H

#include "OMX_Core.h"

namespace vtest {

/**
 * @brief Utility class for sleeping
 */
class Sleeper {
public:

    /**
     * @brief Sleep for the specified time
     *
     * @param nTimeMillis The time to sleep in milliseconds
     */
    static OMX_ERRORTYPE Sleep(OMX_S32 nTimeMillis);

private:
    Sleeper() { }
    ~Sleeper() { }
};
} // namespace vtest

#endif // #ifndef _VTEST_SLEEPER_H
