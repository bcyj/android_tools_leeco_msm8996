/******************************************************************************
 * @file    ILocationEngine.java
 * @brief   Defines the interface a GPS location engine should provide
 *
 *  Copyright (C) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ******************************************************************************/

package com.qualcomm.location.vzw_library.imp;

import com.qualcomm.location.vzw_library.IVzwHalGpsCallback;
import com.qualcomm.location.vzw_library.VzwHalCriteria;

public interface ILocationEngine {
    /**
     * initialize the location engine
     * @return true if the initialization is successful
     */
    public boolean init();

    /**
     * this would be the last call to the location engine before the next call to init
     */
    public void cleanup();

    /**
     * @param criteria specifies criteria for this specific request
     * @param sessionId an integer which will be passed back in the location report.
     * it's recommended to specify an unique number for each request
     * @return true if the request is entered into the location engine
     */
    public boolean start(VzwHalCriteria criteria, int sessionId, String app);

    /**
     * @return true if the stop is successful
     */
    public boolean stop();

    /**
     * set which AGPS server to be used. the setting would be persistent across sessions, but not written into NV items
     *
     * @param type can be one of VzwHalGpsLocationProvider.AGPS_SERVER_ADDR_TYPE_SUPL,
     * VzwHalGpsLocationProvider.AGPS_SERVER_ADDR_TYPE_C2K, or VzwHalGpsLocationProvider.AGPS_SERVER_ADDR_TYPE_MPC
     * @param hostname host name for the AGPS server
     * @param port port number for the AGPS server
     */
    public void set_agps_server(int type, String hostname, int port);


    /**
     * set the reference to callback handler
     *
     * @param callback interface to the object instance which handles callbacks.
     */
    public void setCallbackInterface (IVzwHalGpsCallback callback);

    public void resetGps(int bits);
}
