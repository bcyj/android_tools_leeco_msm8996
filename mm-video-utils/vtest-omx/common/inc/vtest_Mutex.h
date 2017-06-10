/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_MUTEX_H
#define _VTEST_MUTEX_H

#include "OMX_Core.h"

namespace vtest {
/**
 * @brief Mutex class
 */
class Mutex {

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

    class Autolock {

    public:
        Autolock(Mutex *pMutex);
        ~Autolock();

    private:
        Autolock();
        Mutex *m_pMutex;
    };

private:
    void *m_pMutex;
};

}

#endif // #ifndef _VTEST_MUTEX_H
