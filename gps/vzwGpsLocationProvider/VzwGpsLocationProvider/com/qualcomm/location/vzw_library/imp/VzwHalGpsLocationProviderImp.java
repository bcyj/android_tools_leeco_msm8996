/******************************************************************************
  @file    VzwHalGpsLocationProviderImp.java
  @brief   implementation of the VZW GPS Location Provider

  DESCRIPTION

  this class implements the  VZW GPS Location Provider

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.vzw_library.imp;

import java.lang.ArrayIndexOutOfBoundsException;
import java.net.InetSocketAddress;

import android.util.Log;

import com.qualcomm.location.vzw_library.IVzwHalGpsCallback;
import com.qualcomm.location.vzw_library.IVzwHalGpsLocationProvider;
import com.qualcomm.location.vzw_library.VzwHalCriteria;
import com.qualcomm.location.vzw_library.VzwHalLocation;
import com.qualcomm.location.vzw_library.VzwHalSvInfo;

public class VzwHalGpsLocationProviderImp extends IVzwHalGpsLocationProvider {
    // note Log has a limit of 23 characters!
    private static final String TAG = "VzwHalGpsLocProvider";

    private boolean DEBUG = false;
    private boolean VERBOSE = false;

    public final static int AGPS_SERVER_ADDR_TYPE_SUPL = 1;
    public final static int AGPS_SERVER_ADDR_TYPE_PDE = 2;
    public final static int AGPS_SERVER_ADDR_TYPE_MPC = 3;

    private static enum FixModeDecisionState {
        Dormant, Standalone, MSB, MSA
    };

    private static enum ResetCategories {
        Ephemeris(0x4001), Location(4), Almanac(0x8002), Time(0x180008);
        private final int mMask;
        ResetCategories(int mask) {
            mMask = mask;
        }
    };

    private FixModeDecisionState mModeDecisionState = FixModeDecisionState.Dormant;

    private boolean mSessionIdValid = false;
    private int mCurrentSessionId;
    private IVzwHalGpsCallback mCallback;
    private InetSocketAddress mPdeAddr;
    private InetSocketAddress mLocSrvAddr;

    private ILocationEngine mGpsEngine;

    private boolean mIsInitDone = false;

    private String mCredentials;
    private ResetCategories mResetCategory;

    // note: this is the same file as the normal Android GPS stack. only the items are independent
    //private static final String PROPERTIES_FILE = "/etc/gps.conf";
    // properties loaded from PROPERTIES_FILE
    //private Properties mProperties = new Properties();;

    public boolean isSupported() {
        return true;
    }

    public VzwHalGpsLocationProviderImp () {
        DEBUG = Log.isLoggable(TAG, Log.DEBUG);
        VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);
    }

    private synchronized void modeAdjustmentStateMachine_Dormant (VzwHalCriteria criteria) {
        switch(criteria.getFixMode ())
        {
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE:
                // no change to criteria
                // move to Standalone state
                mModeDecisionState = FixModeDecisionState.Standalone;
                if(DEBUG) Log.d (TAG, "move to Standalone state");
                break;

            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED:
                // no change to criteria
                // move to MSB state
                mModeDecisionState = FixModeDecisionState.MSB;
                if(DEBUG) Log.d (TAG, "move to MSB state");
                break;

            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED:
                // no change to criteria
                // move to MSA state
                mModeDecisionState = FixModeDecisionState.MSA;
                if(DEBUG) Log.d (TAG, "move to MSA state");
                break;

            // lumpped processing of modes not specified by vzw spec
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_AFLT:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_SPEED:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_ACCURACY:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_DATA:
                // no change to criteria
                // move to MSB state
                mModeDecisionState = FixModeDecisionState.MSB;
                if(DEBUG) Log.d (TAG, "move to MSB state for request of AFLT or Optimal mode");
                break;

            default:
                throw new IllegalArgumentException("Mode not recognized");
        }
    }

    private synchronized void modeAdjustmentStateMachine_Standalone (VzwHalCriteria criteria) {
        switch(criteria.getFixMode ())
        {
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE:
                // no change to criteria
                // stay in Standalone state
                break;

            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED:
                // no change to criteria
                // move to MSB state
                mModeDecisionState = FixModeDecisionState.MSB;
                if(DEBUG) Log.d (TAG, "move to MSB state from MSS");
                break;

            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED:
                // change to Standalone mode
                criteria.setFixMode (IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE);
                if(DEBUG) Log.d (TAG, "override with Standalone mode");
                // stay in Standalone state
                break;

            // lumpped processing of modes not specified by vzw spec
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_AFLT:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_SPEED:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_ACCURACY:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_DATA:
                // no change to criteria
                // move to MSB state
                mModeDecisionState = FixModeDecisionState.MSB;
                if(DEBUG) Log.d (TAG, "move to MSB state from MSS for request of AFLT or Optimal mode");
                break;

            default:
                throw new IllegalArgumentException("Mode not recognized");
        }
    }

    private synchronized void modeAdjustmentStateMachine_MSA (VzwHalCriteria criteria) {
        switch(criteria.getFixMode ())
        {
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE:
                // no change to criteria
                // move to Standalone state
                mModeDecisionState = FixModeDecisionState.Standalone;
                if(DEBUG) Log.d (TAG, "move to Standalone state from MSA");
                break;

            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED:
                // no change to criteria
                // move to MSB state
                mModeDecisionState = FixModeDecisionState.MSB;
                if(DEBUG) Log.d (TAG, "move to MSB state from MSA");
                break;

            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED:
                // no change to criteria
                // stay in MSA state
                break;

            // lumpped processing of modes not specified by vzw spec
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_AFLT:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_SPEED:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_ACCURACY:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_DATA:
                // no change to criteria
                // move to MSB state
                mModeDecisionState = FixModeDecisionState.MSB;
                if(DEBUG) Log.d (TAG, "move to MSB state from MSA for request of AFLT or Optimal mode");
                break;

            default:
                throw new IllegalArgumentException("Mode not recognized");
        }
    }

    private synchronized void modeAdjustmentStateMachine_MSB (VzwHalCriteria criteria) {
        switch(criteria.getFixMode ())
        {
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE:
                // change to MSB mode
                criteria.setFixMode (IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED);
                if(DEBUG) Log.d (TAG, "override with MSB mode");
                // stay in MSB state
                break;

            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED:
                // no change to criteria
                // stay in MSB state
                break;

            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_ASSISTED:
                criteria.setFixMode (IVzwHalGpsLocationProvider.GPS_POSITION_MODE_MS_BASED);
                if(DEBUG) Log.d (TAG, "override with MSB mode");
                // stay in MSB state
                break;

            // lumpped processing of modes not specified by vzw spec
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_AFLT:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_SPEED:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_ACCURACY:
            case IVzwHalGpsLocationProvider.GPS_POSITION_MODE_OPTIMAL_DATA:
                // no change to criteria
                // stay in MSB state
                break;

            default:
                throw new IllegalArgumentException("Mode not recognized");
        }
    }

    public synchronized boolean start (VzwHalCriteria criteria, int sessionId) {

        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return false;
        }

        mCurrentSessionId = sessionId;
        mSessionIdValid = true;

        if ( null != mPdeAddr )
        {
            // since we are not sure if the engine will change mode automatically, it might be wise to set mode every time
            mGpsEngine.set_agps_server(AGPS_SERVER_ADDR_TYPE_PDE, mPdeAddr.getHostName(), mPdeAddr.getPort());
        }
        if ( null != mLocSrvAddr )
        {
            // since we are not sure if the engine will change mode automatically, it might be wise to set mode every time
            mGpsEngine.set_agps_server(AGPS_SERVER_ADDR_TYPE_SUPL, mLocSrvAddr.getHostName(), mLocSrvAddr.getPort());
        }

        switch(mModeDecisionState)
        {
            case Dormant:
            modeAdjustmentStateMachine_Dormant(criteria);
            break;

            case Standalone:
            modeAdjustmentStateMachine_Standalone(criteria);
            break;

            case MSA:
            modeAdjustmentStateMachine_MSA(criteria);
            break;

            case MSB:
            modeAdjustmentStateMachine_MSB(criteria);
            break;

            default:
            throw new IllegalStateException("Unknown mode decision state");
        };

        if(criteria.getHintNextFixArriveInSec() < 0)
        {
            // no hint, move back to dormant state
            if(DEBUG) Log.d (TAG, "move back to Dormant state for lack of hint");
            mModeDecisionState = FixModeDecisionState.Dormant;
        }

        if(VERBOSE) Log.v (TAG, "Request in mode: " + criteria.getFixMode());

        if(!mGpsEngine.start(criteria, sessionId, mCredentials))
        {
            Log.w (TAG, "engine start failed");
            return false;
        }
        return true;
    }

    public synchronized boolean stop () {
        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return false;
        }

        mSessionIdValid = false;

        // go back to dormant, so we get clean start after this
        mModeDecisionState = FixModeDecisionState.Dormant;

        if(!mGpsEngine.stop())
        {
            Log.w (TAG, "engine stop failed");
            return false;
        }
        return true;
    }

    public InetSocketAddress getPdeAddress() {
        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return null;
        }

        return mPdeAddr;
    }

    public void setPdeAddress(InetSocketAddress address) {
        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return;
        }

        mPdeAddr = address;
        if(VERBOSE && null == mPdeAddr) {
            Log.v (TAG, "PDE server nullified");
        }
    }

    public InetSocketAddress getLocSrvAddress() {
        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return null;
        }

        return mLocSrvAddr;
    }

    public void setLocSrvAddress(InetSocketAddress address) {
        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return;
        }

        mLocSrvAddr = address;
        if(VERBOSE && null == mLocSrvAddr) {
            Log.v (TAG, "SUPL server nullified");
        }
    }

    public synchronized void shutdown() {
        mGpsEngine.stop();
        mGpsEngine.cleanup();
        mGpsEngine = null;
        mCallback = null;
        mIsInitDone = false;

        mPdeAddr = null;
        mLocSrvAddr = null;

        mSessionIdValid = false;
        mModeDecisionState = FixModeDecisionState.Dormant;

        Log.i(TAG,"engine shutdown completed");
    }

    public synchronized void init(IVzwHalGpsCallback callback) {
        mCallback = callback;

        //mGpsEngine = new EngineSimulator();
        mGpsEngine = NativeMethods.getInstance();

        mGpsEngine.setCallbackInterface(mCallbackFromEngine);
        mGpsEngine.init();

        mIsInitDone = true;
        mModeDecisionState = FixModeDecisionState.Dormant;

        DEBUG = Log.isLoggable(TAG, Log.DEBUG);
        VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

        Log.i(TAG,"engine init done");
    }

    public String getCredentials() {
        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return null;
        }
        return mCredentials;
    }

    public void setCredentials(String credentials) {
        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return;
        }
        mCredentials = credentials;
    }

    public int getGpsResetType() {
        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return -1;
        }
        int resetType = mResetCategory.ordinal();
        Log.i(TAG, "getGpsResetType - "+resetType);
        return resetType;
    }

    public Boolean setGpsReset(int resetType) {
        if(!mIsInitDone)
        {
            Log.e (TAG, "Engine is not initialized");
            return false;
        }
        Log.i(TAG, "setGpsResetType - "+resetType);

        try {
            mResetCategory = mResetCategory.values()[resetType];
            mGpsEngine.resetGps(mResetCategory.mMask);
        } catch (ArrayIndexOutOfBoundsException aioob) {
            aioob.printStackTrace();
            return false;
        }

        return true;
    }


    private synchronized IVzwHalGpsCallback getLocationCallbackHandler(int sessionId) {
        if(mSessionIdValid)
        {
            if(sessionId == mCurrentSessionId)
            {
                return mCallback;
            }
            else
            {
                Log.w (TAG, "not matching with active sessoin. drop fix report");
            }
        }
        else
        {
            Log.w (TAG, "no active sessoin. drop fix report");
        }
        return null;
    }

    private synchronized IVzwHalGpsCallback getGeneralCallbackHandler() {
        if(!mIsInitDone)
        {
            return null;
        }

        return mCallback;
    }

    private IVzwHalGpsCallback  mCallbackFromEngine = new IVzwHalGpsCallback() {

        public void ReportLocation(VzwHalLocation location) {
            if(VERBOSE) Log.v (TAG, "fix arrived " + location.getLatitude() + ", " + location.getLongitude());

            IVzwHalGpsCallback callback_copy = getLocationCallbackHandler(location.getSessionId());

            if(null != callback_copy)
            {
                callback_copy.ReportLocation(location);
            }
        }

        public void ReportGpsStatus(int statusCode) {
            IVzwHalGpsCallback callback_copy = getGeneralCallbackHandler();

            if(null != callback_copy)
            {
                callback_copy.ReportGpsStatus(statusCode);
            }
        }

        public void ReportEngineStatus(int statusCode) {
            switch(statusCode)
            {
            case ENGINE_STATUS_SESSION_BEGIN:
                if(VERBOSE) Log.v (TAG, "engine state: session begin");
                break;
            case ENGINE_STATUS_SESSION_END:
                if(VERBOSE) Log.v (TAG, "engine state: session end");
                break;
            case ENGINE_STATUS_ENGINE_ON:
                if(VERBOSE) Log.v (TAG, "engine state: engine on");
                break;
            case ENGINE_STATUS_ENGINE_OFF:
                if(VERBOSE) Log.v (TAG, "engine state: engine off");
                break;
            default:
                if(VERBOSE) Log.v (TAG, "engine state: unknown");
                break;
            }

            IVzwHalGpsCallback callback_copy = getGeneralCallbackHandler();

            if(null != callback_copy)
            {
                callback_copy.ReportEngineStatus(statusCode);
            }
        }

        public void ReportSvStatus(VzwHalSvInfo svSvInfo) {
            if(VERBOSE) Log.v (TAG, "SV info arrived: SV in view count: " + svSvInfo.getNumSatellitesInView());

            IVzwHalGpsCallback callback_copy = getGeneralCallbackHandler();

            if(null != callback_copy)
            {
                callback_copy.ReportSvStatus(svSvInfo);
            }
        }
    };

}
