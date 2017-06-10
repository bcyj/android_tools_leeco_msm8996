/*==============================================================================
*       WFDMMSinkMediaSource.cpp
*
*  DESCRIPTION:
*       Connects RTP decoder and parser and provides Audio and Video samples to
*       the media framework.
*
*
*  Copyright (c) 2013 -2014 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "WFDMMThreads.h"
#include "MMMalloc.h"
#include "WFDMMLogs.h"


WFDMMThreads::WFDMMThreads(unsigned int nNumSignals)
{
    int nRet = 0;

    mSignalQ      = NULL;
    mFnCb         = NULL;
    mbReady       = false;
    mpnSignalVals = NULL;
    mnNumSignals  = 0;
    mExitSignal   = NULL;
    mClientData   = 0;
    mPriority     = 0;
    mThread       = NULL;
    /*--------------------------------------------------------------------------
      Basically memset "this" to 0.
    ----------------------------------------------------------------------------
    */
    for(int i = 0; i < WFD_MM_THREAD_MAX_SIGNALS; i++)
    {
        mSignals[i] = NULL;
    }

    if(nNumSignals >= WFD_MM_THREAD_MAX_SIGNALS)
    {
        WFDMMLOGE("WFDMMThreads Num of signals exceeds limit");
        return;
    }

    /*--------------------------------------------------------------------------
        Store Num signals. will be useful in destructor
    ----------------------------------------------------------------------------
    */
    mnNumSignals = nNumSignals;

    if(nNumSignals)
    {
        /*----------------------------------------------------------------------
           If client wants to use Signals Provide a Signal Q
        ------------------------------------------------------------------------
        */
        nRet = MM_SignalQ_Create(&mSignalQ);

        if(nRet)
        {
            WFDMMLOGE("Failed to create Signal Q");
            return;
        }

        /*----------------------------------------------------------------------
         MM OSAL seem to be taking a pointer to signal value that the signal
         itself. In that case we need to allocate some memory for signal values
        ------------------------------------------------------------------------
        */
        mpnSignalVals = (unsigned int*)MM_Malloc((nNumSignals + 1) *
                                                 sizeof(unsigned int));

        if(!mpnSignalVals)
        {
            WFDMMLOGE("Failed to allocate Signals");
            return;
        }

        for(unsigned int i = 0; i < nNumSignals; i++)
        {
            mpnSignalVals[i] = i;
            nRet = MM_Signal_Create(mSignalQ,
                                    (void*)(&mpnSignalVals[i]),
                                    NULL,
                                    &mSignals[i]);
            if(nRet)
            {
                WFDMMLOGE1("Failed to allocate Signal %d", i);
                return;
            }
        }

        /*----------------------------------------------------------------------
            Create the Exit Signal
        ------------------------------------------------------------------------
        */
        mpnSignalVals[nNumSignals] = nNumSignals;
        nRet = MM_Signal_Create(mSignalQ,
                                (void*)(&mpnSignalVals[nNumSignals]),
                                NULL,
                                &mExitSignal);
        if(nRet)
        {
            WFDMMLOGE("Failed to allocate Exit Signal");
            return;
        }
    }
    mbReady = true;
}

/*==============================================================================

         FUNCTION:         Start

         DESCRIPTION:
*//**       @brief         User starts a new thread by calling this function
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pFn         - User's callback funtion
                           nPriority   - Priority of the thread desired
                           nStackSize  - Desired stack size
                           nClientData - ClientData returned with callback
                           name        - Desired name of the thread

*//*     RETURN VALUE:
*//**       @return
                           0 - success
                          -1 - fail
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
int WFDMMThreads::Start
(
    WFDMMThreadCbType pfn,
    int  nPriority,
    int  nStackSize,
    void  *nClientData,
    const char *name
)
{
    int nRet = 0;

    /*--------------------------------------------------------------------------
       Check if all resources needed for the thread are available
    ----------------------------------------------------------------------------
    */
    if(!mbReady)
    {
        WFDMMLOGE("WFDMMThreads not ready. Somethng wrong fix it");
        return -1;
    }

    if(!pfn || !nStackSize)
    {
        WFDMMLOGE("WFDMMThreads:Start Invalid Args");
        return -1;
    }

    mClientData = nClientData;
    mPriority   = nPriority;
    mFnCb       = pfn;

    nRet =  MM_Thread_CreateEx(nPriority,
                               0,
                               WFDMMThreads::ThreadEntry,
                               this,
                               nStackSize,
                               name && strlen(name) ? name: "WfdMMThread",
                               &mThread);
    if(nRet)
    {
        mbReady = false;
        WFDMMLOGE("Failed to create WFDMMThread");
        return -1;
    }
    return 0;
}

