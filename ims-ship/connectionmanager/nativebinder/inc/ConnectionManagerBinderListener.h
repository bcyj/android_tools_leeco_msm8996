/** @file ConnectionManagerBinderListener.h
*/

/*=======================================================================
Copyright (c)2014-2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=======================================================================*/

#ifndef QIMSCM_CONNECTIONMANAGERBINDERLISTENER_H
#define QIMSCM_CONNECTIONMANAGERBINDERLISTENER_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>

/** @cond
*/

using namespace android;

namespace IMSConnectionManager
{

    class ConnectionManagerBinderListener: virtual public RefBase
    {
    public:

/** @endcond */

/** @addtogroup conn_mgr_binder_listener
@{ */

        /*=====================================================================
          Function: onConnectionManagerStatusChange
        =====================================================================*/
        /** @xreflabel{hdr:onConnectionManagerStatusChange}
        Callback invoked when there is a change in the status of the IMS
        Connection Manager.

        If the Connection Manager native service crashes after a listener is
        added, it is indicated through QIMSCM_STATUS_SERVICE_DIED. In this
        case, the client must close the Connection Manager and start it again.

        @param[in] status New state of the connection manager. See #QIMSCM_STATUS.

        @return
        None.
        */
        virtual void onConnectionManagerStatusChange(int32_t status) = 0;

        /** @cond
        */

        /*=====================================================================
          Function: onRatChange
        =====================================================================*/
        /**
        Callback invoked when there is a change of the RAT.

        @param[in] currentRat Current RAT on which the device is camped.
        @param[in] newRat New RAT to which the device is moving.

        @return
        None.
        */
        virtual void onRatChange(int32_t currentRat, int32_t newRat) = 0;

        /** @endcond */

        /*=====================================================================
          Function: onConfigurationChange
        =====================================================================*/
        /**
        Callback invoked when the configuration is updated.

        This is invoked when UE parameters change where there is a registration
        status change or a configuration update through OMA-DM (Open Mobile
        Alliance - Device Management) or other configuration update procedures.
        Clients can then query the configuration through getConfiguration().

        @return
        None.
        */
        virtual void onConfigurationChange() = 0;
    };

/** @} */ /* end_addtogroup conn_mgr_binder_listener */

};

#endif
