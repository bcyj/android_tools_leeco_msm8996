/*==============================================================================
*        @file MMAdaptor.h
*
*  @par DESCRIPTION:
*        Thread class.
*
*
*  Copyright (c) 2011 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:


when        who         what, where, why
--------    ------      --------------------------------------------------------

================================================================================
*/

#ifndef _MMADAPTOR_H
#define _MMADAPTOR_H


#include <unistd.h>
#include "wdsm_mm_interface.h"
#include "Device.h"




/**----------------------------------------------------------------------------
   MMAdaptor class
-------------------------------------------------------------------------------
*/

class MMAdaptor
{
private:
    static WFD_MM_HANDLE mmHandle;
    static bool mmCreated;
    static WFD_device_t deviceType_conversion(DeviceType);
    static bool mmHdcpCreated;
    static DeviceType mmDevType;

    static void capability_change_cb(void*);
    static void stream_play_cb(void*, WFD_status_t);
    static void stream_pause_cb(void*, WFD_status_t);
    static void capability_update_cb(void*, WFD_status_t);
    static void event_update_cb(WFD_MM_Event_t , WFD_status_t status, void *pEvtData);

    static void idr_trigger_cb(void*);
    static uint32 get_conn_speed_cb(void*);
    static void av_format_change_cb(void*);

    static bool isMMStreamEnabled();

    static WFD_MM_callbacks_t mmCallbacks;

public:
    MMAdaptor();
    ~MMAdaptor();

    static bool updateLocalMMCapability(MMCapability*, DeviceType);
    static bool getNegotiatedCapability(MMCapability*, MMCapability*, MMCapability*,MMCapability*);
    static bool getProposedCapability(MMCapability*, MMCapability*, MMCapability*);

    static bool createHDCPSession(DeviceType,MMCapability*, MMCapability*, MMCapability*);
    static bool createSession(MMCapability*, DeviceType);
    static bool destroySession();
    static bool streamPlay();
    static bool streamPlayPrepare();
    static bool streamPause();

    static bool standby();
    static bool resume();

    static bool updateSession(MMCapability*);

    static bool sendIDRFrame();
    static bool setFrameRate(int);
    static bool setBitRate(int);
    static bool executeRuntimeCommand (int);
    static bool getAVFormatChangeTiming(uint32*, uint32*);
    static bool streamControl(WFD_device_t eDeviceType,
                              WFD_MM_AV_Stream_Control_t control,
                              int controlValue);
    static uint64 getCurrentPTS();


};




#endif /*_MMADAPTOR_H*/
