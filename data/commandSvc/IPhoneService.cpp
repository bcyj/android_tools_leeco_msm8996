/******************************************************************************

                  I P H O N E S E R V I C E . CPP

******************************************************************************/

/******************************************************************************

  @file    IPhoneService.cpp
  @brief   Proxy / stub implementation of IPhoneService service

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

#include "IPhoneService.h"

namespace android {

enum {
    TRANSACTION_enableDataConnectivity = (IBinder::FIRST_CALL_TRANSACTION ),
};

class BpPhoneService: public BpInterface<IPhoneService>
{
public:
    BpPhoneService(const sp<IBinder>& impl)
        : BpInterface<IPhoneService>(impl)
    {
    }

    virtual bool enableDataConnectivity(bool value)
    {
        Parcel data, reply;
        int enable = value ? 1:0;
        data.writeInterfaceToken(IPhoneService::getInterfaceDescriptor());
        data.writeInt32(enable);
        remote()->transact(TRANSACTION_enableDataConnectivity, data, &reply);
        if (reply.readInt32())
            return false;
        return (0!=reply.readInt32());
    }

};

IMPLEMENT_META_INTERFACE(PhoneService, "com.qualcomm.qualcommsettings.IDun");

};
