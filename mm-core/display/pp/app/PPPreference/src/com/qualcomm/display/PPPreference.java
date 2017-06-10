/******************************************************************************
 * @file    PPPreference.java
 * @brief   Provides an interface to enable/disable/set the HSIC values.
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2012 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qualcomm.display;

import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;
import android.preference.SeekBarPreference;
import android.preference.CheckBoxPreference;

import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.os.SystemClock;

/*
   Hue          -> Valid from -180 to 180 degrees
   Saturation   -> Valid from -1.0 to 1.0
   Intensity    -> Valid from -255 to 255
   Contrast     -> Valid from -1.0 to 1.0
*/

public class PPPreference extends PreferenceActivity implements OnSharedPreferenceChangeListener {

    private static final String TAG = "PPPreference";

    /* To Hold the default values of checkbox and the HSIC values */
    public static final boolean PP_DEFAULT_STATUS = false;
    public static final int mDefaultHue = 180;
    public static final int mDefaultSaturation = 180;
    public static final int mDefaultIntensity = 255;
    public static final int mDefaultContrast = 180;

    private CheckBoxPreference mPPEnable;
    private SeekBarPreference mHueValue;
    private SeekBarPreference mSaturationValue;
    private SeekBarPreference mIntensityValue;
    private SeekBarPreference mContrastValue;

    /* Strings to identify the preferences in the preferences xml file*/
    public static final String KEY_PP_ENABLE = "pp_checkbox";
    public static final String KEY_HUE = "hue";
    public static final String KEY_SATURATION = "saturation";
    public static final String KEY_INTENSITY = "intensity";
    public static final String KEY_CONTRAST = "contrast";

    public static final int MAX_CONNECT_RETRY = 10;

    private PPDaemonConnector mConnector;
    private SharedPreferences mPPPrefs;

    @Override
        public void onCreate(Bundle savedInstanceState) {
            /** Called when the activity is first created. */

            super.onCreate(savedInstanceState);
            addPreferencesFromResource(R.xml.pppreferences);

            mPPPrefs = PreferenceManager.getDefaultSharedPreferences(this);
            PreferenceManager.setDefaultValues(this, R.xml.pppreferences, false);

            mPPEnable = (CheckBoxPreference)getPreferenceScreen().
                                           findPreference(KEY_PP_ENABLE);
            mHueValue = (SeekBarPreference)getPreferenceScreen().
                                           findPreference(KEY_HUE);
            mSaturationValue = (SeekBarPreference)getPreferenceScreen().
                                           findPreference(KEY_SATURATION);
            mIntensityValue = (SeekBarPreference)getPreferenceScreen().
                                           findPreference(KEY_INTENSITY);
            mContrastValue = (SeekBarPreference)getPreferenceScreen().
                                           findPreference(KEY_CONTRAST);

            mConnector = new PPDaemonConnector();
            if (mConnector == null) {
                Log.e(TAG, "Null object!");
            }

            Thread t = new Thread(mConnector, "DaemonConnector");
            if(t != null) {
                /*At this point, the UI thread will continue to
                 * run in the foreground.
                 */
                t.start();
            }
            else
                Log.e(TAG,"Couldnot create background thread " +
                        "to communicate with PPDaemon");

            int tryCount = 0;
            while (!mConnector.isConnected() &&
                    (++tryCount < MAX_CONNECT_RETRY)) {
                SystemClock.sleep(5);
                    }

            if (tryCount == MAX_CONNECT_RETRY) {
                Log.e(TAG, "Failed to establish connection with daemon!");
            }
        }

