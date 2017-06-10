/************************************************************************
Copyright (c)2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
************************************************************************/

#ifndef QIMSCM_ICONNECTIONCLIENT_H
#define QIMSCM_ICONNECTIONCLIENT_H

#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include "ConnectionBinderListener.h"

using namespace android;

namespace IMSConnectionManager
{

    class IConnectionClient: public IInterface
    {
    public:
        DECLARE_META_INTERFACE(ConnectionClient);

        virtual void handleConnectionEvent(int32_t event) = 0;

        virtual void handleIncomingMessage(char* messageContent, int32_t contentlength) = 0;

		virtual void handleCommandStatus(uint32_t status, uint32_t messageId) = 0;
    };

    class BnConnectionClient: public BnInterface<IConnectionClient>
    {
    protected:
        Vector<sp<ConnectionBinderListener> > mConnectionListenersList;

    public:
        virtual status_t onTransact( uint32_t code,
                                     const Parcel& data,
                                     Parcel* reply,
                                     uint32_t flags = 0);

		virtual void handleConnectionEvent(int32_t event);

        virtual void handleIncomingMessage(char* messageContent, int32_t contentlength);

		virtual void handleCommandStatus(uint32_t status, uint32_t messageId);
    };
};

#endif
