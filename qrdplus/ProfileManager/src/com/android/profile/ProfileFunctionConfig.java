/*
 * Copyright (c) 2011-2012, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

import static com.android.profile.ProfileConst.LOG;

import java.util.HashMap;
import java.util.Map;

import org.xmlpull.v1.XmlPullParser;

import android.content.Context;
import android.util.Log;

public class ProfileFunctionConfig {

    public final static String SILENT = "Silent";
    public final static String VIBRATE = "Vibrate";
    public final static String RING_VOLUME = "RingVolume";
    public final static String RING_TONE = "Ringtone";
    public final static String DATA = "Data";
    public final static String WIFI = "Wifi";
    public final static String BLUETOOTH = "Bluetooth";
    public final static String GPS_LOCATION = "GpsLocation";
    public final static String NETWORK_LOCATION = "NetworkLocation";
    public final static String BRIGHTNESS = "Brightness";

    private final static String TAG_NAME = "ProfileFunction";
    private final static String ATTRIBUTE_NAME = "name";
    private final static String ATTRIBUTE_ENABLE = "enable";
    private final static String ENABLE = "true";
    private final static String DISABLE = "false";
    private final static String TAG = "ProfileFunctionConfig";
    private static Map<String, Boolean> mFunctionConfigMap = new HashMap<String, Boolean>();

    private static Context mContext = null;

    private static boolean allCtrlEnabled = true;

    private static boolean silentCtrlEnabled = true;
    private static boolean vibrateCtrlEnabled = true;
    private static boolean ringVolumeCtrlEnabled = true;
    private static boolean dataCtrlEnabled = true;
    private static boolean wifiCtrlEnabled = true;
    private static boolean bluetoothCtrlEnabled = true;
    private static boolean gpsLocationCtrlEnabled = true;
    private static boolean networkLocationCtrlEnabled = true;
    private static boolean brightnessCtrlEnabled = true;
    private static boolean ringtoneCtrlEnabled = true;

    public ProfileFunctionConfig(Context context) {

        mContext = context;
    }

    public void getConfigFromXml(int resId) {

        XmlPullParser mXmlPullParser = null;
        mXmlPullParser = mContext.getResources().getXml(resId);
        try {
            int mEventType = mXmlPullParser.getEventType();

            while (mEventType != XmlPullParser.END_DOCUMENT) {

                if (mEventType == XmlPullParser.START_TAG) {
                    String tagName = mXmlPullParser.getName();
                    if (TAG_NAME.equals(tagName)) {
                        String name = mXmlPullParser.getAttributeValue(null, ATTRIBUTE_NAME);
                        String enable = mXmlPullParser.getAttributeValue(null, ATTRIBUTE_ENABLE);
                        logd("1=" + name);
                        if (DISABLE.equals(enable))
                            mFunctionConfigMap.put(name, new Boolean(false));
                        else if (ENABLE.equals(enable))
                            mFunctionConfigMap.put(name, new Boolean(true));
                        logd(mFunctionConfigMap.get(name));
                    }
                }
                mEventType = mXmlPullParser.next();
            }
        } catch (Exception e) {
            loge(e);
        }
        setAllConfig();
    }

    public static Map<String, Boolean> getProfileFunctionMap() {

        logd(mFunctionConfigMap.get("Wifi"));
        return mFunctionConfigMap;
    }

    public static boolean isAllCtrlEnabled() {

        return allCtrlEnabled;
    }

    public static boolean isSilentCtrlEnabled() {

        return silentCtrlEnabled;
    }

    public static void setSilentCtrlEnabled(boolean silentCtrlEnabled) {

        ProfileFunctionConfig.silentCtrlEnabled = silentCtrlEnabled;
    }

    public static boolean isVibrateCtrlEnabled() {

        return vibrateCtrlEnabled;
    }

    public static void setVibrateCtrlEnabled(boolean vibrateCtrlEnabled) {

        ProfileFunctionConfig.vibrateCtrlEnabled = vibrateCtrlEnabled;
    }

    public static boolean isRingVolumeCtrlEnabled() {

        return ringVolumeCtrlEnabled;
    }

    public static void setRingVolumeCtrlEnabled(boolean ringVolumeCtrlEnabled) {

        ProfileFunctionConfig.ringVolumeCtrlEnabled = ringVolumeCtrlEnabled;
    }

    public static boolean isDataCtrlEnabled() {

        return dataCtrlEnabled;
    }

    public static void setDataCtrlEnabled(boolean dataCtrlEnabled) {

        ProfileFunctionConfig.dataCtrlEnabled = dataCtrlEnabled;
    }

    public static boolean isWifiCtrlEnabled() {

        return wifiCtrlEnabled;
    }

    public static void setWifiCtrlEnabled(boolean wifiCtrlEnabled) {

        ProfileFunctionConfig.wifiCtrlEnabled = wifiCtrlEnabled;
    }

    public static boolean isBluetoothCtrlEnabled() {

        return bluetoothCtrlEnabled;
    }

    public static void setBluetoothCtrlEnabled(boolean bluetoothCtrlEnabled) {

        ProfileFunctionConfig.bluetoothCtrlEnabled = bluetoothCtrlEnabled;
    }

    public static boolean isGpsLocationCtrlEnabled() {

        return gpsLocationCtrlEnabled;
    }

    public static void setGpsLocationCtrlEnabled(boolean gpsLocationCtrlEnabled) {

        ProfileFunctionConfig.gpsLocationCtrlEnabled = gpsLocationCtrlEnabled;
    }

    public static boolean isNetworkLocationCtrlEnabled() {

        return networkLocationCtrlEnabled;
    }

    public static void setNetworkLocationCtrlEnabled(boolean networkLocationCtrlEnabled) {

        ProfileFunctionConfig.networkLocationCtrlEnabled = networkLocationCtrlEnabled;
    }

    public static boolean isBrightnessCtrlEnabled() {

        return brightnessCtrlEnabled;
    }

    public static void setBrightnessCtrlEnabled(boolean brightnessCtrlEnabled) {

        ProfileFunctionConfig.brightnessCtrlEnabled = brightnessCtrlEnabled;
    }

    public static boolean isRingtoneCtrlEnabled() {

        return ringtoneCtrlEnabled;
    }

    public static void setRingtoneCtrlEnabled(boolean ringtoneCtrlEnabled) {

        ProfileFunctionConfig.ringtoneCtrlEnabled = ringtoneCtrlEnabled;
    }

    private void setAllConfig() {

        allCtrlEnabled = true;
        Boolean disable = new Boolean(false);
        if (disable.equals(mFunctionConfigMap.get(SILENT))) {
            setSilentCtrlEnabled(false);
            allCtrlEnabled = false;
        }
        if (disable.equals(mFunctionConfigMap.get(VIBRATE))) {
            setVibrateCtrlEnabled(false);
            allCtrlEnabled = false;
        }
        if (disable.equals(mFunctionConfigMap.get(RING_VOLUME))) {
            setRingVolumeCtrlEnabled(false);
            allCtrlEnabled = false;
        }
        if (disable.equals(mFunctionConfigMap.get(RING_TONE))) {
            setRingtoneCtrlEnabled(false);
            allCtrlEnabled = false;
        }
        if (disable.equals(mFunctionConfigMap.get(DATA))) {
            setDataCtrlEnabled(false);
            allCtrlEnabled = false;
        }
        if (disable.equals(mFunctionConfigMap.get(WIFI))) {
            setWifiCtrlEnabled(false);
            allCtrlEnabled = false;
        }
        if (disable.equals(mFunctionConfigMap.get(BLUETOOTH))) {
            setBluetoothCtrlEnabled(false);
            allCtrlEnabled = false;
        }
        if (disable.equals(mFunctionConfigMap.get(GPS_LOCATION))) {
            setGpsLocationCtrlEnabled(false);
            allCtrlEnabled = false;
        }
        if (disable.equals(mFunctionConfigMap.get(NETWORK_LOCATION))) {
            setNetworkLocationCtrlEnabled(false);
            allCtrlEnabled = false;
        }
        if (disable.equals(mFunctionConfigMap.get(BRIGHTNESS))) {
            setBrightnessCtrlEnabled(false);
            allCtrlEnabled = false;
        }
    }

    private static void loge(Object e) {

        if (!LOG)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }

    private static void logd(Object s) {

        if (!LOG)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

}
