/******************************************************************************
  @file    IVzwHalGpsLocationProvider.java
  @brief   interface for VZW GPS Location Provider

  DESCRIPTION

  VZW GPS Location Provider: this interface is supposed to be the same for all
  implementations from diff silicon vendors

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.vzw_library;

import java.net.InetSocketAddress;

import com.qualcomm.location.vzw_library.imp.VzwHalGpsLocationProviderImp;

public abstract class IVzwHalGpsLocationProvider {

    public static final String VZW_GPS_LOCATION_PROVIDER_NAME = "vzw_gps";

    // used in VzHalCriteria
    public static final int GPS_POSITION_MODE_MS_ASSISTED =         0;
    public static final int GPS_POSITION_MODE_MS_BASED =            1;
    public static final int GPS_POSITION_MODE_STANDALONE =          2;
    public static final int GPS_POSITION_MODE_AFLT =                3;
    public static final int GPS_POSITION_MODE_OPTIMAL_SPEED =       4;
    public static final int GPS_POSITION_MODE_OPTIMAL_ACCURACY =    5;
    public static final int GPS_POSITION_MODE_OPTIMAL_DATA =        6;
    public static final int GPS_POSITION_MODE_CID =                 7;
    public static final int GPS_POSITION_MODE_ECID =                8;
    public static final int GPS_POSITION_MODE_WIFI_MSA =            9;
    public static final int GPS_POSITION_MODE_WIFI_MSB =           10;
    public static final int GPS_POSITION_MODE_HYBRID =             11;

     // GpsStatus
    public static final int GPS_EVENT_ESTABLISH_CONNECTION_FAILED =     0x00000001;
    public static final int GPS_EVENT_AGPS_AUTH_PASS =                  0x00000002;
    public static final int GPS_EVENT_AGPS_AUTH_FAIL =                  0x00000003;
    public static final int GPS_EVENT_AGPS_AUTH_PDE_NOT_REACHABLE =     0x00000004;
    public static final int GPS_EVENT_AGPS_AUTH_EXPIRED =               0x00000005;
    public static final int GPS_EVENT_AGPS_AUTH_DNS_FAIL =              0x00000006;

    public static final int GPS_EVENT_INIT_FAIL =                       0x00000007;
    public static final int GPS_EVENT_INIT_PASS =                       0x00000008;

    public static final int GPS_EVENT_LOCATION_AVAILABLE =              0x00000009;
    public static final int GPS_EVENT_FIRST_FIX =                       0x0000000A;
    public static final int GPS_EVENT_FIX_REQ_FAIL =                    0x0000000B;
    public static final int GPS_EVENT_FIX_REQUESTED =                   0x0000000C;

    public static final int GPS_EVENT_GENERAL_AGPS_FAILURE =            0x0000000D;
    public static final int GPS_EVENT_GENERAL_FAILURE =                 0x0000000E;

    public static final int GPS_EVENT_INIT_CONFIG_NOT_PROVIDED =        0x0000000F;
    public static final int GPS_EVENT_LOCATION_REQUEST_TIMEOUT =        0x00000010;

    public static final int GPS_EVENT_INIT_SATELLITE_STATUS =           0x00000011;
    public static final int GPS_EVENT_SECURITY_FAILED =                 0x00000012;
    public static final int GPS_EVENT_SET_FIX_MODE_FAIL =               0x00000013;
    public static final int GPS_EVENT_SET_FIX_RATE_FAIL =               0x00000014;
    public static final int GPS_EVENT_SET_GPS_PERFORMANCE_FAIL =        0x00000015;
    public static final int GPS_EVENT_SET_PDE_FAIL =                    0x00000016;

    public static final int GPS_EVENT_STARTED =                         0x00000017;
    public static final int GPS_EVENT_STOPPED =                         0x00000018;

    public static final int GPS_EVENT_DEVICE_STATUS =                   0x00000019;
    public static final int GPS_EVENT_INIT_IN_PROGRESS =                101;
    public static final int GPS_EVENT_TRACKING_SESSION_TIMEDOUT =       305;


    // these need to match GpsStatusValue defines in gps.h
    public static final int ENGINE_STATUS_NONE = 0;
    public static final int ENGINE_STATUS_SESSION_BEGIN = 1;
    public static final int ENGINE_STATUS_SESSION_END = 2;
    public static final int ENGINE_STATUS_ENGINE_ON = 3;
    public static final int ENGINE_STATUS_ENGINE_OFF = 4;

    public abstract boolean isSupported();

    public abstract boolean start (VzwHalCriteria criteria, int sessionId);

    public abstract boolean stop ();

    public abstract void setPdeAddress (InetSocketAddress address);

    public abstract InetSocketAddress getPdeAddress ();

    public abstract void shutdown();

    public abstract void init(IVzwHalGpsCallback callback);

    public abstract String getCredentials();
    public abstract void setCredentials(String appIdPasswd);

    public abstract InetSocketAddress getLocSrvAddress ();
    public abstract void setLocSrvAddress (InetSocketAddress address);

    public abstract int getGpsResetType();
    public abstract Boolean setGpsReset(int resetType);

    private static VzwHalGpsLocationProviderImp mImp = null;

    public static IVzwHalGpsLocationProvider getInstance () {
        if(mImp == null)
        {
            mImp = new VzwHalGpsLocationProviderImp();
        }
        return mImp;
    }
}
