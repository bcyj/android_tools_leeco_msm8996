/******************************************************************************
  @file    NativeMethods.java
  @brief   interface to the JNI library for VZW GPS Location Provider

  DESCRIPTION

  this class directly talks to the JNI library

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.location.vzw_library.imp;

import com.qualcomm.location.vzw_library.IVzwHalGpsCallback;
import com.qualcomm.location.vzw_library.VzwHalCriteria;
import com.qualcomm.location.vzw_library.VzwHalLocation;
import com.qualcomm.location.vzw_library.VzwHalSvInfo;
import com.qualcomm.location.vzw_library.IVzwHalGpsLocationProvider;

import android.util.Log;

public class NativeMethods implements ILocationEngine {

    private static final String TAG = "VzwHal.Native";

    private static NativeMethods mInstance;

    private boolean DEBUG = false;
    private boolean VERBOSE = false;

    // for GPS SV statistics
    private static final int MAX_SVS = 32;

    // index into mSvFlags
    private static final int INDEX_SV_VALID_MASK = 0;

    // index into mSvMasks
    private static final int EPHEMERIS_MASK = 0;
    private static final int ALMANAC_MASK = 1;
    private static final int USED_FOR_FIX_MASK = 2;

    // index into mSvDops
    private static final int INDEX_PDOP = 0;
    private static final int INDEX_HDOP = 1;
    private static final int INDEX_VDOP = 2;

    // Maximum age we would accept DOP and SV USED from previous SV report
    private static final long MAX_AGE_SV_REPORT_MSEC = 3000;

    // preallocated arrays, to avoid memory allocation in reportStatus()
    private int mSvFlags[] = new int[1];
    private int mSvs[] = new int[MAX_SVS];
    private float mSnrs[] = new float[MAX_SVS];
    private float mSvElevations[] = new float[MAX_SVS];
    private float mSvAzimuths[] = new float[MAX_SVS];
    private int mSvMasks[] = new int[3];
    private float mSvDops[] = new float[3];

    private int [] mSvInViewTrimed;
    private float [] mSvInViewSnrTrimed;
    private float [] mSvInViewElevationTrimed;
    private float [] mSvInViewAzimuthTrimed;


    private int lastRequestedFixMode = -1;

    long timeLastSvReport;

    private boolean mSessionIdIsValid = false;
    private int mCurrentSessionId;

    private class EventThread extends Thread {

        public boolean fgTerminate = false;

        @Override
        public void run() {
            super.run();

            if(VERBOSE) Log.v(TAG,"event thread running");

            try{
                while(!(isInterrupted() || fgTerminate) )
                {
                    // note: callbacks happen in this thread
                    native_wait_for_event();
                }
            }
            catch(Exception e) {
                e.printStackTrace();
            }
            if(VERBOSE) Log.v (TAG,"event thread terminated");
        }
    }

    static {

        Log.v(TAG,"loading native library");

        System.loadLibrary("loc_ext");

        Log.v(TAG,"jni class init");

        class_init_native();
    }

    private IVzwHalGpsCallback mCallback;
    private EventThread mEventThread;

    private NativeMethods () {
        DEBUG = Log.isLoggable(TAG, Log.DEBUG);
        VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);
    };

    public synchronized void setCallbackInterface (IVzwHalGpsCallback callback) {
        mCallback = callback;
    }

    public synchronized void resetGps(int bits) {
        native_reset_gps(bits);
    }

    public static ILocationEngine getInstance() {
        if(null == mInstance)
        {
            mInstance = new NativeMethods();
        }
        return mInstance;
    }

    protected void finalize () {
        cleanup();
    }

    public synchronized void cleanup() {

        if(null != mEventThread)
        {
            if(VERBOSE) Log.v (TAG,"killing event thread");
            mEventThread.fgTerminate = true;
            mEventThread.interrupt();
            mEventThread = null;
        }

        lastRequestedFixMode = -1;

        native_disable ();
        native_cleanup();

        mSessionIdIsValid = false;
    }

    public synchronized boolean init() {

        if(VERBOSE) Log.v(TAG,"init");

        mEventThread = new EventThread();
        mEventThread.start();
        mSessionIdIsValid = false;

        lastRequestedFixMode = -1;

        if(VERBOSE) Log.v(TAG,"native init");

        boolean result = false;
        try {
            result = native_init();
        }
        catch(RuntimeException e)
        {
            e.printStackTrace();
        }

        if(VERBOSE) Log.v(TAG,"native init done");

        return result;
    }

    public synchronized void set_agps_server(int type, String hostname, int port) {
        native_set_agps_server(type, hostname, port);
    }

    private boolean fixCriteria(VzwHalCriteria criteria) {

        return true;
    }

    public synchronized boolean start(VzwHalCriteria criteria, int sessionId, String credentials) {

        if(fixCriteria(criteria))
        {
            mSessionIdIsValid = true;
            mCurrentSessionId = sessionId;

            boolean result = false;

            int time_to_next_fix_sec = criteria.getHintNextFixArriveInSec();
            if(time_to_next_fix_sec < 0)
            {
                // LocMW does not expect negative value, and adjusts 0 to 1 internally
                // we may change LocMW's behavior to take negative value as indication of 'no hint'
                time_to_next_fix_sec = 1;
            }

            lastRequestedFixMode = criteria.getFixMode();

            result = native_start(lastRequestedFixMode,
                                  true,
                                  time_to_next_fix_sec,
                                  criteria.getMaximumResponseTime(),
                                  (IVzwHalGpsLocationProvider.GPS_POSITION_MODE_STANDALONE
                                   == lastRequestedFixMode ? null : credentials));

            if(!result)
            {
                Log.e(TAG,"cannot start the location engine");
            }

            return result;
        }
        else
        {
            Log.w(TAG,"criteria is foobar");
        }

        return false;
    }

    public synchronized boolean stop() {
        mSessionIdIsValid = false;
        return native_stop();
    }

    private synchronized boolean setSessionIdIfValid (VzwHalLocation location)
    {
        if( mSessionIdIsValid)
        {
            location.setSessionId(mCurrentSessionId);
            return true;
        }
        else
        {
            return false;
        }
    }

    private final static int VZW_GPS_LOCATION_HAS_LAT_LONG =            0x0001;
    private final static int VZW_GPS_LOCATION_HAS_ALTITUDE =            0x0002;
    private final static int VZW_GPS_LOCATION_HAS_SPEED =               0x0004;
    private final static int VZW_GPS_LOCATION_HAS_BEARING =             0x0008;
    private final static int VZW_GPS_LOCATION_HAS_CIRC_ACCURACY =       0x0010;
    private final static int VZW_GPS_LOCATION_HAS_ALTITUDE_SEA_LEVEL =  0x0100;
    private final static int VZW_GPS_LOCATION_HAS_ELLI_ACCURACY =       0x0200;
    private final static int VZW_GPS_LOCATION_HAS_VERT_ACCURACY =       0x0400;
    private final static int VZW_GPS_LOCATION_HAS_CONFIDENCE =          0x0800;
    private final static int VZW_GPS_LOCATION_HAS_TIMESTAMP =           0x1000;
    private final static int VZW_GPS_LOCATION_HAS_MAGNETIC_VARIATION =  0x2000;
    private final static int VZW_GPS_LOCATION_HAS_DOP                =  0x4000;

    private final static int  GPS_SV_STATUS_VALID_SV_LIST     = 0x00000001;
    private final static int  GPS_SV_STATUS_VALID_EPH_MASK    = 0x00000002;
    private final static int  GPS_SV_STATUS_VALID_ALM_MASK    = 0x00000004;
    private final static int  GPS_SV_STATUS_VALID_USE_MASK    = 0x00000008;
    private final static int  GPS_SV_STATUS_VALID_PDOP        = 0x00000010;
    private final static int  GPS_SV_STATUS_VALID_HDOP        = 0x00000020;
    private final static int  GPS_SV_STATUS_VALID_VDOP        = 0x00000040;


    @SuppressWarnings("unused")
    private void reportLocation(int flags, double latitude, double longitude, double altitude,
            double altitude_sea_level,
              float speed, float bearing, float accuracy_hor_circular,
              float accuracy_hor_ellipse_semi_major,
              float accuracy_hor_ellipse_semi_minor,
              float accuracy_hor_ellipse_angle,
              float accuracy_vertical,
              float confidence_horizontal,
              float magnetic_variation,
              float pdop,
              float hdop,
              float vdop,
              long timestamp) {

        if(VERBOSE) Log.v (TAG, "reportLocation-lat:" +latitude+"lon: "+longitude+"alt: "+altitude+"accuracy: "+accuracy_hor_circular);

        VzwHalLocation location = new VzwHalLocation();

        if(setSessionIdIfValid (location))
        {
            int mask = 0;
            if((flags & VZW_GPS_LOCATION_HAS_LAT_LONG) != 0)
            {
                location.setLatitude(latitude);
                location.setLongitude(longitude);
                mask |= (VzwHalLocation.GPS_VALID_LATITUDE | VzwHalLocation.GPS_VALID_LONGITUDE);
            }

            if((flags & VZW_GPS_LOCATION_HAS_ALTITUDE) != 0)
            {
                // standard location already have attributes for validity of these fields
                location.setAltitude(altitude);
            }

            if((flags & VZW_GPS_LOCATION_HAS_VERT_ACCURACY) != 0)
            {
                location.setVerticalAccuracy(accuracy_vertical);
                mask |= VzwHalLocation.GPS_VALID_VERTICAL_ACCURACY;
            }

            if((flags & VZW_GPS_LOCATION_HAS_ALTITUDE_SEA_LEVEL) != 0)
            {
                location.setAltitudeWrtSeaLevel(altitude_sea_level);
                mask |= VzwHalLocation.GPS_VALID_ALTITUDE_WRT_SEA_LEVEL;
            }

            if((flags & VZW_GPS_LOCATION_HAS_SPEED) != 0)
            {
                // standard location already have attributes for validity of these fields
                location.setSpeed(speed);
            }

            if((flags & VZW_GPS_LOCATION_HAS_BEARING) != 0)
            {
                // standard location already have attributes for validity of these fields
                location.setBearing(bearing);
            }

            if((flags & VZW_GPS_LOCATION_HAS_CIRC_ACCURACY) != 0)
            {
                // standard location already have attributes for validity of these fields
                location.setAccuracy(accuracy_hor_circular);
            }

            if((flags & VZW_GPS_LOCATION_HAS_ELLI_ACCURACY) != 0)
            {
                location.setMajorAxis(accuracy_hor_ellipse_semi_major);
                location.setMajorAxisAngle(accuracy_hor_ellipse_angle);
                location.setMinorAxis(accuracy_hor_ellipse_semi_minor);

                mask |= VzwHalLocation.GPS_VALID_ELLIPTICAL_ACCURACY;
            }

            if((flags & VZW_GPS_LOCATION_HAS_CONFIDENCE) != 0)
            {
                location.setHorizontalConfidence(confidence_horizontal);

                mask |= VzwHalLocation.GPS_VALID_HORIZONTAL_CONFIDENCE;
            }

            if((flags & VZW_GPS_LOCATION_HAS_TIMESTAMP) != 0)
            {
                // standard location assumes time is always valid
                location.setTime(timestamp);

                // it might be better to check for validity before you acctually call getTime on
                // the standard location interface
                mask |= VzwHalLocation.GPS_VALID_TIME;
            }

            long time_delta = System.currentTimeMillis() - timeLastSvReport;
            if((time_delta >= 0) && (time_delta <= MAX_AGE_SV_REPORT_MSEC))
            {
                // harvest SV information from 'previous' SV report
                // this is an design issue from LocMW which SV Used and DOP inforation
                // is only reported 'after' the location report
                // however, there is no gurantee when the SV report will come

                if((mSvFlags[INDEX_SV_VALID_MASK] & GPS_SV_STATUS_VALID_PDOP) != 0)
                {
                    mask |= VzwHalLocation.GPS_VALID_POSITION_DILUTION_OF_PRECISION;
                    location.setPositionDilutionOfPrecision(mSvDops[INDEX_PDOP]);
                }

                if((mSvFlags[INDEX_SV_VALID_MASK] & GPS_SV_STATUS_VALID_HDOP) != 0)
                {
                    mask |= VzwHalLocation.GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION;
                    location.setHorizontalDilutionOfPrecision(mSvDops[INDEX_HDOP]);
                }

                if((mSvFlags[INDEX_SV_VALID_MASK] & GPS_SV_STATUS_VALID_VDOP) != 0)
                {
                    mask |= VzwHalLocation.GPS_VALID_VERTICAL_DILUTION_OF_PRECISION;
                    location.setVerticalDilutionOfPrecision(mSvDops[INDEX_VDOP]);
                }

                if((mSvFlags[INDEX_SV_VALID_MASK] & GPS_SV_STATUS_VALID_USE_MASK) != 0)
                {
                    mask |= VzwHalLocation.GPS_VALID_SATELLITES_USED_PRNS;
                    location.setSatellitesUsedPRN(getPrnArray(mSvMasks[USED_FOR_FIX_MASK]));
                }

                if(mSvInViewTrimed != null) {
                    location.setSatellitesInViewPRNs(mSvInViewTrimed);
                    mask |= VzwHalLocation.GPS_VALID_SATELLITES_IN_VIEW_PRNS;
                }
                if(mSvInViewAzimuthTrimed != null) {
                    location.setSatellitesInViewAzimuth(mSvInViewAzimuthTrimed);
                    mask |= VzwHalLocation.GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH;
                }
                if(mSvInViewElevationTrimed != null) {
                    location.setSatellitesInViewElevation(mSvInViewElevationTrimed);
                    mask |= VzwHalLocation.GPS_VALID_SATELLITES_IN_VIEW_ELEVATION;
                }
                if(mSvInViewSnrTrimed != null) {
                    location.setSatellitesInViewSignalToNoiseRatio(mSvInViewSnrTrimed);
                    mask |= VzwHalLocation.GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO;
                }
            }
            else
            {
                Log.w (TAG, "no fresh SV report to populate SV info in location report");
            }


            if(lastRequestedFixMode >= 0) {
                location.setFixMode (lastRequestedFixMode);
                mask |= VzwHalLocation.GPS_VALID_FIX_MODE;
            }

            if((flags & VZW_GPS_LOCATION_HAS_MAGNETIC_VARIATION) != 0)
            {
                location.setMagneticVariation(magnetic_variation);
                mask |= VzwHalLocation.GPS_VALID_MAGNETIC_VARIATION;
            }

            if((flags & VZW_GPS_LOCATION_HAS_DOP) != 0)
            {
                mask |= VzwHalLocation.GPS_VALID_POSITION_DILUTION_OF_PRECISION;
                location.setPositionDilutionOfPrecision(pdop);
                mask |= VzwHalLocation.GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION;
                location.setHorizontalDilutionOfPrecision(hdop);
                mask |= VzwHalLocation.GPS_VALID_VERTICAL_DILUTION_OF_PRECISION;
                location.setVerticalDilutionOfPrecision(vdop);
            }

            location.setValidFieldMask(mask);
        }
        else
        {
            Log.w (TAG, "no active session. drop fix report");
        }

        if(null != location)
        {
            mCallback.ReportLocation(location);
        }
    }
    @SuppressWarnings("unused")
    private void reportStatus(int status) {
        if(VERBOSE) Log.v (TAG, "reportStatus-status:" +status);

        mCallback.ReportEngineStatus(status);
    }

    @SuppressWarnings("unused")
    private void reportVzwEvent(int status) {
        if(VERBOSE) Log.v (TAG, "reportVzwEvent:" +status);

        mCallback.ReportGpsStatus(status);
    }

     private int getNumOfSvs (int mask) {
         int count = 0;
         for(int i = 0; i < 32; ++i)
         {
             if((mask & 1) == 1)
             {
                 ++count;
             }

             // unsigned shift
             mask >>>= 1;
         }
         return count;
     }

     private int [] getPrnArray (int mask) {
         int[] array = new int[getNumOfSvs(mask)];
         int count = 0;
         for(int prn = 0; prn < 32; ++prn)
         {
             if((mask & 1) == 1)
             {
                 array[count] = prn+1;
                 ++count;
             }

             // unsigned shift
             mask >>>= 1;
         }
         return array;
     }

     private int countNumFirstNonZeroPrn (int [] svs) {
         for(int i = 0; i < svs.length; ++i)
         {
             if(svs[i] == 0)
             {
                 return i;
             }
         }
         return svs.length;
     }

     private int[] copyArrayPartial (int [] array, int length)
     {
         int [] result = new int[length];
         for(int i = 0; i < length; ++i)
         {
             result[i] = array[i];
         }
         return result;
     }

     private float[] copyArrayPartial (float [] array, int length)
     {
         float [] result = new float[length];
         for(int i = 0; i < length; ++i)
         {
             result[i] = array[i];
         }
         return result;
     }

    // the 'synchronized' pragma has been removed because that caused confliction
    // between the top-down and bottom-up call flow, which leads to deadlock.
    // there is no need for synchronization here because the data is strictly
    // used by this function and reportLocation and only, and all reportXXX
    // functions are serialized by EventThread
    @SuppressWarnings("unused")
    private void reportSvStatus() {

        timeLastSvReport = System.currentTimeMillis();

        VzwHalSvInfo svInfo = new VzwHalSvInfo();

        // flush contents in sv array
        int i;
        for(i = 0; i < mSvFlags.length; ++i) {
            mSvFlags[i] = 0;
        }
        for(i = 0; i < mSvs.length; ++i) {
            mSvs[i] = 0;
            mSnrs[i] = 0;
            mSvElevations[i] = 0;
            mSvAzimuths[i] = 0;
        }
        for(i = 0; i < mSvMasks.length; ++i) {
            mSvMasks[i] = 0;
        }
        for(i = 0; i < mSvDops.length; ++i) {
            mSvDops[i] = 0;
        }

        native_read_sv_status(mSvFlags, mSvs, mSnrs, mSvElevations, mSvAzimuths, mSvMasks, mSvDops);

        int mask_sv_with_ephemris = mSvMasks[EPHEMERIS_MASK];
        int mask_sv_with_almanac = mSvMasks[ALMANAC_MASK];
        //int mask_sv_used = mSvMasks[USED_FOR_FIX_MASK];

        int [] svs_with_ephemeris = getPrnArray(mSvMasks[EPHEMERIS_MASK]);
        int [] svs_with_almanac = getPrnArray(mSvMasks[ALMANAC_MASK]);
        //int [] svs_used = getPrnArray(mSvMasks[USED_FOR_FIX_MASK]);

        int numSvInView = countNumFirstNonZeroPrn(mSvs);
        mSvInViewTrimed = copyArrayPartial(mSvs, numSvInView);
        mSvInViewSnrTrimed = copyArrayPartial(mSnrs, numSvInView);
        mSvInViewElevationTrimed = copyArrayPartial(mSvElevations, numSvInView);
        mSvInViewAzimuthTrimed = copyArrayPartial(mSvAzimuths, numSvInView);

        svInfo.setNumSatellitesInView(mSvInViewTrimed.length);
        svInfo.setSatellitesInViewAzimuth(mSvInViewAzimuthTrimed);
        svInfo.setSatellitesInViewElevation(mSvInViewElevationTrimed);
        svInfo.setSatellitesInViewPRNs(mSvInViewTrimed);
        svInfo.setSatellitesInViewSignalToNoiseRatio(mSvInViewSnrTrimed);
        svInfo.setSatellitesWithEphemeris (svs_with_ephemeris);
        svInfo.setSatellitesWithAlmanac(svs_with_almanac);

        int mask =
            VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_COUNT
            | VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_PRNS
            | VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_ELEVATION
            | VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH
            | VzwHalSvInfo.GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO
            | VzwHalSvInfo.GPS_VALID_SATELLITES_WITH_EPHEMERIS
            | VzwHalSvInfo.GPS_VALID_SATELLITES_WITH_ALMANAC;
        svInfo.setValidFieldMask(mask);

        mCallback.ReportSvStatus(svInfo);
    }

    private static native void class_init_native ();
    private native boolean native_init();
    private native void native_disable();
    private native void native_cleanup();
    private native boolean native_start(int positionMode, boolean isSingleShot, int fixInterval, int timeOut_sec, String app);
    private native boolean native_stop();
    private native void native_wait_for_event();
    private native int native_read_sv_status(int[] flags, int[] svs, float[] snrs,
          float[] elevations, float[] azimuths, int[] masks, float[] dops);
    private native void native_set_agps_server(int type, String hostname, int port);
    private native void native_reset_gps(int bits);
}

