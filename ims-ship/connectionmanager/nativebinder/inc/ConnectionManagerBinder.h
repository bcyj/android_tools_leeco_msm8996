/** @file ConnectionManagerBinder.h
*/

/*=======================================================================
Copyright (c)2014-2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=======================================================================*/

#ifndef QIMSCM_CONNECTIONMANAGERBINDER_H
#define QIMSCM_CONNECTIONMANAGERBINDER_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <utils/threads.h>

#include "IConnectionManagerClient.h"
#include "IConnectionManagerService.h"
#include "ConnectionManagerBinderListener.h"
#include "ConnectionBinder.h"
#include "IConnectionBinder.h"
#include "IConnectionManagerBinder.h"

/** @cond
*/

using namespace android;

namespace IMSConnectionManager
{
    class IConnectionManagerService;
    class IConnectionManagerBinder;
    class ConnectionBinder;
	class IConnectionBinder;
	class IConnectionManagerClient;

    class ConnectionManagerBinder: public BnConnectionManagerClient, public IBinder::DeathRecipient
    {
    public:

/** @endcond */

/** @addtogroup conn_mgr_binder
@{ */

        /*=====================================================================
          Function: startService
        =====================================================================*/
        /**
        Starts the native Android service returning a binder for further
        interaction with the Connection Manager.

        @return
        Valid ConnectionManagerBinder pointer if the call is successful,
        otherwise a NULL is returned.
        */
        static sp<ConnectionManagerBinder> startService();

        /*=====================================================================
          Function: initialize
        =====================================================================*/
        /**
        Initializes the native Connection Manager object.

        The service initialization status is returned asynchronously via the
        onConnectionManagerStatusChange() function. Clients are expected to
        wait for a service initialization success status before calling any
        other functions.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned.
        */
        status_t initialize();

        /*=====================================================================
          Function: createConnection
        =====================================================================*/
        /** @xreflabel{hdr:createConnection}
        Creates a new native IMS connection object.

        There must be a corresponding connection object for each feature tag.
        The URI passed as part of this call is validated with the standard
        RCS 5.1 URIs, and the connection objects are created only for supported
        URIs. Registration for the URI is triggered after this API is invoked.

        <b>URI format for a single URI:</b>

        @indent FeatureTagName="FeatureTagValue"

        Example
    @code
    +g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session" @endcode

        <b>URI format for multiple URIs:</b>

        @indent FeatureTagName1="FeatureTagValue1"; FeatureTagName2="FeatureTagValue2"

        Example
    @code
    "+g.oma.sip-im";"+g.3gpp.cs-voice";"+g.3gpp.iari-ref="urn%3Aurn-7%3A3gpp-
    application.ims.iari.gsma-is"";"+g.3gpp.iari-ref="urn%3Aurn-7%3A3gpp-
    application.ims.iari.rcse.ft""" @endcode

        @note1hang FeatureTagName and FeatureTagValue are defined in the RCS 5.1
        Specification.


        @param[in] uri NULL-teminated URI associated with a particular
                       application.

        @return
        Valid ConnectionBinder pointer if the call is successful,
        otherwise NULL is returned.
        */
        sp<ConnectionBinder> createConnection(char* uri);

        /*=====================================================================
          Function: close
        =====================================================================*/
        /**
        Closes the native Connection Manager.

        Closing the Connection Manager forces the pending connection objects
        to be immediately deleted regardless of what state they are in. @vertspace{-4}

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned.
        */
        status_t close();

        /*=====================================================================
          Function: triggerRegistration
        =====================================================================*/
        /** @xreflabel{triggerRegistration()}
        Used to trigger registration.

        This must be done once all the connections are created, enabling
        registration triggering with all the required Feature Tags (FTs)
        at once. @vertspace{-4}

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned.
        */
		status_t triggerRegistration();

        /*=====================================================================
          Function: getConfiguration
        =====================================================================*/
        /** @xreflabel{hdr:getConfiguration}
        Queries the configuration structure consisting of various parameters in
        which a client may be interested to form the SIP messages.

        Some of these configuration parameters are populated once the UE is
        successfully registered, so clients should wait for successful
        registration of at least one of the URIs before calling this function.

        @param[in] configurationType Configuration type based on the
                                     #QIMSCM_CONFIG_TYPE_ENUM.
        @param[out] cmConfiguration Pointer to be filled based on the
                                    configurationType requested.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned. @newpage
        */
        status_t getConfiguration(uint32_t configurationType, union CMConfiguration* cmConfiguration);

        /*=====================================================================
          Function: addListener
        =====================================================================*/
        /**
        Adds listeners to listen to Connection Manager events.

        @param[in] listener Pointer to where the callbacks will be recieved.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned. \n
        If the service crashes after startService() and before a listener
        has been added, it is indicated through QIMSCM_SERVICE_DIED. In this
        case, the client must close the Connection Manager and start it again.
        */
        status_t addListener(const sp<ConnectionManagerBinderListener>& listener);

        /*=====================================================================
          Function: removeListener
        =====================================================================*/
        /**
        Removes listeners that were added to listen to Connection Manager
        events.

        @param[in] listener Pointer to the listener to be removed.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned.
        */
        status_t removeListener(const sp<ConnectionManagerBinderListener>& listener);

        /** @cond
        */

    private:

        ConnectionManagerBinder();
		~ConnectionManagerBinder();

        ConnectionManagerBinder(const ConnectionManagerBinder&);

        ConnectionManagerBinder& operator=(const ConnectionManagerBinder&);

        static const sp<IConnectionManagerService>& getConnectionManagerService();

        virtual void binderDied(const wp<IBinder> &who);

        static sp<ConnectionManagerBinder> mSingletonBinder;

        sp<IConnectionManagerBinder> mConnectionManagerBinder;

		bool serviceCrashed;

        static sp<IConnectionManagerService> mConnectionManagerService;
		//sp<IConnectionBinder> mConnectionBinder;

        //Vector<wp<ConnectionBinder> > mConnectionsList;

		Mutex mBinderLock;

        static Mutex mStaticBinderLock;

        /** @endcond */

    };

/** @} */ /* end_addtogroup conn_mgr_binder */

}

#endif
