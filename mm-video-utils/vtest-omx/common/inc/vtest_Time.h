/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_TIME_H
#define _VTEST_TIME_H

#include "OMX_Core.h"

namespace vtest {

/**
 * @brief Utility class for getting system time
 */
class Time {
public:

    /**
     * @brief Get the timestamp in microseconds
     */
    static OMX_TICKS GetTimeMicrosec();

private:
    Time() { }
    ~Time() { }
};
} // namespace vtest

#endif // #ifndef _VTEST_TIME_H