/*==============================================================================

         FUNCTION:         SetSignal

         DESCRIPTION:
*//**       @brief         Sets a specified signal
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         nSignal - Index of the signal to set

*//*     RETURN VALUE:
*//**       @return
                           0 - success
                          -1 - fail
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
int WFDMMThreads::SetSignal
(
    unsigned int nSignal
)
{
    /*--------------------------------------------------------------------------
       Check if all resources needed for the thread are available
    ----------------------------------------------------------------------------
    */
    if(!mbReady)
    {
        WFDMMLOGE("WFDMMThreads not ready. Somethng wrong fix it");
        return -1;
    }

    if(nSignal >= mnNumSignals)
    {
        WFDMMLOGE("WFDMMThreads:SetSignal Invalid Args");
        return -1;
    }

    if(mSignals[nSignal])
    {
        return MM_Signal_Set(mSignals[nSignal]);
    }
    return -1;
}


/*==============================================================================

         FUNCTION:         ThreadEntry

         DESCRIPTION:
*//**       @brief         Thread Entry function
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pThis - this pointer of the object

*//*     RETURN VALUE:
*//**       @return
                           0 - success
                          -1 - fail
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
int WFDMMThreads::ThreadEntry(void *pThis)
{
    WFDMMThreads *pMe = (WFDMMThreads*)pThis;

    if(!pMe)
    {
        WFDMMLOGE("WFDMMThreads Failed in Thread Entry");
        return 0;
    }

    pMe->ThreadLoop();

    return 0;
}

/*==============================================================================

         FUNCTION:         ThreadLoop

         DESCRIPTION:
*//**       @brief         Main processing loop running in the created thread
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return
                           0 - success
                          -1 - fail
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
int WFDMMThreads::ThreadLoop()
{
    bool bRunning = true;
    WFDMMLOGH("WFDMMThreads:: Loop");
    while ( bRunning )
    {
        unsigned int *pEvent = NULL;

        if(mnNumSignals == 0)
        {
            if(mFnCb)
            {
                mFnCb(mClientData, 0);
            }
        }
        else if (0 == MM_SignalQ_Wait(mSignalQ, (void **)&pEvent))
        {
            if(*pEvent == mnNumSignals)
            {
                WFDMMLOGH("WFDMMThreads Received Exit Signal");
                bRunning = false;
            }
            else
            {
                if(mFnCb)
                {
                    mFnCb(mClientData, *pEvent);
                }
            }
        }
    }
    MM_Thread_Exit(mThread, 0);
    return 0;
}

/*==============================================================================

         FUNCTION:         ~WFDMMThreads

         DESCRIPTION:
*//**       @brief         Destructor of the WFDMMThread class
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param

*//*     RETURN VALUE:
*//**       @return

@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
WFDMMThreads::~WFDMMThreads()
{
    if(mExitSignal && mThread)
    {
        int exitCode = 0;
        MM_Signal_Set(mExitSignal);
        MM_Thread_Join(mThread, &exitCode );
    }

    for(unsigned int i = 0; i < mnNumSignals; i++)
    {
        if(mSignals[i])
        {
            MM_Signal_Release(mSignals[i]);
        }
    }

    if(mExitSignal)
    {
        MM_Signal_Release(mExitSignal);
    }

    if(mSignalQ)
    {
        MM_SignalQ_Release(mSignalQ);
    }

    if(mpnSignalVals)
    {
        MM_Free(mpnSignalVals);
    }

    if(mThread)
    {
        MM_Thread_Release(mThread);
    }
    /*--------------------------------------------------------------------------
       Phew.... Cleaned up
    ----------------------------------------------------------------------------
    */
    return;
}

