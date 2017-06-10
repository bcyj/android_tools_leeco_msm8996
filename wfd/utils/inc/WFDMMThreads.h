#ifndef __WFDMMTHREADS_H
#define __WFDMMTHREADS_H
/*==============================================================================
*       WFDMMThreads.h
*
*  DESCRIPTION:
*       Class declaration for WFDMMThreads. This provides trhead services
*   for WFD MM Modules
*
*
*  Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
03/25/2013         SK            InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/
#include "MMThread.h"
#include "MMSignal.h"

#define WFD_MM_THREAD_MAX_SIGNALS 16

typedef void (*WFDMMThreadCbType)(void*,unsigned int);


class WFDMMThreads
{
public:
    WFDMMThreads(unsigned int nNumSignals);

    ~WFDMMThreads();

    int Start
    (
        WFDMMThreadCbType pfn,
        int               nPriority,
        int               nStackSize,
        void*             nClientData,
        const char       *name
    );

    int SetSignal(unsigned int);

private:
    int ThreadLoop();
    static int ThreadEntry(void *pThis);

    MM_HANDLE    mSignals[WFD_MM_THREAD_MAX_SIGNALS];
    MM_HANDLE    mSignalQ;
    WFDMMThreadCbType mFnCb;
    bool         mbReady;
    unsigned int *mpnSignalVals;
    unsigned int mnNumSignals;
    MM_HANDLE    mExitSignal;
    MM_HANDLE    mThread;
    void*        mClientData;
    int          mPriority;
};
#endif /*__WFDMMTHREADS_H*/
