/** @file ConnectionBinder.h
*/

/*=======================================================================
Copyright (c)2014-2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=======================================================================*/

#ifndef QIMSCM_CONNECTIONBINDER_H
#define QIMSCM_CONNECTIONBINDER_H

#include <utils/RefBase.h>
#include <utils/threads.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>

#include "IConnectionClient.h"
#include "IConnectionBinder.h"
#include "ConnectionBinderListener.h"
#include "ConnectionManagerBinder.h"

/** @cond
*/

using namespace android;

namespace IMSConnectionManager
{
    class IConnectionBinder;

    class ConnectionBinder: public BnConnectionClient
    {
    public:

/** @endcond */

/** @addtogroup conn_binder
@{ */

        /*=====================================================================
          Function: close
        =====================================================================*/
        /** @xreflabel{hdr:close}
        Closes the connection and triggers deregistration of the associated
        URI.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned.
        */
        status_t close();

        /*=====================================================================
          Function: addListener
        =====================================================================*/
        /**
        Adds listeners to listen to connection events.

        @param[in] listener ConnectionBinderListener pointer to where
                            the callbacks will be received.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned.
        */
        status_t addListener(const sp<ConnectionBinderListener>& listener);

        /*=====================================================================
          Function: removeListener
        =====================================================================*/
        /**
        Removes listeners that were added to listen to connection events.

        @param[in] listener ConnectionBinderListener pointer to the
                            listener to be removed.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned. @newpage
        */
        status_t removeListener(const sp<ConnectionBinderListener>& listener);

        /*=====================================================================
          Function: sendMessage
        =====================================================================*/
        /** @xreflabel{hdr:sendMessage}
        Sends a SIP message with a unique call ID.

        @param[in] outboundProxy Message destination. For SIP requests, an
                                 outbound proxy address can be specified; for
                                 SIP responses, a header address can be used.
        @param[in] callId Refers to the SIP request/response call ID value as
                          per SIP RFC 3261. Refer to @xhyperref{S1, [S1]}.
        @param[in] message Message content.
        @param[in] messageLen Length of the message content.
        @param[in] messageId For each send message, the client passes a unique
                             integer value as the message ID in the connection
                             scope.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned.
        */
        status_t sendMessage(char* outboundProxy, char* callId, char* message, size_t messageLen, uint32_t messageId);

        /*=====================================================================
          Function: closeTransaction
        =====================================================================*/
        /**
        Terminates a SIP transaction with particular call ID.

        @param[in] callId Call ID.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned.
        */
        status_t closeTransaction(char* callId);

        /*=====================================================================
          Function: closeAllTransactions
        =====================================================================*/
        /**
        Terminates all transactions being handled by the connection object.

        @return
        A value from #QIMSCM_STATUSCODE is returned. If the call is successful,
        QIMSCM_SUCCESS is returned, otherwise an appropriate error code is
        returned.
        */
        status_t closeAllTransactions();

        /**@cond
        */

    private:
		ConnectionBinder();
        virtual ~ConnectionBinder();

        ConnectionBinder(const ConnectionBinder&);
        ConnectionBinder& operator=(const ConnectionBinder&);

        sp<IConnectionBinder> mConnectionBinder;
		friend class ConnectionManagerBinder;

        Mutex mBinderLock;

        /** @endcond */
    };

/** @} */ /* end_addtogroup conn_binder */

};

#endif
