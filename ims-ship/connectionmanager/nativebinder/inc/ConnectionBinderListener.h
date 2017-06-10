/** @file ConnectionBinderListener.h
*/

/*=======================================================================
Copyright (c)2014-2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=======================================================================*/

#ifndef QIMSCM_CONNECTIONBINDERLISTENER_H
#define QIMSCM_CONNECTIONBINDERLISTENER_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>

/** @cond
*/

using namespace android;

namespace IMSConnectionManager
{

    class ConnectionBinderListener: virtual public RefBase
    {
        public:

/** @endcond */

/** @addtogroup conn_binder_listener
@{ */

            /*=====================================================================
              Function: handleConnectionEvent
            =====================================================================*/
            /** @xreflabel{hdr:handleConnectionEvent}
            Callback function to inform clients about registration status
            change events.

            Informs of changes in service allowed by the policy manager due to
            RAT changes and any forceful terminations of the connection object
            by the QC framework due to PDP status changes.

            @param[in] event Connection event based on #QIMSCONNECTION_EVENT.

            @return
            None.
            */
            virtual void handleConnectionEvent(int32_t event) = 0;

            /*=====================================================================
              Function: handleIncomingMessage
            =====================================================================*/
            /** @xreflabel{hdr:handleIncomingMessage}
            Callback function to indicate the incoming message to the client.

            @param[in] messageContent Message content.
            @param[in] contentlength Message content length.

            @return
            None.
            */
            virtual void handleIncomingMessage(char* messageContent, int32_t contentlength) = 0;

            /*=====================================================================
              Function: handleCommandStatus
            =====================================================================*/
            /** @xreflabel{hdr:handleCommandStatus}
            Status of the sendMessage function (whether or not the message was
            transmitted to the network).

            The status is returned asynchronously via the handleCommandStatus()
            callback with messageId as a parameter. The client takes
            responsibility for mapping messages across different sessions
            based on the SIP call ID.

            @param[in] status Status of the sent message; mapped to
                              #QIMSCM_STATUSCODE.
            @param[in] messageId Message ID value passed in the sendMessage()
                                 function for which the send status is
                                 returning asynchronously.

            @return
            None.
            */
			virtual void handleCommandStatus(uint32_t status, uint32_t messageId) = 0;
    };

/** @} */ /* end_addtogroup conn_binder_listener */

};

#endif
