/************************************************************************
Copyright (c)2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
************************************************************************/

#ifndef QIMSCM_ICONNECTIONMANAGERBINDER_H
#define QIMSCM_ICONNECTIONMANAGERBINDER_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>

#include "IConnectionManagerClient.h"
#include "ConnectionManagerBase.h"
#include "IConnectionBinder.h"
#include "IConnectionClient.h"

using namespace android;

namespace IMSConnectionManager
{
    class IConnectionManagerClient;

	class IConnectionClient;

	class IConnectionBinder;

    class IConnectionManagerBinder : public IInterface
    {
    public:
        DECLARE_META_INTERFACE(ConnectionManagerBinder);

        virtual status_t startService(const sp<IConnectionManagerClient>& client) = 0;

        virtual status_t initialize() = 0;

        virtual sp<IConnectionBinder> createConnection(const sp<IConnectionClient> connectionClient, char* uri) = 0;

        virtual status_t getConfiguration(uint32_t configurationType, union CMConfiguration* cmConfiguration) = 0;

		virtual status_t close() = 0;

		virtual status_t triggerRegistration() = 0;

        virtual status_t addListener() = 0;
    };

    class BnConnectionManagerBinder : public BnInterface<IConnectionManagerBinder>
    {
    public:
        virtual status_t onTransact( uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
    };
};

#endif
