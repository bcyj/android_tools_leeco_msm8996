/************************************************************************
Copyright (c)2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
************************************************************************/

#ifndef QIMSCM_ICONNECTIONBINDER_H
#define QIMSCM_ICONNECTIONBINDER_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

using namespace android;

namespace IMSConnectionManager
{
    class IConnectionBinder: public IInterface
    {
    public:
        DECLARE_META_INTERFACE(ConnectionBinder);

        virtual status_t sendMessage(char* outboundProxy, char* callId, char* message, size_t messageLen, uint32_t messageId) = 0;

        virtual status_t close() = 0;

        virtual status_t closeTransaction(char* callId) = 0;

        virtual status_t closeAllTransactions() = 0;

        virtual status_t addListener() = 0;
    };

    class BnConnectionBinder: public BnInterface<IConnectionBinder>
    {
    public:
        virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
    };
};

#endif
