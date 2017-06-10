/******************************************************************************

                    I P H O N E S E R V I C E . H

******************************************************************************/

/******************************************************************************

  @file    IPhoneService.h
  @brief   Native end of the implementation

******************************************************************************/
/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef I_PHONESERVICE_H
#define I_PHONESERVICE_H

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include "common_log.h"

namespace android {

class IPhoneService: public IInterface
{
public:
    DECLARE_META_INTERFACE(PhoneService);

    virtual bool    enableDataConnectivity(bool value) = 0;
};

// ----------------------------------------------------------------------------
class BnPhoneService: public BnInterface<IPhoneService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

inline sp<IPhoneService> getIPhoneService() {
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder;
    do {
        if(sm == 0)
            return NULL;

        binder = sm->getService(String16("dun"));
        if (binder != 0) {
            break;
        }
        LOGW("dun service not published, waiting...");
        /*
         * Retry afer 5s and yield in the cases when dun service is
         * is about to be published
         */
        usleep(5000000); // 5 s
    } while(true);

    return interface_cast<IPhoneService>(binder);
}


inline bool IPhoneService_enableDataConnectivity(bool value) {
    sp<IPhoneService> t = getIPhoneService();
    if (t != NULL) return t->enableDataConnectivity(value);
    return false;
}


};
#endif // I_PHONESERVICE_H

