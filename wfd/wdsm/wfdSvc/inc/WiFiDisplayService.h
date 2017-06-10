#ifndef _WFD_WIFIDISPLAYSERVICE_H_
#define _WFD_WIFIDISPLAYSERVICE_H_

/*==============================================================================
*       WiFiDisplayService.h
*
*  DESCRIPTION:
*       Class declaration WiFiDisplayService
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

#include "IWiFiDisplayService.h"
#include <gui/Surface.h>

class WiFiDisplayService :
    public BnWiFiDisplayService
{

public:

    WiFiDisplayService();

    static void publishService();

    virtual int enableWfdService(const sp<IWiFiDisplayListener>& listener,
                              WfdDevice *localDevice);

    virtual int disableWfdService ();

    virtual int startWfdSessionService (WfdDevice *peerDevice);

    virtual int stopWfdSessionService (int sessId);

    virtual int play(int sessId, bool secureFlag );

    virtual int pause(int sessId, bool secureFlag );

    virtual int standby(int sessId);

    virtual int teardown(int sessId, bool isRTSP);

    virtual int setRtpTransportType(int transportType);

    virtual int queryTCPSupport();

    virtual int setDecoderLatencyValue(int latency);

    virtual int tcpPlaybackControlCmd(int cmdType, int cmdVal);

    virtual int negotiateRtpTransportType(int TransportType,
                 int BufferLenMs,int portNum);

    virtual int executeRuntimeCmd(int cmd);

    virtual int setVideoSurface(const sp<IGraphicBufferProducer>& surface);

    virtual int getConfigItems(int** configItems, size_t* length);

    virtual int setUIBCSession(int sessId);

    virtual int enableUIBCSession(int sessId);

    virtual int disableUIBCSession(int sessId);

    virtual int startUIBCDataPath();

    virtual int stopUIBCDataPath();

    virtual int transmitUIBCEvent(const WFD_uibc_event_t& event);

    virtual int setAVMode(int mode);

    virtual int setBitrateValue(int value);

    virtual int setSessionResolution(int formatType, int value, int* resParams,
                                            int len);

    virtual int getSessionResolution(int32_t* width, int32_t* height);

    virtual int getCommonResolutionBitmap(uint32_t** bitmap,
                                                    int32_t* numProf);

    virtual int getNegotiatedResolutionBitmap(uint32_t** bitmap);

protected:

    virtual ~WiFiDisplayService();

private:

    FORBID_CONSTRUCTORS(WiFiDisplayService);

    sp<IWiFiDisplayListener> mListener;
    sp<Surface> videoSurface;
    static WiFiDisplayService* sWiFiDisplayService;
    static void StringArrayCallback(const char* eName,
                                        int numObjects,
                                        char strArray[][256]);
    static boolean sendGenericEvent(WFD_uibc_event_t* ev, void * pClientData );

    static boolean sendHIDEvent(uint8* HIDPack, uint8 len, HIDDataType type);
};
#endif // _WFD_WIFIDISPLAYSERVICE_H_
