/************************************************************************
Copyright (c)2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
************************************************************************/

#ifndef QIMSCM_ICONNECTIONMANAGERSERVICE_H
#define QIMSCM_ICONNECTIONMANAGERSERVICE_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "IConnectionManagerClient.h"
#include "IConnectionManagerBinder.h"

using namespace android;

namespace IMSConnectionManager
{

	class IConnectionManagerClient;
	class IConnectionManagerBinder;

    class IConnectionManagerService : public IInterface
    {
    public:
        enum
        {
            START_SERVICE = IBinder::FIRST_CALL_TRANSACTION,
        };
    public:
        DECLARE_META_INTERFACE(ConnectionManagerService);

        virtual sp<IConnectionManagerBinder> startService(const sp<IConnectionManagerClient>& connectionManagerClient) = 0;

    };

    class BnConnectionManagerService : public BnInterface<IConnectionManagerService>
    {
    public:
        virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
    };
};

#endif
