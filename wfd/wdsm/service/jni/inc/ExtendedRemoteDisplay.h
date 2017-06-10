/*==============================================================================
*       ExtendedRemoteDisplay.h
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

#ifndef EXT_REMOTE_DISPLAY_H
#define EXT_REMOTE_DISPLAY_H

#include <gui/IGraphicBufferProducer.h>
#include <gui/SurfaceComposerClient.h>
#include <media/stagefright/SurfaceMediaSource.h>
#include <media/stagefright/Utils.h>
#include <stdint.h>
#include <sys/types.h>
#include <utils/RefBase.h>
#include <media/stagefright/foundation/ABase.h>
#include "MMDebugMsg.h"

using namespace android;

class ExtendedRemoteDisplay
{
public:
    //--------------------------------------------------------------------------
    // Public Methods
    //--------------------------------------------------------------------------
     static ExtendedRemoteDisplay *createNativeHelper();


     ExtendedRemoteDisplay();

     ~ExtendedRemoteDisplay();

     IGraphicBufferProducer* createSurface(int width, int height);

     int destroySurface(IGraphicBufferProducer* surface);

private:
     sp<SurfaceMediaSource>         mpSurfaceMediaSrc; //! Pointer to SurfaceMediaSource
     sp<IGraphicBufferProducer>     mpProducer;     //! Surface  texture Instance
};
#endif //QC_REMOTE_DISPLAY_H
