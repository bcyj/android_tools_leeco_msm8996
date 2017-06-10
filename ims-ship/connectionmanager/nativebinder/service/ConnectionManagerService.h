/************************************************************************
Copyright (c)2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
************************************************************************/

#ifndef QIMSCM_CONNECTIONMANAGERSERVICE_H
#define QIMSCM_CONNECTIONMANAGERSERVICE_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <utils/Vector.h>

#include <../inc/IConnectionManagerService.h>
#include <../inc/IConnectionManagerClient.h>
#include <../inc/IConnectionClient.h>
#include <../inc/IConnectionManagerBinder.h>
#include <../inc/IConnectionBinder.h>
#include <../inc/ConnectionManagerBase.h>

using namespace android;

namespace IMSConnectionManager
{
	class IConnectionManagerService;
    class IConnectionManagerClient;
    class IConnectionClient;
	class IConnectionManagerBinder;
	class IConnectionBinder;

    enum
    {
        HANDLE_CONNECTION_MANAGER_STATUS,
        HANDLE_RAT_CHANGE,
            HANDLE_CONFIGURATION_CHANGE,
    };

    enum
    {
        HANDLE_CONNECTION_EVENT,
        HANDLE_INCOMING_MESSAGE,
		    HANDLE_COMMAND_STATUS,
    };

    class ConnectionManagerService: public BnConnectionManagerService
    {
        class Client;

        class ConnectionClient;

    public:
        static void instantiate();

        virtual sp<IConnectionManagerBinder> startService(const sp<IConnectionManagerClient>& connectionManagerClient);

        static void notifyClientEvents(uint32_t callback, int callback_data_1, int callback_data_2, void* user);

        static void notifyConnectionClientEvents(uint32_t callback, int callback_data_1, char* callback_data_2, int callback_data_3, void* user);

    private:

        class Client : public BnConnectionManagerBinder, public IBinder::DeathRecipient
        {
        public:
            virtual int startService(const sp<IConnectionManagerClient>& client);

            virtual int initialize();

            virtual sp<IConnectionBinder> createConnection(sp<IConnectionClient> connectionClient, char* uri);

            virtual int close();

			virtual int triggerRegistration();

            virtual status_t getConfiguration(uint32_t configurationType, union CMConfiguration* cmConfiguration);

            virtual int addListener();

            void onConnectionManagerStatusChange(int32_t status);

            void onRatChange(int32_t currentRat, int32_t newRat);

            void onConfigurationChange();

			const sp<IConnectionManagerClient>&    getConnectionManagerClient() const { return mConnectionManagerClient; }

            //void removeConnectionClient(sp<ConnectionClient> connectionClient);

        private:
            friend class ConnectionManagerService;
			friend class ConnectionClient;

            Client(const sp<ConnectionManagerService>& connectionManagerService,
                   const sp<IConnectionManagerClient>& connectionManagerClient,
                   pid_t clientPid,
                   cmNativeHandle nativeHandle);

            Client();

            void removeConnectionClient(sp<ConnectionClient> connectionClient);

            virtual ~Client();

            virtual void binderDied(const wp<IBinder> &who);

            //status_t checkPid();

            cmNativeHandle mNativeHandle;
			pid_t		   mClientPid;

            sp<ConnectionManagerService> mConnectionManagerService;
            sp<IConnectionManagerClient> mConnectionManagerClient;

			Vector<sp<ConnectionClient> > mConnectionClientsList;
        };

        class ConnectionClient : public BnConnectionBinder
        {
        public:
            virtual int sendMessage(char* remoteUri, char* callId, char* message, size_t messageLen, uint32_t messageId);

            virtual int close();

            virtual int closeTransaction(char* callId);

            virtual int closeAllTransactions();

            virtual int addListener();

            void handleConnectionEvent(int32_t event);

            void handleIncomingMessage(char* messageContent, int32_t contentlength);

			void handleCommandStatus(uint32_t status, uint32_t messageId);

        private:
            friend class ConnectionManagerService;
            friend class Client;

            ConnectionClient(const sp<ConnectionManagerService>& connectionManagerService,
                            const sp<IConnectionClient>& connectionClient,
                            const sp<Client>& connectionManagerClient,
                            pid_t clientPid,
                            cmConnectionNativeHandle connectionNativeHandle);

            ConnectionClient();

            virtual ~ConnectionClient();

            cmConnectionNativeHandle mConnectionNativeHandle;
			pid_t					 mClientPid;

            sp<ConnectionManagerService> mConnectionManagerService;
            sp<Client>  mConnectionManagerClientParent;

            sp<IConnectionClient> mConnectionClient;

        };

        //virtual void binderDied(const wp<IBinder> &who);
        int removeClient(sp<Client> client);

		ConnectionManagerService();

		virtual ~ConnectionManagerService();

        Mutex mServiceLock;

        //wp<Client> mClient;
		Vector< sp<Client> > mClientsList;

		static sp<ConnectionManagerService> holdServicePointer;
    };
}

#endif
