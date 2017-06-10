#ifndef _WFD_IWIFIDISPLAYSERVICE_H_
#define _WFD_IWIFIDISPLAYSERVICE_H_

/*==============================================================================
*       IWiFiDisplayService.h
*
*  DESCRIPTION:
*       Class declaration IWiFiDisplayService
*
*
*  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
11/06/2014                    InitialDraft
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

#include <stdint.h>
#include <sys/types.h>

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include "WiFiDisplayServiceUtils.h"
#include "IWiFiDisplayListener.h"
#include "WFDDefs.h"
#include "UIBCDefs.h"
#include <gui/IGraphicBufferProducer.h>

using namespace android;

class IWiFiDisplayService : public IInterface
{

public:

    DECLARE_META_INTERFACE(WiFiDisplayService);

    virtual int enableWfdService (const sp<IWiFiDisplayListener>& listener,
                                WfdDevice *localDevice) = 0;

    virtual int disableWfdService () = 0;

    virtual int startWfdSessionService(WfdDevice *peerDevice) = 0;

    virtual int stopWfdSessionService(int sessId) = 0;

    virtual int play (int sessId, bool secureFlag ) = 0;

    virtual int pause(int sessId, bool secureFlag ) = 0;

    virtual int standby(int sessId) = 0;

    virtual int teardown(int sessId, bool isRTSP) = 0;

    virtual int setRtpTransportType(int transportType) = 0;

    virtual int queryTCPSupport() = 0;

    virtual int setDecoderLatencyValue(int latency) = 0;

    virtual int tcpPlaybackControlCmd(int cmdType, int cmdVal) = 0;

    virtual int negotiateRtpTransportType(int TransportType,
                                            int BufferLenMs,int portNum) = 0;

    virtual int executeRuntimeCmd(int cmd) = 0;

    virtual int setVideoSurface(const sp<IGraphicBufferProducer>& surface) = 0;

    virtual int getConfigItems(int** configItems, size_t* length) = 0;

    virtual int setUIBCSession(int sessId) = 0;

    virtual int enableUIBCSession(int sessId) = 0;

    virtual int disableUIBCSession(int sessId) = 0;

    virtual int startUIBCDataPath() = 0;

    virtual int stopUIBCDataPath() = 0;

    virtual int transmitUIBCEvent(const WFD_uibc_event_t& event) = 0;

    virtual int setAVMode(int mode) = 0;

    virtual int setBitrateValue(int value) = 0;

    virtual int setSessionResolution(int formatType, int value, int* resParams,
                                            int len) = 0;

    virtual int getSessionResolution(int32_t* width, int32_t* height) = 0;

    virtual int getCommonResolutionBitmap(uint32_t** bitmap,
                                                    int32_t* numProf) = 0;

    virtual int getNegotiatedResolutionBitmap(uint32_t** bitmap) = 0;

private:

    FORBID_CONSTRUCTORS(IWiFiDisplayService);

};

// ----------------------------------------------------------------------------

class BnWiFiDisplayService : public BnInterface<IWiFiDisplayService>
{

public:
    virtual int onTransact( uint32_t code,
                            const Parcel& data,
                            Parcel* reply,
                            uint32_t flags = 0);
};

#endif // _WFD_IWIFIDISPLAYSERVICE_H_
