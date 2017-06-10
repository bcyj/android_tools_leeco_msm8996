/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.wifi.cmcc;

import java.util.ArrayList;
import java.util.List;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;
import android.provider.Settings;
import com.android.wifi.cmcc.R;

/**
 * This class is the main controller for CMCC. Now it's responsible for the
 * following things: 1) Show the PresetNetworkInfo Activity to represent the
 * preset networks.
 */
public class CmccService extends Service {

    private static final String TAG = "CmccService";

    private static final int EVENT_CHECK_AND_SAVE_PRESET_NETWORK = 109;
    private static final String ALREADY = "already";
    private static final String NOT_YET = "not_yet";
    private static final String KEY_ALREADY_IMPORTED = "already_imported";
    private static final String KEY_ALREADY_SAVE_PRESET_CMCC_NETWORK =
            "already_save_preset_cmcc_network";
    private static final String ACTION_WIFI_AUTOCONNECT_TYPE_CHANGE =
            "wifi.autoconnect.type.change";

    private static final int CMCC_DEFAULT_EAP_METHOD = 0;

    private AutoConnectTypeObserver mAutoConnectTypeObserver;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        IntentFilter filter = new IntentFilter();
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        registerReceiver(mReceiver, filter);
        mAutoConnectTypeObserver = new AutoConnectTypeObserver(new Handler());
    }

    /**
     *  observer the auto connect type is changed.
     */
    private class AutoConnectTypeObserver extends ContentObserver {

        public AutoConnectTypeObserver(Handler handler) {
            super(handler);
            ContentResolver cr = getContentResolver();
            cr.registerContentObserver(Settings.System.getUriFor(
                    Utils.WIFI_AUTO_CONNECT_TYPE), false, this);
            cr.registerContentObserver(Settings.System.getUriFor(
                    Utils.DATA_TO_WIFI_CONNECT_TYPE), false, this);
        }

        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);
            // Dialog may pop up when long click home button
            CmccMainReceiver.resetCellularToWlanDialogControlParas();
        }
    }

    BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "receive action: " + action);
            if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
                if (intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                        WifiManager.WIFI_STATE_UNKNOWN) == WifiManager.WIFI_STATE_DISABLED) {
                    // Dialog may pop up when close wlan through system ui.
                    CmccMainReceiver.resetCellularToWlanDialogControlParas();
                }
            }
        }
    };

    private class CmccEventHandler extends Handler {
        @Override
        public void handleMessage(Message message) {
            switch (message.what) {
                case EVENT_CHECK_AND_SAVE_PRESET_NETWORK:
                    try {
                        checkAndSavePresetNetwork(getApplicationContext());
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    break;
                default:
                    break;
            }
        }
    }

    /**
     * Check if Wi-Fi needs to be enabled and start if needed This function is
     * used only at first time when Wi-Fi is enabled
     */
    public void checkAndSavePresetNetwork(Context context) {
        String flag = Settings.System.getString(context.getContentResolver(),
                KEY_ALREADY_SAVE_PRESET_CMCC_NETWORK);
        if (isNullString(flag)) {
            flag = NOT_YET;
        }

        Log.d(TAG, "checkAndSavePresetNetwork, "
                + KEY_ALREADY_SAVE_PRESET_CMCC_NETWORK + ": " + flag);
        if (ALREADY.equals(flag)) {
            return;
        }

        WifiManager wifiManager = (WifiManager) context
                .getSystemService(Context.WIFI_SERVICE);
        WifiManager.ActionListener mSaveListener = new WifiManager.ActionListener() {
            public void onSuccess() {
            }

            public void onFailure(int reason) {
                Log.d(TAG, "failed to save network, reason:" + reason);
            }
        };

        String ssids[] = new String[] {
                getResources().getString(R.string.prser_network_cmcc_edu),
                getResources().getString(R.string.prser_network_cmcc),
                getResources().getString(R.string.prser_network_cmcc_auto)
        };

        for (int i = 0; i < ssids.length; i++) {
            WifiConfiguration config = new WifiConfiguration();
            config.priority = i + 1;
            config.SSID = Utils.convertToQuotedString(ssids[i]);
            if (ssids[i].equals(getResources().getString(R.string.prser_network_cmcc_auto))) {
                config.allowedKeyManagement.set(KeyMgmt.WPA_EAP);
                config.enterpriseConfig.setEapMethod(CMCC_DEFAULT_EAP_METHOD);
            } else {
                config.allowedKeyManagement.set(KeyMgmt.NONE);
            }
            Log.d(TAG, "save network: " + config.SSID);
            wifiManager.save(config, mSaveListener);
            wifiManager.startScan();
        }
        Settings.System.putString(context.getContentResolver(),
                KEY_ALREADY_SAVE_PRESET_CMCC_NETWORK, ALREADY);
    }

    private boolean isNullString(String value) {
        if (value == null || value.length() == 0) {
            return true;
        }
        return false;
    }
}