    @Override
        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
                String key) {

            if ( key.equals(KEY_PP_ENABLE) ) {
                //Update HSIC Values
                updateHSIC();
                //Update HSIC Titles
                updateHSICTitle(KEY_HUE,mHueValue);
                updateHSICTitle(KEY_SATURATION,mSaturationValue);
                updateHSICTitle(KEY_INTENSITY,mIntensityValue);
                updateHSICTitle(KEY_CONTRAST,mContrastValue);
                Log.d(TAG,"PP status is "+ mPPEnable.isChecked());
            } else if ( key.equals(KEY_HUE) ) {
                updateHSIC();
                updateHSICTitle(key,mHueValue);
                Log.d(TAG,"Updating Hue Value as " + mHueValue.getProgress());
            } else if ( key.equals(KEY_SATURATION) ) {
                updateHSIC();
                updateHSICTitle(key,mSaturationValue);
                Log.d(TAG,"Updating Saturation Value as " +
                        mSaturationValue.getProgress());
            } else if ( key.equals(KEY_INTENSITY) ) {
                updateHSIC();
                updateHSICTitle(key,mIntensityValue);
                Log.d(TAG,"Updating Intensity Value as " +
                        mIntensityValue.getProgress());
            } else if ( key.equals(KEY_CONTRAST) ) {
                updateHSIC();
                updateHSICTitle(key,mContrastValue);
                Log.d(TAG,"Updating Contrast Value as " +
                        mContrastValue.getProgress());
            } else {
                Log.e(TAG,"Invalid Preference " + key);
                return;
            }
        }

        /* A little hack to display HSIC values on screen by changing the
         * android:title of the seekBarPreference, there by avoiding
         * adding additional peferences in pppreferences.xml
         */
        public static void updateHSICTitle(String key,SeekBarPreference seekbarpreference){

            int progress = seekbarpreference.getProgress();
            double value;

            if(key.equals(KEY_HUE)){
                value = progress-180;
            } else if ( key.equals(KEY_SATURATION)){
                value = ((int)(((progress-180)*100)/180.0))/100.0;
            } else if ( key.equals(KEY_INTENSITY)){
                value = progress-255;
            } else if ( key.equals(KEY_CONTRAST)){
                value = ((int)(((progress-180)*100)/180.0))/100.0;
            } else {
                Log.e(TAG,"Invalid Preference " + key);
                return;
            }
            seekbarpreference.setTitle(Character.toUpperCase(key.charAt(0))+key.substring(1)+" :-  "+value);
        }

    @Override
        public void onResume() {
            super.onResume();
            PreferenceScreen prefScreen = getPreferenceScreen();
            if( null != prefScreen ) {
                SharedPreferences sharedPrefs =  prefScreen.getSharedPreferences();
                if( null != sharedPrefs ) {
                    sharedPrefs.registerOnSharedPreferenceChangeListener(this);
                    //Update HSIC Titles
                    String[] hsic_keys = {PPPreference.KEY_HUE,
                        PPPreference.KEY_SATURATION,PPPreference.KEY_INTENSITY,
                        PPPreference.KEY_CONTRAST};
                    for(String key : hsic_keys){
                        PPPreference.updateHSICTitle(key,
                                (SeekBarPreference)getPreferenceScreen().findPreference(key));
                    }

                }
            }
        }

    @Override
        public void onPause() {
            super.onPause();
            PreferenceScreen prefScreen = getPreferenceScreen();
            if( null != prefScreen ) {
                SharedPreferences sharedPrefs = prefScreen.getSharedPreferences();
                if( null != sharedPrefs ) {
                    sharedPrefs.unregisterOnSharedPreferenceChangeListener(this);
                }
            }
        }

    private void updateHSIC()
    {
        PreferenceScreen prefScreen = getPreferenceScreen();
        if( null != prefScreen ) {
            SharedPreferences sharedPrefs =  prefScreen.getSharedPreferences();
            if( null != sharedPrefs ) {

                if(mPPEnable.isChecked()){
                    if(mConnector!=null){
                        if(!mConnector.startPP()) {
                            Toast.makeText(this, "Failed to enable Post processing.",
                                    Toast.LENGTH_SHORT).show();
                            mPPPrefs.edit().putBoolean(KEY_PP_ENABLE, false).commit();
                            mPPEnable.setChecked(false);
                            return;
                        }
                        if( mConnector.getPPStatus() ){

                            Log.d(TAG,"Setting HSIC values as " + mHueValue.getProgress() +
                                    " " + mSaturationValue.getProgress() + " " +
                                    mIntensityValue.getProgress() + " " +
                                    mContrastValue.getProgress());

                            if(!mConnector.setPPhsic(mHueValue.getProgress(),
                                        mSaturationValue.getProgress(),
                                        mIntensityValue.getProgress(),
                                        mContrastValue.getProgress()))
                                Toast.makeText(this, "Failed to set HSIC values.",
                                        Toast.LENGTH_SHORT).show();
                        }
                    } else {
                        Toast.makeText(this,"Connector maynot be running",
                                Toast.LENGTH_SHORT).show();
                    }
                }

                else {
                    if(mConnector!=null){
                        if( !mConnector.getPPStatus() ){
                            /* Needed inorder to set Default HSIC Values */
                            if(!mConnector.startPP()) {
                                Toast.makeText(this, "Failed to disable Post processing.",
                                        Toast.LENGTH_SHORT).show();
                                mPPPrefs.edit().putBoolean(KEY_PP_ENABLE, true).commit();
                                mPPEnable.setChecked(true);
                                return;
                            }
                        }
                        if( mConnector.getPPStatus() ){
                            // Set the default HSIC values
                            Log.d(TAG,"Setting HSIC values as " + mDefaultHue + " " +
                                    mDefaultSaturation + " " + mDefaultIntensity + " " +
                                    mDefaultContrast);

                            mConnector.setPPhsic(mDefaultHue,mDefaultSaturation,
                                    mDefaultIntensity,mDefaultContrast);

                            /* Restore back the seekbars and update the preferences to
                             * default values */

                            mPPPrefs.edit().putInt(KEY_HUE, mDefaultHue).commit();
                            mHueValue.setProgress(mDefaultHue);

                            mPPPrefs.edit().putInt(KEY_SATURATION,
                                    mDefaultSaturation).commit();
                            mSaturationValue.setProgress(mDefaultSaturation);

                            mPPPrefs.edit().putInt(KEY_INTENSITY,
                                    mDefaultIntensity).commit();
                            mIntensityValue.setProgress(mDefaultIntensity);

                            mPPPrefs.edit().putInt(KEY_CONTRAST,
                                    mDefaultContrast).commit();
                            mContrastValue.setProgress(mDefaultContrast);

                            if(!mConnector.stopPP()) {
                                Toast.makeText(this, "Failed to disable Post processing.",
                                        Toast.LENGTH_SHORT).show();
                            }
                        }
                        else {
                            Log.e(TAG,"Couldnot set Default HSIC values");
                        }
                    } else {
                        Toast.makeText(this,"Connector maynot be running",Toast.LENGTH_SHORT).show();
                    }
                }
            }
        }

    }

}
