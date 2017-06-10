/*
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.GPS;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.location.Criteria;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.IBinder;
import android.util.Log;
import android.view.LayoutInflater;

import com.qti.factory.R;
import com.qti.factory.Utils;
import com.qti.factory.Values;
import com.qti.factory.Framework.MainApp;

public class GpsService extends Service {

    static String TAG = "GpsService";
    private Context mContext;

    private Location location;
    LayoutInflater mInflater = null;
    LocationManager mLocationManager = null;
    final int OUT_TIME = 120 * 1000;
    final int MIN_SAT_NUM = 1;
    int index = -1;
    boolean result = false;

    public int onStartCommand(Intent intent, int flags, int startId) {
		if(intent == null)
			return -1;
        index = intent.getIntExtra(Values.KEY_SERVICE_INDEX, -1);
        if (index < 0)
            return -1;
        
        init(getApplicationContext());
        startGPS();
        return super.onStartCommand(intent, flags, startId);
    };

    @Override
    public void onDestroy() {
        logd("");
        finish(false);
        super.onDestroy();
    }

    CountDownTimer mCountDownTimer = new CountDownTimer(OUT_TIME, 5000) {

        @Override
        public void onTick(long arg0) {
            logd("onTick from " + TAG);
        }

        @Override
        public void onFinish() {

            fail(getString(R.string.time_out));
        }
    };

    void startGPS() {

        Criteria criteria;
        criteria = new Criteria();
        criteria.setAccuracy(Criteria.ACCURACY_FINE);
        criteria.setAltitudeRequired(false);
        criteria.setBearingRequired(false);
        criteria.setCostAllowed(true);
        criteria.setPowerRequirement(Criteria.POWER_LOW);

        String provider = mLocationManager.getBestProvider(criteria, true);
        if (provider == null) {
            fail("Fail to get GPS Provider!");
        }
        mLocationManager.requestLocationUpdates(provider, 500, 0, mLocationListener);
        mLocationManager.addGpsStatusListener(gpsStatusListener);

        location = mLocationManager.getLastKnownLocation(provider);
    }

    LocationListener mLocationListener = new LocationListener() {

        public void onLocationChanged(Location location) {

            pass();
        }

        public void onProviderDisabled(String provider) {

        }

        public void onProviderEnabled(String provider) {

        }

        public void onStatusChanged(String provider, int status, Bundle extras) {

        }
    };

    private GpsStatus mGpsStatus;
    private Iterable<GpsSatellite> mSatellites;
    List<String> satelliteList = new ArrayList<String>();
    GpsStatus.Listener gpsStatusListener = new GpsStatus.Listener() {

        public void onGpsStatusChanged(int arg0) {
            
            switch (arg0) {
            case GpsStatus.GPS_EVENT_STARTED:
                mCountDownTimer.start();
                break;
            case GpsStatus.GPS_EVENT_STOPPED:
                break;
            case GpsStatus.GPS_EVENT_FIRST_FIX:
                pass();
                break;
            case GpsStatus.GPS_EVENT_SATELLITE_STATUS:
                logd("GPS_EVENT_SATELLITE_STATUS");
                mGpsStatus = mLocationManager.getGpsStatus(null);
                mSatellites = mGpsStatus.getSatellites();
                Iterator<GpsSatellite> iterator = mSatellites.iterator();
                int count = 0;
                satelliteList.clear();
                while (iterator.hasNext()) {
                    GpsSatellite gpsS = (GpsSatellite) iterator.next();
                    satelliteList.add(count++, "Prn" + gpsS.getPrn() + " Snr:" + gpsS.getSnr());
                    logd("Prn" + gpsS.getPrn() + " Snr:" + gpsS.getSnr());
                }
                if (count >= MIN_SAT_NUM)
                    pass();
                break;
            default:
                break;
            }

        }

    };

    void stopGPS() {

        try {
            mLocationManager.removeUpdates(mLocationListener);
            mLocationManager.removeGpsStatusListener(gpsStatusListener);
        } catch (Exception e) {
            loge(e);
        }
    }
    
    public void finish() {
        finish(true);
    }

    public void finish(boolean writeResult) {
        if (mCountDownTimer != null)
            mCountDownTimer.cancel();
        stopGPS();
        
        if (writeResult) {
            Map<String, String> item = (Map<String, String>) MainApp.getInstance().mItemList.get(index);
            if (result) {
                item.put("result", Utils.RESULT_PASS);
                Utils.saveStringValue(getApplicationContext(), item.get("title"), Utils.RESULT_PASS);
                Utils.writeCurMessage(TAG, Utils.RESULT_PASS);
            } else {
                item.put("result", Utils.RESULT_FAIL);
                Utils.saveStringValue(getApplicationContext(), item.get("title"), Utils.RESULT_FAIL);
                Utils.writeCurMessage(TAG, Utils.RESULT_FAIL);
            }
            sendBroadcast(new Intent(Values.BROADCAST_UPDATE_MAINVIEW));
        }
        
    };
    
    void init(Context context) {
        mContext = context;
        mLocationManager = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
        if (mLocationManager == null) {
            fail("Fail to get LOCATION_SERVICE!");

        }
    }

    void fail(Object msg) {
        loge(msg);
        result = false;
        finish();
    }

    void pass() {

        result = true;
        finish();
    }

    private void loge(Object e) {

        if (e == null)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }

    private void logd(Object s) {

        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }

}
