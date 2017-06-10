#ifndef _WFD_IWIFIDISPLAYLISTENER_H_
#define _WFD_IWIFIDISPLAYLISTENER_H_

/*==============================================================================
*       IWiFiDisplayListener.h
*
*  DESCRIPTION:
*       Class declaration IWiFiDisplayListener
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
#include "UIBCDefs.h"

using namespace android;

class IWiFiDisplayListener : public IInterface
{

public:

    DECLARE_META_INTERFACE(WiFiDisplayListener);

    virtual int notify(const char* eName, int numObjects,
        char strArray[][256], const Parcel* obj = NULL) = 0; //one-way

    virtual int notifyUIBCGenericEvent(WFD_uibc_event_t* ev,
        void * pClientData ) = 0; //one-way

    virtual int notifyUIBCHIDEvent(uint8* HIDPack, uint8 len,
        HIDDataType type) = 0; //one-way

private:

    FORBID_CONSTRUCTORS(IWiFiDisplayListener);

};

// ----------------------------------------------------------------------------

class BnWifiDisplayListener : public BnInterface<IWiFiDisplayListener>
{

public:
    virtual int onTransact( uint32_t code,
                            const Parcel& data,
                            Parcel* reply,
                            uint32_t flags = 0);
};

#endif // _WFD_IWIFIDISPLAYLISTENER_H_
