#ifndef _WFD_WIFIDISPLAYCLIENT_H_
#define _WFD_WIFIDISPLAYCLIENT_H_

/*==============================================================================
*       WiFiDisplayClient.h
*
*  DESCRIPTION:
*       Class declaration WiFiDisplayClient
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

class WiFiDisplayClient :
    public BnWifiDisplayListener
{

public:

    static int enableWfd (WfdDevice *localDevice,stringarray_callback cb);

    static int disableWfd ();

    static int startWfdSession (WfdDevice *peerDevice);

    static int stopWfdSession (int sessId);

    static int play_rtsp (int sessId, bool secureFlag);

    static int pause_rtsp (int sessId, bool secureFlag);

    static int standby_rtsp(int sessId);

    static int teardown_rtsp(int sessId, bool isRTSP);

    static int setRtpTransport(int transportType);

    static int queryTCPTransportSupport();

    static int setDecoderLatency(int latency);

    static int tcpPlaybackControl(int cmdType, int cmdVal);

    static int negotiateRtpTransport(int TransportType,int BufferLenMs,
                int portNum);

    static int executeRuntimeCommand(int cmd);

    static int setSurface(Surface* surface);

    static int getConfigItems(int** configItems, size_t* length);

    static void registerUibcCallbacks(
                wfd_uibc_send_event_cb SendEvent,
                wfd_uibc_hid_event_cb sendHIDEvent);

    static int setUIBC(int sessId);

    static int enableUIBC(int sessId);

    static int disableUIBC(int sessId);

    static int startUIBC();

    static int stopUIBC();

    static int sendUIBCEvent(const WFD_uibc_event_t& event);

    static int setAVPlaybackMode(int mode);

    static int setBitrate(int value);

    static int setResolution (int formatType, int value, int* resParams, int len);

    static int getResolution(int32_t* width, int32_t* height);

    static int getCommonResolution(uint32_t** bitmap, int32_t* numProf);

    static int getNegotiatedResolution(uint32_t** bitmap);

protected:

    virtual int notify(const char* eName, int numObjects,
        char strArray[][256], const Parcel* obj = NULL);

    virtual int notifyUIBCGenericEvent(WFD_uibc_event_t* ev,
        void * pClientData );

    virtual int notifyUIBCHIDEvent(uint8* HIDPack, uint8 len,
        HIDDataType type);

    virtual ~WiFiDisplayClient();

private:

    FORBID_CONSTRUCTORS(WiFiDisplayClient);

    WiFiDisplayClient(){;};

    typedef void (*stringarray_callback)(const char* eName, int numObjects,
        char strArray[][256]);

    static const sp<IWiFiDisplayService>& getWiFiDisplayService();

    static sp<IWiFiDisplayService> sWiFiDisplayService;

    class WiFiDisplayServiceDeathRecepient: public IBinder::DeathRecipient
    {
        public:
        virtual ~WiFiDisplayServiceDeathRecepient();
        virtual void binderDied(const wp<IBinder>& who);
    };

    static sp<WiFiDisplayClient::WiFiDisplayServiceDeathRecepient> sDeathNotifier;
    static WiFiDisplayClient* spMe;
    static stringarray_callback clientCallback;
    static wfd_uibc_send_event_cb genericCallback;
    static wfd_uibc_hid_event_cb hidCallback;
};
#endif // _WFD_WIFIDISPLAYCLIENT_H_
