/*==============================================================================
*        @file Device.h
*
*  @par DESCRIPTION:
*        Device class.
*
*
*  Copyright (c) 2012 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:


when        who         what, where, why
--------    ------      --------------------------------------------------------

================================================================================
*/

#ifndef _DEVICE_H
#define _DEVICE_H

//#include "wdsm_mm_interface.h"
#include <string>
#include "WFDDefs.h"
using namespace std;

/* Forward Declarations */
class MMCapability;



enum Bits
{
    B0    = 0x00000001,
    B1    = 0x00000002,
    B2    = 0x00000004,
    B3    = 0x00000008,
    B4    = 0x00000010,
    B5    = 0x00000020,
    B6    = 0x00000040,
    B7    = 0x00000080,
    B8    = 0x00000100,
    B9    = 0x00000200,
};



#define PREFERREDCONNECTIVITY_P2P   false
#define PREFERREDCONNECTIVITY_TDLS  true

#define COUPLEDSINKSTATUS_NOTCOUPLED        0
#define COUPLEDSINKSTATUS_COUPLED           1
#define COUPLEDSINKSTATUS_TEARDOWNCOUPLING  2


#define WFD_DEVICETYPE_BITMASK                      (B0 | B1)
#define WFD_COUPLEDSINK_SUPPORTED_BY_SRC_BITMASK    (B2)
#define WFD_COUPLEDSINK_SUPPORTED_BY_SINK_BITMASK   (B3)
#define WFD_SESSION_AVAILABLE_BITMASK               (B4 | B5)
#define WFD_SERVICEDISCOVERY_SUPPORTED_BITMASK      (B6)
#define WFD_PREFERRED_CONNECTIVITY_BITMASK          (B7)
#define WFD_CONTENT_PROTECTION_SUPPORTED_BITMASK    (B8)
#define WFD_TIMESYNC_SUPPORTED_BITMASK              (B9)

#define WFD_DEVICETYPE_OFFSET                       0
#define WFD_COUPLEDSINK_SUPPORTED_BY_SRC_OFFSET     2
#define WFD_COUPLEDSINK_SUPPORTED_BY_SINK_OFFSET    3
#define WFD_SESSION_AVAILABLE_OFFSET                4
#define WFD_SERVICEDISCOVERY_SUPPORTED_OFFSET       6
#define WFD_PREFERRED_CONNECTIVITY_OFFSET           7
#define WFD_CONTENT_PROTECTION_SUPPORTED_OFFSET     8
#define WFD_TIMESYNC_SUPPORTED_OFFSET               9

#define WFD_DEVICETYPE_VALUE(bitmap)                        ((bitmap & WFD_DEVICETYPE_BITMASK) >> WFD_DEVICETYPE_OFFSET)
#define WFD_COUPLEDSINK_SUPPORTED_BY_SRC_VALUE(bitmap)      ((bitmap & WFD_COUPLEDSINK_SUPPORTED_BY_SRC_BITMASK) >> WFD_COUPLEDSINK_SUPPORTED_BY_SRC_OFFSET)
#define WFD_COUPLEDSINK_SUPPORTED_BY_SINK_VALUE(bitmap)     ((bitmap & WFD_COUPLEDSINK_SUPPORTED_BY_SINK_BITMASK) >> WFD_COUPLEDSINK_SUPPORTED_BY_SINK_OFFSET)
#define WFD_SESSION_AVAILABLE_VALUE(bitmap)                 ((bitmap & WFD_SESSION_AVAILABLE_BITMASK) >> WFD_SESSION_AVAILABLE_OFFSET)
#define WFD_SERVICEDISCOVERY_SUPPORTED_VALUE(bitmap)        ((bitmap & WFD_SERVICEDISCOVERY_SUPPORTED_BITMASK) >> WFD_SERVICEDISCOVERY_SUPPORTED_OFFSET)
#define WFD_PREFERRED_CONNECTIVITY_VALUE(bitmap)            ((bitmap & WFD_PREFERRED_CONNECTIVITY_BITMASK) >> WFD_PREFERRED_CONNECTIVITY_OFFSET)
#define WFD_CONTENT_PROTECTION_SUPPORTED_VALUE(bitmap)      ((bitmap & WFD_CONTENT_PROTECTION_SUPPORTED_BITMASK) >> WFD_CONTENT_PROTECTION_SUPPORTED_OFFSET)
#define WFD_TIMESYNC_SUPPORTED_VALUE(bitmap)                ((bitmap & WFD_TIMESYNC_SUPPORTED_BITMASK) >> WFD_TIMESYNC_SUPPORTED_OFFSET)






/**----------------------------------------------------------------------------
   Device class
-------------------------------------------------------------------------------
*/

class Device
{
public:
    string macAddr;
    string ipAddr;

    unsigned short wfdDeviceInfoBitmap;
    unsigned short sessionMngtControlPort;
    int            decoderLatency;
    unsigned short maxThroughput;
    unsigned char coupleSinkStatusBitmap;

    MMCapability* pMMCapability;
    MMCapability* pNegotiatedMMCapability;
    MMCapability* pUserCapability;
    MMCapability* pCommonCapability;

    string coupledPeerMacAddr;

    Device(string, DeviceType);
    ~Device();

    DeviceType getDeviceType() const;
    void setDeviceType(DeviceType devType);

};




#endif /*_DEVICE_H*/
