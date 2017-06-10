/******************************************************************************
 * @file    PPService.java
 * @brief   Sets the HSIC values on bootup
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qualcomm.display;

import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.util.Log;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.preference.SeekBarPreference;
import android.preference.CheckBoxPreference;

import com.android.display.IPPService;
import com.android.display.IPPService.Stub;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

public class PPService extends Service {

    private static final String TAG = "PPService";
    public static final String INTENT_PP_BOOT =
        "com.qualcomm.display.intent.action.BOOT_COMPLETED";
    private SharedPreferences mPPPrefs;
    private PPDaemonConnector mConnector;

    private static final String OFF_VALUE = "0";
    private static final String ON_VALUE = "1";
    private static final String QDCMMobleAppOnProperty = "persist.tuning.qdcm";
    boolean mPPEnable;
    int mcurrent_hue_value;
    int mcurrent_saturation_value;
    int mcurrent_intensity_value;
    int mcurrent_contrast_value;

    private static boolean isPropertyOn(String property) {
        return ON_VALUE.equals(SystemProperties.get(property, OFF_VALUE));
    }

    private IPPService.Stub mBinder = new Stub() {
        public boolean startPP() {
            if (null != mConnector) {
                return mConnector.startPP();
            }
            return false;
        }

        public boolean stopPP() {
            if (null != mConnector) {
                return mConnector.stopPP();
            }
            return false;
        }

        public boolean updateHSIC(int h, int s, int i, int c) {
            if (null != mConnector && !isPropertyOn(QDCMMobleAppOnProperty)) {
                return mConnector.setPPhsic(h, s, i, c);
            }
            return false;
        }

        public boolean getPPStatus() {
            if (null != mConnector) {
                return mConnector.getPPStatus();
            }
            return false;
        }

    };

    @Override
    public IBinder onBind(Intent arg0) {
        if (null == mConnector || !mConnector.isConnected()) {
            mConnector = new PPDaemonConnector();
            Thread t = new Thread(mConnector, "PPDaemonConnector");
            if (t != null)
                t.start();
            Log.d(TAG, "bind service and start daemon connector");
        }
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        int connectCount = 0;
        SystemProperties.set("ctl.start", "ppd");
        while ((++connectCount <= PPPreference.MAX_CONNECT_RETRY) && (false ==
                "running".equals(SystemProperties.get("init.svc.ppd")))) {
            SystemClock.sleep(5);
        }
        if (connectCount > PPPreference.MAX_CONNECT_RETRY) {
            Log.e(TAG, "Daemon not running!");
            return 0;
        }
        mConnector = new PPDaemonConnector();
        mPPPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        PreferenceManager.setDefaultValues(this, R.xml.pppreferences, false);

        if (intent == null || mPPPrefs == null || mConnector == null) {
            Log.e(TAG, "Null object!");
            return 0;
        }

        Thread t = new Thread(mConnector, "PPDaemonConnector");
        if(t != null)
            t.start();
        int tryCount = 0;
        while (!mConnector.isConnected() &&
                (++tryCount < PPPreference.MAX_CONNECT_RETRY)) {
            SystemClock.sleep(5);
        }
        if (tryCount == PPPreference.MAX_CONNECT_RETRY) {
            Log.e(TAG, "Failed to establish connection with daemon!");
            return 0;
        }

        mPPEnable = mPPPrefs.getBoolean(PPPreference.KEY_PP_ENABLE,
                PPPreference.PP_DEFAULT_STATUS);
        mcurrent_hue_value = mPPPrefs.getInt(PPPreference.KEY_HUE,
                PPPreference.mDefaultHue);
        mcurrent_saturation_value = mPPPrefs.getInt(PPPreference.KEY_SATURATION,
                PPPreference.mDefaultSaturation);
        mcurrent_intensity_value = mPPPrefs.getInt(PPPreference.KEY_INTENSITY,
                PPPreference.mDefaultIntensity);
        mcurrent_contrast_value = mPPPrefs.getInt(PPPreference.KEY_CONTRAST,
                PPPreference.mDefaultContrast);

        String action = intent.getAction();
        if (null != action && action.equals(INTENT_PP_BOOT))
        {
            if(mPPEnable){
                boolean have_set_hsic_values = false;
                if(mConnector!=null){
                    if(mConnector.startPP()) {
                        if( mConnector.getPPStatus() ) {
                            if(!mConnector.setPPhsic(mcurrent_hue_value,mcurrent_saturation_value,
                                        mcurrent_intensity_value,mcurrent_contrast_value))
                            {
                                Log.e(TAG,"Couldnot set the HSIC values at boot time");
                            }else {
                                have_set_hsic_values = true;
                                Log.d(TAG,"Setting HSIC values as " + mcurrent_hue_value + " " +
                                        mcurrent_saturation_value + " " + mcurrent_intensity_value + " " +
                                        mcurrent_contrast_value);
                            }
                        }
                    }
                }

                if(!have_set_hsic_values){
                    /* Couldnot set the hsic values at boot-time.
                     * Update the preferences to default values */

                    mPPPrefs.edit().putBoolean(PPPreference.KEY_PP_ENABLE,
                            PPPreference.PP_DEFAULT_STATUS).commit();

                    mPPPrefs.edit().putInt(PPPreference.KEY_HUE,
                            PPPreference.mDefaultHue).commit();

                    mPPPrefs.edit().putInt(PPPreference.KEY_SATURATION,
                            PPPreference.mDefaultSaturation).commit();

                    mPPPrefs.edit().putInt(PPPreference.KEY_INTENSITY,
                            PPPreference.mDefaultIntensity).commit();

                    mPPPrefs.edit().putInt(PPPreference.KEY_CONTRAST,
                            PPPreference.mDefaultContrast).commit();
                }
            }



        }
        mConnector.stopListener();
        stopSelf();
        return 0;
    }
}
