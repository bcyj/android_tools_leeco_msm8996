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

import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.ScanResult;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.util.AsyncChannel;
import com.android.wifi.cmcc.R;

public class CmccMainReceiver extends BroadcastReceiver {

    private static final String ACTION_WIFI_SETTING = "android.settings.WIFI_SETTINGS";

    // SettingsProvider
    private static final String SAVE_CMCC_FEATURE_DEFAULT_VALUE = "cmcc_wifi_default_value";
    private static final int CMCC_DEFAULT_VALUE_NOT_SAVED = 0;
    private static final int CMCC_DEFAULT_VALUE_SAVED = 1;

    // Security that supported by QC from WifiConfiguration$KeyMgmt
    private static final String WEP_STRING = "WEP";
    private static final String PSK_STRING = "PSK";
    private static final String EAP_STRING = "EAP";
    private static final String WAPI_PSK_STRING = "WAPI-PSK";
    private static final String WAPI_CERT_STRING = "WAPI-CERT";
    private static final int SECURITY_NONE = 0;
    private static final int SECURITY_WEP = 1;
    private static final int SECURITY_PSK = 2;
    private static final int SECURITY_EAP = 3;
    private static final int SECURITY_WAPI_PSK = 4;
    private static final int SECURITY_WAPI_CERT = 5;

    // TODO: Assign below paras in order.
    private static final String WIFI_SETTINGS_CLASS_NAME =
            "com.android.settings.Settings$WifiSettingsActivity";
    private static final String WIFI_SUB_SETTINGS_CLASS_NAME =
            "com.android.settings.SubSettings$WifiSubSettings";

    private static final String WIFI_SSID_BEGIN = "\"";
    private static final String WIFI_SSID_END = "\"";
    private static char SINGLE_QUOTES = '"';

    private static final int CMCC_NEED_APS_MIN = 2;
    private static final int WIFI_SIGNAL_GOOD = -85;
    private static final int WIFI_SIGNAL_WEEK = -79;
    private static final int SSID_FAULT = -1;
    private static final int RSSI_FAULT = -200;
    private static final int RSSI_SIGNAL_WEEK_LEVEL = 1;
    // The {@link System#currentTimeMillis()} must be at least this value for us
    // to show the notification again.
    private static final long NOTIFICATION_REPEAT_DELAY_MS = 60 * 60 * 1000;
    // for manually, the minimum time interval 5 minutes
    private static final long NOTIFICATION_REPEAT_MS_MANUALLY = 5 * 60 * 1000;

    private int mHighestPriority = -1;
    private int mHighestPriorityNetworkId = -1;
    private ConnectivityManager mConnService;
    private Context mContext;
    private int mBestApRssi = RSSI_FAULT;
    private int mBestSignalNID = SSID_FAULT;
    private String mBestApName;
    private String mHighestPriorityNetworkSSID = null;
    private NetworkInfo mNetworkInfo;
    private WifiManager mWifiManager;

    // Only one kind of dialog will pop up. The two kinds of dialog, Ask and
    // Manually dialog can not pop up at the same time.
    private static AlertDialog mCellularToWlanDlg = null;
    public static long mCellularToWlanDialogUserClickCancelTime = -1;
    public static boolean mCellularToWlanDialogNeverPopUp = false;
    private static AlertDialog mAirplaneWifiWarningDlg = null;
    private static AlertDialog mCmccWarningDlg = null;
    public static long mCmccWarningDlgStartTime = -1;
    private static AlertDialog mSignalWeakDlg = null;

