/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.wifi.cmcc;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.wifi.WifiManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;
import static android.telephony.TelephonyManager.SIM_STATE_READY;

public class Utils {

    public static final String TAG = "CMCC_WIFI";
    private static final boolean DBG = true;
    // Intents
    public static final String ACTION_CMCC_SERVICE = "com.android.wifi.cmcc.CmccService";
    public static final String ACTION_TOAST_FIND_AP = "com.android.wifi.cmcc.WifiGsmDialog";
    public static final String EXTRA_NOTIFICATION_SSID = "ssid";
    public static final String EXTRA_NOTIFICATION_NETWORKID = "network_id";
    public static final String ACTION_TOAST_SIGNAL_WEAK =
            "com.android.wifi.cmcc.WifiSelectInSsidsDialog";
    public static final String ACTION_WIFI_AUTOCONNECT_TYPE_CHANGE =
            "wifi.autoconnect.type.change";

    // SharePreferences
    public static final String SHARE_PREFERENCE_FILE_NAME = "cmcc_wifi";
    public static final String KEY_WIFI_CONNECT = "isConnected";

    // SettingsProvider Strings
    public static final String WIFI_AUTO_CONNECT_TYPE = "wifi_auto_connect_type";
    public static final int AUTO_CONNECT_ENABLED = 0;
    public static final String WLAN_CELLULAR_HINT = "wlan_to_cellular_hint";
    public static final String DATA_TO_WIFI_CONNECT_TYPE = "cellular_to_wlan_type";
    public static final int DATA_WIFI_CONNECT_TYPE_AUTO = 0;
    public static final int DATA_WIFI_CONNECT_TYPE_MANUAL = 1;
    public static final int DATA_WIFI_CONNECT_TYPE_ASK = 2;
    public static final String DISCONNECT_FROM_NETWORK = "disconnect_from_network";
    public static final int NOT_MANUALLY_DISCONNECT_LAST_AVAILABLE_AP = 0;
    public static final String NOTIFY_USER_CONNECT_CMCC = "notify_user_when_connect_cmcc";
    public static final String CELLULAR_TO_WLAN_HINT = "cellular_to_wlan_hint";
    public static final int NOTIFY_USER = 0;
    public static final int DO_NOT_NOTIFY_USER = -1;
    public static final String AIRPLANE_WLAN_NEED_WARNING = "airplane_wlan_warning";
    // CMCC wlan feature default value
    public static final int CELLULAR_WLAN_DEFAULT_VALUE = AUTO_CONNECT_ENABLED;
    public static final int AUTO_CONNECT_DEFAULT_VALUE = DATA_WIFI_CONNECT_TYPE_AUTO;
    public static final int WLAN_CELLULAR_HINT_DEFAULT_VALUE = NOTIFY_USER;

    // CMCC SSIDs
    public static final String SSID_CMCC = "CMCC";

    public static boolean isAutoConnectMode(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                WIFI_AUTO_CONNECT_TYPE, AUTO_CONNECT_ENABLED) == AUTO_CONNECT_ENABLED;
    }

    public static boolean isWlan2CellularHintEnabled(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                WLAN_CELLULAR_HINT,
                NOTIFY_USER) == NOTIFY_USER;
    }

    public static boolean isAirplaneModeOn(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }

    public static boolean isSimCardReady() {
        int numPhones = TelephonyManager.getDefault().getPhoneCount();
        for (int index = 0; index < numPhones; index++) {
            if (TelephonyManager.getDefault().getSimState(index)
                == TelephonyManager.SIM_STATE_READY) {
                return true;
            }
        }
        return false;
    }

    public static void log(String msg) {
        if (DBG) {
            Log.d(TAG, msg);
        }
    }

    public static void autoConnectAp(Context context, int highestPriorityNetworkId) {
        log("AutoConnectAp()");
        WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        if (wifiManager != null) {
            WifiManager.ActionListener mConnectListener = new WifiManager.ActionListener() {
                public void onSuccess() {
                }

                public void onFailure(int reason) {
                    Log.i("TAG", "auto connect wifi ap is failure! ");
                }
            };
            wifiManager.connect(highestPriorityNetworkId, mConnectListener);
        }
    }

    public static void setMobileDataEnabled(Context context, boolean enable) {
        TelephonyManager tm = TelephonyManager.from(context);
        tm.setDataEnabled(enable);
    }

    public static String convertToQuotedString(String string) {
        return "\"" + string + "\"";
    }

    public static boolean isCellularWlanHintEnable(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                CELLULAR_TO_WLAN_HINT, NOTIFY_USER) == NOTIFY_USER;
    }

    public static boolean needNotifyInAirplaneMode(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                AIRPLANE_WLAN_NEED_WARNING, NOTIFY_USER) == NOTIFY_USER;
    }
}
