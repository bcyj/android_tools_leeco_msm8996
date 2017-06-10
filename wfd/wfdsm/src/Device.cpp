/*==============================================================================
*        @file Device.cpp
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

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Device_CPP"

#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include "Device.h"
#include "MMCapability.h"


Device::Device(string macAddress, DeviceType devType){
    macAddr = macAddress;
    ipAddr = string();

    switch ((unsigned int)devType) {
    case SOURCE:
        wfdDeviceInfoBitmap = 0 << WFD_DEVICETYPE_OFFSET;
        break;
    case PRIMARY_SINK:
        wfdDeviceInfoBitmap = 1 << WFD_DEVICETYPE_OFFSET;
        break;
    case SECONDARY_SINK:
        wfdDeviceInfoBitmap = 2 << WFD_DEVICETYPE_OFFSET;
        break;
    }

    pMMCapability = new MMCapability();
    pNegotiatedMMCapability = new MMCapability();
    pCommonCapability = new MMCapability();

}


Device::~Device() {
    if (pCommonCapability)
      delete pCommonCapability;
    if (pMMCapability)
      delete pMMCapability;
    if (pNegotiatedMMCapability)
      delete pNegotiatedMMCapability;
}


DeviceType Device::getDeviceType() const {
    switch (WFD_DEVICETYPE_VALUE(wfdDeviceInfoBitmap)) {
    case SOURCE:
        return SOURCE;
    case PRIMARY_SINK:
        return PRIMARY_SINK;
    case SECONDARY_SINK:
        return SECONDARY_SINK;
    }
    return UNKNOWN;
}

void Device::setDeviceType(DeviceType devType) {
    wfdDeviceInfoBitmap &= ~WFD_DEVICETYPE_BITMASK;
    switch ((unsigned int)devType) {
    case SOURCE:
        wfdDeviceInfoBitmap |= 0 << WFD_DEVICETYPE_OFFSET;
        break;
    case PRIMARY_SINK:
        wfdDeviceInfoBitmap |= 1 << WFD_DEVICETYPE_OFFSET;
        break;
    case SECONDARY_SINK:
        wfdDeviceInfoBitmap |= 2 << WFD_DEVICETYPE_OFFSET;
        break;
    }
}

