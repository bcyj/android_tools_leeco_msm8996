/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
*/
/*
 * Copyright (C) 2013 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

package com.android.setupwizard.util;

import com.android.setupwizard.R;

import android.app.Fragment;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;

import static com.android.internal.telephony.PhoneConstants.MAX_PHONE_COUNT_TRI_SIM;

public class SetupUtils {

    public static final String ACTION_SETUP_WIFI = "com.android.net.wifi.SETUP_WIFI_NETWORK";

    public static final String EXTRA_FIRST_RUN = "firstRun";
    public static final String EXTRA_ALLOW_SKIP = "allowSkip";
    public static final String EXTRA_AUTO_FINISH = "wifi_auto_finish_on_connect";
    public static final String EXTRA_SHOW_BUTTON_BAR = "extra_prefs_show_button_bar";
    public static final String EXTRA_PREF_BACK_TEXT = "extra_prefs_set_back_text";
    public static final String EXTRA_ONLY_ACCESS_POINTS = "only_access_points";
    public static final String EXTRA_THEME = "theme";
    public static final String THEME_HOLO = "holo";
    
    public static final int REQUEST_CODE_SETUP_WIFI = 0;
    
    public static void tryEnablingWifi(Context context) {
        WifiManager wifiManager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
        if (!wifiManager.isWifiEnabled()) {
            wifiManager.setWifiEnabled(true);
        }
    }

    public static void tryDisenablingWifi(Context context) {
        WifiManager wifiManager = (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
        if (wifiManager.isWifiEnabled()) {
            wifiManager.setWifiEnabled(false);
        }
    }

    private static Intent getWifiSetupIntent(Context context) {
        Intent intent = new Intent(ACTION_SETUP_WIFI);
        intent.putExtra(EXTRA_FIRST_RUN, true);
        intent.putExtra(EXTRA_ALLOW_SKIP, true);
        intent.putExtra(EXTRA_SHOW_BUTTON_BAR, true);
        intent.putExtra(EXTRA_ONLY_ACCESS_POINTS, true);
        intent.putExtra(EXTRA_AUTO_FINISH, true);
        intent.putExtra(EXTRA_PREF_BACK_TEXT, context.getString(R.string.back));
        intent.putExtra(EXTRA_THEME, THEME_HOLO);
        return intent;
    }

    public static void launchWifiSetup(Fragment fragment) {
        final Context context = fragment.getActivity();
        SetupUtils.tryEnablingWifi(context);
        Intent intent = getWifiSetupIntent(context);
        fragment.startActivityForResult(intent, REQUEST_CODE_SETUP_WIFI);
    }
    
    public static boolean isWifiConnected(Context context) {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo mWifi = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        return mWifi != null && mWifi.isConnected();
    }
    
    public static boolean isMobileDataConnected(Context context) {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo mMobile = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        return mMobile != null && mMobile.isConnected();
    }
    
    public static boolean isSimReady() {
        TelephonyManager multiSimManager = TelephonyManager.getDefault();
        if (multiSimManager.isMultiSimEnabled()) {
            int numPhones = multiSimManager.getPhoneCount();
            for (int i = 0; i < numPhones; i++) {
                // Because the status of slot1/2 will return
                // SIM_STATE_UNKNOWN under airplane mode.
                if (multiSimManager.getSimState(i) == TelephonyManager.SIM_STATE_READY) {
                    return true;
                }
            }
        }
        return false;
    }
    
    public static void tryEnablingMobileData(Context context) {
        TelephonyManager mTelephonyManager = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);
        mTelephonyManager.setDataEnabled(true);
        for (int i = 0; i < MAX_PHONE_COUNT_TRI_SIM; i++) {
            Settings.Global.putInt(context.getContentResolver(),
                    Settings.Global.MOBILE_DATA + i, 1 );
        }
    }
    
    public static void tryDisEnablingMobileData(Context context) {
        TelephonyManager mTelephonyManager = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);
        mTelephonyManager.setDataEnabled(false);
        for (int i = 0; i < MAX_PHONE_COUNT_TRI_SIM; i++) {
            Settings.Global.putInt(context.getContentResolver(),
                    Settings.Global.MOBILE_DATA + i, 0 );
        }
    }
}
