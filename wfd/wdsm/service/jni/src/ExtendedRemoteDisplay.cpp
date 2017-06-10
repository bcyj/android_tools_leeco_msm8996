/*==============================================================================
*       ExtendedRemoteDisplay.cpp
*
*  DESCRIPTION:
*       Native side helper class for ExtendedRemoteDisplay.java. This provides all
*    services that requires a native interface.
*
*  Copyright (c) 2013-2014 by Qualcomm Technologies, Incorporated.
*  All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
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

#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>


#include <utils/Log.h>
#ifndef WFD_ICS
#include "common_log.h"
#endif
#include "ExtendedRemoteDisplay.h"
#include "MMDebugMsg.h"
#include "gralloc_priv.h"


#define EXTRDLOGE(x) MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, x)
#define EXTRDLOGH(x) MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, x)
#define EXTRDLOGM(x) MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, x)

#define EXTRDLOGE1(x,y) MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, x, y)
#define EXTRDLOGH1(x,y) MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, x, y)
#define EXTRDLOGM1(x,y) MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, x, y)

ExtendedRemoteDisplay::ExtendedRemoteDisplay()
{
    EXTRDLOGH("EXTRD constructor");

    mpSurfaceMediaSrc = NULL;
    mpProducer     = NULL;

}

ExtendedRemoteDisplay*  ExtendedRemoteDisplay::createNativeHelper()
{
    //--------------------------------------------------------------------------
    //  Create an instance of ExtendedRemoteDisplay.
    //--------------------------------------------------------------------------
    ExtendedRemoteDisplay *pMe = new ExtendedRemoteDisplay();

    if(!pMe)
    {
        EXTRDLOGE("Unable to create Native RemoteDisplay Instance");
    }

    return pMe;

}

IGraphicBufferProducer* ExtendedRemoteDisplay::createSurface(int width, int height)
{

    //--------------------------------------------------------------------------
    // Create an instance of surface media source for surface texture.
    // Write back from this Media source will not be used. That is, read API
    // will not be called.
    //--------------------------------------------------------------------------
    mpSurfaceMediaSrc = new SurfaceMediaSource(width, height);

    if(mpSurfaceMediaSrc == NULL)
    {
        EXTRDLOGE("Unable to create Surface media Source");
        return NULL;
    }

    EXTRDLOGH("EXTRD Created SurfaceMediaComposer");

    mpProducer = mpSurfaceMediaSrc->getProducer();

    if(mpProducer == NULL)
    {
        EXTRDLOGE("Unable to obtain Producer from SurfaceMediaSource");
        return NULL;
    }

    EXTRDLOGH("EXTRD returning Surface texture");

    return mpProducer.get();
}

int ExtendedRemoteDisplay::destroySurface(IGraphicBufferProducer* pBufQ)
{
    if(pBufQ == mpSurfaceMediaSrc->getProducer().get())
    {
        //----------------------------------------------------------------------
        // Check if surface texture belongs to this instance of media source
        //----------------------------------------------------------------------
        EXTRDLOGH("EXTRD Release surface texture");
        return 0;
    }
    else
    {
        EXTRDLOGE("EXTRD Unknown texture provided for release.. Fail");
        return -1;
    }
}

ExtendedRemoteDisplay::~ExtendedRemoteDisplay()
{
    EXTRDLOGH("EXTRD Destructor");
    return;
}