    @Override
    public void onReceive(Context context, Intent intent) {

        mContext = context;
        String action = intent.getAction();

        Utils.log("receive action: " + action);

        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            setCmccFeatureDefaultValue();
            startService();
        } else if (WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) {
            DetailedState state = ((NetworkInfo) intent
                    .getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO)).getDetailedState();
            if (state == DetailedState.CONNECTED) {
                setWifiStatusAsconnected();
                if (needPromptCmccWarningDialog() && isSsidCmcc()) {
                    promptCmccIsConnected();
                }
            } else if (state == DetailedState.DISCONNECTED) {
                Utils.log("Disconnect from AP");
                if (!Utils.isSimCardReady()) {
                    Utils.log("Sim card is not Ready, Wlan->Cellular Hint feature disable");
                    return;
                }
                int netType = ((NetworkInfo) intent
                        .getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO)).getType();
                if (isWifiConnected() &&
                        netType == ConnectivityManager.TYPE_WIFI &&
                        Utils.isWlan2CellularHintEnabled(context)) {
                    getHighestPriorityNetworkId();
                    Utils.log("The highest priority network id is " + mHighestPriorityNetworkId);
                    // When one of the following conditions are met, will pop up dialog:
                    // 1. not auto connect type
                    // 2. user manually forget the connected AP which is the
                    // only available AP, use highest priority to check
                    // 3. user manually disconnect the only available AP
                    // 4. turn off or go out of the range ap which is last
                    // conncected.
                    if (!Utils.isAutoConnectMode(context) || mHighestPriorityNetworkId == -1
                            || !isManuallyDisconnectFromAp()
                            || getAvaiConfigNetworkSize() <= 1) {
                        showWlanDisconnectToast();
                    }
                }
            }
        } else if (WifiManager.SCAN_RESULTS_AVAILABLE_ACTION.equals(action)) {
            if (Utils.isAutoConnectMode(context)) {
                Utils.log("no cell->wlan dlg when auto mode");
                return;
            } else {
                // If auto connect mode is off, it is not necessary to
                // display the prompt dialog on the interface of
                // WifiSettingsActivity. Because although tap positive button ,
                // still need connect to the AP manually.
                if (isWifiSettingsActivity()) {
                    // TODO: this method is different begin at 3.7.1
                    return;
                }
            }
            if (Utils.isCellularWlanHintEnable(mContext) &&
                    hasAvaApDuringUsingMobileData(context)) {
                promptCellular2WlanDialog(context);
            }
        } else if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
            int wifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                    WifiManager.WIFI_STATE_UNKNOWN);
            if (wifiState == WifiManager.WIFI_STATE_DISABLED) {
                Utils.log("Wlan is trunning off by user, need toast");
                if (!Utils.isSimCardReady()) {
                    Utils.log("no sim card, do not need popup, return");
                    return;
                }
                if (Utils.isWlan2CellularHintEnabled(context)) {
                    showWlanDisconnectToast();
                }
            }
        } else if (WifiManager.WIFI_AP_STATE_CHANGED_ACTION.equals(action)) {
            if (WifiManager.WIFI_AP_STATE_ENABLED == intent.getIntExtra(
                    WifiManager.EXTRA_WIFI_AP_STATE,
                    WifiManager.WIFI_AP_STATE_FAILED)) {
                if (!isMobileDataEnabled()) {
                    Toast.makeText(
                            context,
                            context.getResources().getString(
                                    R.string.enable_Data_switch_after_wifiAp_enable),
                            Toast.LENGTH_SHORT).show();
                }
            }
        } else if (WifiManager.RSSI_CHANGED_ACTION.equals(action)) {
            if (Utils.isAutoConnectMode(context)) {
                Utils.log("auto mode, reselect ssid feature is disabled");
                return;
            }
            mConnService = (ConnectivityManager) mContext.
                    getSystemService(Context.CONNECTIVITY_SERVICE);
            if (mConnService == null) {
                return;
            }
            NetworkInfo netInfo = (NetworkInfo) mConnService
                    .getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            if (netInfo.isConnected()) {
                showSelectSsidDlg();
            }
        } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
            if (mAirplaneWifiWarningDlg != null) {
                Utils.log("mAirplaneWifiWarningDlg is not null");
                return;
            }
            if (Utils.needNotifyInAirplaneMode(mContext) && Utils.isAirplaneModeOn(context)) {
                showDialogInAirplaneMode();
            }
        }
    }

    private boolean isMobileDataEnabled() {
        final TelephonyManager tm =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        if (tm.getSimCount() > 1) {
            int preferredDataSubscription = SubscriptionManager.getDefaultDataSubId();
            Utils.log("preferredDataSubscription" + preferredDataSubscription);
            return android.provider.Settings.Global.getInt(mContext.getContentResolver(),
                    android.provider.Settings.Global.MOBILE_DATA
                            + SubscriptionManager.getPhoneId(preferredDataSubscription), 0) != 0;
        } else {
            return tm.getDataEnabled();
        }
    }

    private void showDialogInAirplaneMode() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        View layout = LayoutInflater.from(mContext).inflate(
                R.layout.dialog_view, null);
        builder.setView(layout);
        TextView textView = (TextView) layout.findViewById(R.id.msg_dialog_prompt);
        textView.setText(R.string.airplane_wifi_warning_msg);
        CheckBox checkBox = (CheckBox) layout.findViewById(R.id.is_ask_again);

        checkBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Settings.System.putInt(mContext.getContentResolver(),
                        Utils.AIRPLANE_WLAN_NEED_WARNING,
                        isChecked ? Utils.DO_NOT_NOTIFY_USER : Utils.NOTIFY_USER);
            }
        });
        checkBox.setChecked(false);
        builder.setPositiveButton(R.string.airplane_ok, null);
        AlertDialog dlg = builder.create();
        dlg.getWindow().setType(android.view.WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        dlg.setOnDismissListener(new DialogInterface.OnDismissListener() {
            public void onDismiss(DialogInterface dialog) {
                mAirplaneWifiWarningDlg = null;
            }
        });
        dlg.show();
        mAirplaneWifiWarningDlg = dlg;
    }

    private void showSelectSsidDlg() {
        if (mSignalWeakDlg != null) {
            return;
        }
        mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        if (mWifiManager == null) {
            return;
        }
        if (!mWifiManager.isWifiEnabled()) {
            return;
        }

        // It is not necessary to display the prompt dialog
        // when on the interface of WifiSettingsActivity
        if (isWifiSettingsActivity()) {
            return;
        }

        WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
        if (wifiInfo == null) {
            return;
        }

        int rssi = wifiInfo.getRssi();
        int newSignalLevel = WifiManager.calculateSignalLevel(rssi, WifiManager.RSSI_LEVELS);
        Utils.log("wlan signal is " + rssi);
        if (newSignalLevel > RSSI_SIGNAL_WEEK_LEVEL) {
            // Signal is good, nothing need to be done.
            return;
        } else {
            Utils.log("Requirement: signal is too weak, disconnect!");
            mWifiManager.disconnect();
        }

        if (!Utils.isCellularWlanHintEnable(mContext)) {
            Utils.log("Since hint is disable, return.");
            return;
        }

        List<WifiConfiguration> configs = mWifiManager.getConfiguredNetworks();
        int configCount = configs == null ? 0 : configs.size();
        if (configCount < CMCC_NEED_APS_MIN) {
            Utils.log("APs are less than 2, then no AP is better than current AP, just return");
            return;
        }

        mBestSignalNID = getBestSignalNetworkId();
        if (SSID_FAULT == mBestSignalNID) {
            Utils.log("Can not get any best signal network id, return");
            return;
        }

        final int dataToWifiConnectType = Settings.System.getInt(mContext.getContentResolver(),
                Utils.DATA_TO_WIFI_CONNECT_TYPE, Utils.DATA_WIFI_CONNECT_TYPE_AUTO);

        AlertDialog.Builder b = new AlertDialog.Builder(mContext);
        b.setPositiveButton(com.android.internal.R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        if (dataToWifiConnectType == Utils.DATA_WIFI_CONNECT_TYPE_MANUAL) {
                            Intent intent = new Intent();
                            intent.setAction(ACTION_WIFI_SETTING);
                            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                            mContext.startActivity(intent);
                        } else if (dataToWifiConnectType == Utils.DATA_WIFI_CONNECT_TYPE_ASK) {
                            mWifiManager.connect(mBestSignalNID,
                                    new WifiManager.ActionListener() {
                                        @Override
                                        public void onSuccess() {
                                        }

                                        @Override
                                        public void onFailure(int reason) {
                                        }
                                    });
                        }
                    }
                });
        b.setNegativeButton(com.android.internal.R.string.cancel,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                    }
                });
        if (dataToWifiConnectType == Utils.DATA_WIFI_CONNECT_TYPE_MANUAL) {
            b.setMessage(R.string.wlan_signal_weak);
        } else if (dataToWifiConnectType == Utils.DATA_WIFI_CONNECT_TYPE_ASK) {
            String msg = mContext.getResources().getString(R.string.wlan_signal_weak_ask,
                    mBestApName);
            b.setMessage(msg);
        }
        AlertDialog dlg = b.create();
        dlg.setOnDismissListener(new DialogInterface.OnDismissListener() {
            public void onDismiss(DialogInterface dialog) {
                mSignalWeakDlg = null;
            }
        });
        dlg.getWindow().setType(android.view.WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        dlg.show();
        mSignalWeakDlg = dlg;
    }

    private void showWlanDisconnectToast() {
        Utils.log("Toast user wlan disconnect.");
        Toast.makeText(mContext, mContext.getResources().getString(R.string.wlan_disconnect_toast),
                Toast.LENGTH_LONG).show();
    }

    private boolean hasAvaApDuringUsingMobileData(Context context) {
        WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        if (wifiManager == null || !wifiManager.isWifiEnabled()) {
            Utils.log("Wlan is not enable, no need pop");
            return false;
        }
        List<ScanResult> scanResults = wifiManager.getScanResults();
        if (scanResults == null || scanResults.size() == 0) {
            Utils.log("No available AP, no need pop");
            return false;
        }

        ConnectivityManager cm = (ConnectivityManager) context
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        if (cm == null) {
            return false;
        }
        NetworkInfo networkInfo = cm.getActiveNetworkInfo();
        if (networkInfo == null) {
            return false;
        }
        NetworkInfo.State state = networkInfo.getState();
        if ((state == NetworkInfo.State.CONNECTED)
                && (networkInfo.getType() == ConnectivityManager.TYPE_MOBILE)) {
            getHighestPriorityNetworkId();
            if (mHighestPriorityNetworkId == -1
                    || TextUtils.isEmpty(mHighestPriorityNetworkSSID)) {
                Utils.log("Get network id -1, or get ssid null, no cellular->wlan dialog");
                return false;
            }
            return true;
        }
        return false;
    }

    /*
     * Clears variables related to tracking whether a Dialog has been shown
     * recently.
     */
    public static void resetCellularToWlanDialogControlParas() {
        // Cancel time counting to -1
        mCellularToWlanDialogUserClickCancelTime = -1;
        // CheckBox status to false
        mCellularToWlanDialogNeverPopUp = false;
    }

    public void setCellularToWlanDialogUserClickCancelTime() {
        mCellularToWlanDialogUserClickCancelTime = System.currentTimeMillis();
    }

    private void promptCellular2WlanDialog(Context context) {
        if (mCellularToWlanDlg != null) {
            return;
        }
        int dataToWifiConnectType = Settings.System.getInt(context.getContentResolver(),
                Utils.DATA_TO_WIFI_CONNECT_TYPE, Utils.DATA_WIFI_CONNECT_TYPE_AUTO);
        Utils.log("mHighestPriorityNetworkId=" + mHighestPriorityNetworkId
                + "//mHighestPriorityNetworkSSID=" + mHighestPriorityNetworkSSID);
        if (dataToWifiConnectType == Utils.DATA_WIFI_CONNECT_TYPE_ASK) {
            if ((System.currentTimeMillis() - mCellularToWlanDialogUserClickCancelTime
            > NOTIFICATION_REPEAT_DELAY_MS)) {
                AlertDialog.Builder b = new AlertDialog.Builder(mContext);
                b.setPositiveButton(com.android.internal.R.string.ok,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                if (mWifiManager != null) {
                                    mWifiManager.connect(mHighestPriorityNetworkId,
                                            new WifiManager.ActionListener() {
                                                public void onSuccess() {
                                                }

                                                public void onFailure(int reason) {
                                                }
                                            });
                                }
                            }
                        });
                b.setNegativeButton(com.android.internal.R.string.cancel,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                setCellularToWlanDialogUserClickCancelTime();
                            }
                        });
                b.setTitle(context.getResources().getString(R.string.wifi_title));
                String msg = String.format(
                        mContext.getResources().getString(R.string.cellular_wlan_ask),
                        mHighestPriorityNetworkSSID);
                b.setMessage(msg);
                AlertDialog dlg = b.create();
                dlg.setOnDismissListener(new DialogInterface.OnDismissListener() {
                    public void onDismiss(DialogInterface dialog) {
                        mCellularToWlanDlg = null;
                    }
                });
                dlg.getWindow().setType(android.view.WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
                dlg.show();
                mCellularToWlanDlg = dlg;
            }
        } else if (dataToWifiConnectType == Utils.DATA_WIFI_CONNECT_TYPE_MANUAL) {
            if ((System.currentTimeMillis() - mCellularToWlanDialogUserClickCancelTime
            > NOTIFICATION_REPEAT_MS_MANUALLY)) {
                AlertDialog.Builder b = new AlertDialog.Builder(mContext);
                b.setMessage(mContext.getResources().getString(R.string.cellular_wlan_manual));
                b.setPositiveButton(com.android.internal.R.string.ok,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                Intent intent = new Intent();
                                intent.setAction(ACTION_WIFI_SETTING);
                                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                                mContext.startActivity(intent);
                            }
                        });
                b.setNegativeButton(com.android.internal.R.string.cancel,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                setCellularToWlanDialogUserClickCancelTime();
                            }
                        });
                AlertDialog dlg = b.create();
                dlg.setOnDismissListener(new DialogInterface.OnDismissListener() {
                    public void onDismiss(DialogInterface dialog) {
                        mCellularToWlanDlg = null;
                    }
                });
                dlg.getWindow().setType(android.view.WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
                dlg.show();
                mCellularToWlanDlg = dlg;
            }
        } else {
            Utils.log("auto mode, do nothing");
        }
    }

    private boolean isSsidCmcc() {
        WifiManager wm = (WifiManager) mContext.getSystemService(Service.WIFI_SERVICE);
        WifiInfo wifiInfo = wm.getConnectionInfo();
        String ssidWithQuote = Utils.convertToQuotedString(Utils.SSID_CMCC);
        if (wifiInfo == null || wifiInfo.getSSID() == null) {
            return false;
        }
        return wifiInfo.getSSID().equals(ssidWithQuote);
    }

    private boolean needPromptCmccWarningDialog() {
        return Settings.System.getInt(mContext.getContentResolver(),
                Utils.NOTIFY_USER_CONNECT_CMCC, Utils.NOTIFY_USER) == Utils.NOTIFY_USER;
    }

    private void promptCmccIsConnected() {
        Toast.makeText(mContext, mContext.getResources().getString(R.string.connect_cmcc_warning),
                Toast.LENGTH_SHORT).show();
    }

    private int getNetworkRssi(String SSID) {
        int rssi = Integer.MAX_VALUE;
        List<ScanResult> results = mWifiManager.getScanResults();
        if (SSID != null && results != null) {
            for (ScanResult result : results) {
                if (SSID.equals(WIFI_SSID_BEGIN + result.SSID + WIFI_SSID_END)) {
                    rssi = result.level;
                    break;
                }
            }
        }
        return rssi;
    }

    private int getBestSignalNetworkId() {
        int networkId = SSID_FAULT;
        int rssi = RSSI_FAULT;
        List<WifiConfiguration> configs = mWifiManager.getConfiguredNetworks();
        List<ScanResult> results = mWifiManager.getScanResults();
        if (configs != null && results != null) {
            for (WifiConfiguration config : configs) {
                for (ScanResult result : results) {
                    if (config != null && config.SSID != null
                            && removeDoubleQuotes(config.SSID).equals(result.SSID)
                            && getSecurity(config) == getSecurity(result)) {
                        if (WifiManager.compareSignalLevel(result.level, rssi) > 0) {
                            networkId = config.networkId;
                            rssi = result.level;
                            mBestApName = config.SSID;
                            mBestApRssi = rssi;
                        }
                    }
                }
            }
        }
        if (WifiManager.compareSignalLevel(rssi, WIFI_SIGNAL_WEEK) > 0) {
            Utils.log("there is ap's signal is better than -79.networkId=" + networkId);
            return networkId;
        } else {
            return SSID_FAULT;
        }
    }

    private boolean isWifiSettingsActivity() {
        // TODU: This need checked.
        ActivityManager am = (ActivityManager) mContext
                .getSystemService(Context.ACTIVITY_SERVICE);
        ComponentName cn = null;
        String classname = null;
        if (am.getRunningTasks(1) != null && am.getRunningTasks(1).get(0) != null) {
            cn = am.getRunningTasks(1).get(0).topActivity;
        }
        if (cn != null) {
            classname = cn.getClassName();
            Utils.log("Class Name:" + classname);
        } else {
            Utils.log("ComponentName is null");
        }

        if (classname != null && (classname.equals(WIFI_SETTINGS_CLASS_NAME) ||
                classname.equals(WIFI_SUB_SETTINGS_CLASS_NAME))) {
            Utils.log("In wifi settings screen!");
            return true;
        }

        return false;
    }

    private void getHighestPriorityNetworkId() {
        String highestPriorityNetworkSSID = null;
        int highestPriority = -1;
        int highestPriorityNetworkId = -1;

        mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        List<WifiConfiguration> configs = mWifiManager.getConfiguredNetworks();
        List<ScanResult> results = mWifiManager.getScanResults();
        if (null != configs) {
            for (WifiConfiguration config : configs) {
                for (ScanResult result : results) {
                    if ((config.SSID != null) && (result.SSID != null)
                            && removeDoubleQuotes(config.SSID).equals(result.SSID)
                            && (getSecurity(config) == getSecurity(result))) {
                        if (config.priority > highestPriority) {
                            highestPriority = config.priority;
                            highestPriorityNetworkId = config.networkId;
                            highestPriorityNetworkSSID = config.SSID;
                        }
                    }
                }
            }
        }

        mHighestPriorityNetworkSSID = highestPriorityNetworkSSID;
        mHighestPriority = highestPriority;
        mHighestPriorityNetworkId = highestPriorityNetworkId;
    }

    private int getAvaiConfigNetworkSize() {
        int invalidPriority = -1;
        int availableNetworkSize = 0;

        mWifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        List<WifiConfiguration> configs = mWifiManager.getConfiguredNetworks();
        List<ScanResult> results = mWifiManager.getScanResults();
        if (null != configs) {
            for (WifiConfiguration config : configs) {
                for (ScanResult result : results) {
                    Utils.log("rssi level" + result.level);
                    if (result.level < -85) {
                        continue;
                    }
                    if ((config.SSID != null) && (result.SSID != null)
                            && removeDoubleQuotes(config.SSID).equals(result.SSID)
                            && (getSecurity(config) == getSecurity(result))) {
                        if (config.priority > invalidPriority) {
                            availableNetworkSize++;
                        }
                    }
                }
            }
        }
        Utils.log("getAvaiConfigNetworkSize() -- " + availableNetworkSize);
        return availableNetworkSize;
    }

    static String removeDoubleQuotes(String string) {
        int length = string.length();
        if ((length > 1) && (string.charAt(0) == SINGLE_QUOTES)
                && (string.charAt(length - 1) == SINGLE_QUOTES)) {
            return string.substring(1, length - 1);
        }
        return string;
    }

    static int getSecurity(WifiConfiguration config) {
        if (config.allowedKeyManagement.get(KeyMgmt.WPA_PSK)) {
            return SECURITY_PSK;
        }
        if (config.allowedKeyManagement.get(KeyMgmt.WPA_EAP)
                || config.allowedKeyManagement.get(KeyMgmt.IEEE8021X)) {
            return SECURITY_EAP;
        }
        return (config.wepKeys[0] != null) ? SECURITY_WEP : SECURITY_NONE;
    }

    private static int getSecurity(ScanResult result) {
        if (result.capabilities.contains(WAPI_PSK_STRING)) {
            return SECURITY_WAPI_PSK;
        } else if (result.capabilities.contains(WEP_STRING)) {
            return SECURITY_WEP;
        } else if (result.capabilities.contains(PSK_STRING)) {
            return SECURITY_PSK;
        } else if (result.capabilities.contains(EAP_STRING)) {
            return SECURITY_EAP;
        } else if (result.capabilities.contains(WAPI_CERT_STRING)) {
            return SECURITY_WAPI_CERT;
        }
        Utils.log("private getSecurity: " + result.capabilities);
        return SECURITY_NONE;
    }

    private boolean isManuallyDisconnectFromAp() {
        boolean isManuallyDisconnect = Settings.System.getInt(
                mContext.getContentResolver(),
                Utils.DISCONNECT_FROM_NETWORK,
                Utils.NOT_MANUALLY_DISCONNECT_LAST_AVAILABLE_AP)
                == Utils.NOT_MANUALLY_DISCONNECT_LAST_AVAILABLE_AP;
        // If this AP is user manually disconnect last available AP,
        // should restore to the default values.
        if (!isManuallyDisconnect) {
            Settings.System.putInt(mContext.getContentResolver(), Utils.DISCONNECT_FROM_NETWORK,
                    Utils.NOT_MANUALLY_DISCONNECT_LAST_AVAILABLE_AP);
        }
        return isManuallyDisconnect;
    }

    private void setCmccFeatureDefaultValue() {
        if (!isCmccFeatureDefaultValueSaved()) {
            Utils.log("set default one time");
            Settings.System.putInt(mContext.getContentResolver(),
                    Utils.DATA_TO_WIFI_CONNECT_TYPE,
                    Utils.CELLULAR_WLAN_DEFAULT_VALUE);
            Settings.System.putInt(mContext.getContentResolver(),
                    Utils.WIFI_AUTO_CONNECT_TYPE,
                    Utils.AUTO_CONNECT_DEFAULT_VALUE);
            Settings.System.putInt(mContext.getContentResolver(),
                    Utils.WLAN_CELLULAR_HINT,
                    Utils.WLAN_CELLULAR_HINT_DEFAULT_VALUE);
            Settings.System.putInt(mContext.getContentResolver(),
                    Utils.NOTIFY_USER_CONNECT_CMCC,
                    Utils.NOTIFY_USER);
        }
        setDefaultValueSaved();
    }

    private void setDefaultValueSaved() {
        Settings.System.putInt(mContext.getContentResolver(),
                SAVE_CMCC_FEATURE_DEFAULT_VALUE, CMCC_DEFAULT_VALUE_SAVED);
    }

    private boolean isCmccFeatureDefaultValueSaved() {
        return Settings.System.getInt(mContext.getContentResolver(),
                SAVE_CMCC_FEATURE_DEFAULT_VALUE, CMCC_DEFAULT_VALUE_NOT_SAVED)
                == CMCC_DEFAULT_VALUE_SAVED;
    }

    private void setWifiStatusAsconnected() {
        SharedPreferences sharedPreferences = mContext.getSharedPreferences(
                Utils.SHARE_PREFERENCE_FILE_NAME,
                Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putBoolean(Utils.KEY_WIFI_CONNECT, true);
        editor.commit();
    }

    private boolean isWifiConnected() {
        SharedPreferences sharedPreferences = mContext.getSharedPreferences(
                Utils.SHARE_PREFERENCE_FILE_NAME,
                Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        boolean isConnected = sharedPreferences.getBoolean(Utils.KEY_WIFI_CONNECT, false);
        editor.putBoolean(Utils.KEY_WIFI_CONNECT, false);
        editor.commit();
        return isConnected;
    }

    private void startService() {
        Intent i = new Intent(Utils.ACTION_CMCC_SERVICE);
        i.setClassName(mContext.getPackageName(), CmccService.class.getName());
        mContext.startService(i);
    }
}
