/************************************************************************
Copyright (c)2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
************************************************************************/

#ifndef QIMSCM_ICONNECTIONMANAGERCLIENT_H
#define QIMSCM_ICONNECTIONMANAGERCLIENT_H

#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "ConnectionManagerBinderListener.h"
//#include "ConnectionManagerBinder.h"


using namespace android;

namespace IMSConnectionManager
{
    class IConnectionManagerClient: public IInterface
    {
    public:
        DECLARE_META_INTERFACE(ConnectionManagerClient);

        virtual void onConnectionManagerStatusChange(int32_t status) = 0;

        virtual void onRatChange(int32_t currentRat, int32_t newRat) = 0;

        virtual void onConfigurationChange() = 0;
    };

    class BnConnectionManagerClient : public BnInterface<IConnectionManagerClient>
    {
    protected:
        Vector<sp<ConnectionManagerBinderListener> > mConnectionManagerListenersList;

    public:
        virtual status_t onTransact( uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);

		virtual void onConnectionManagerStatusChange(int32_t status);

        virtual void onRatChange(int32_t currentRat, int32_t newRat);

        virtual void onConfigurationChange();
    };
}

#endif
