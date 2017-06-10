/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.app.AlertDialog;
import android.app.Service;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemClock;
import android.os.SystemVibrator;
import android.os.Vibrator;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.DisconnectCause;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.WindowManager;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;

public class FeatureService extends Service {

    private RoamingSetting mRoamingSetting;
    private SimDetector mSimDetector;
    private Call.State mLastFgCallState = Call.State.IDLE;
    private AlertDialog mCallDurationDialog;

    public static final String ACTION_ENABLE_TDD_LTE =
            "com.qualcomm.qti.phonefeature.ENABLE_TDD_LTE";

    private static final String SP_PROVISION = "provision";

    private static final long TDD_REGISTER_TIMEOUT = 3 * 60 * 1000;

    private static final int EVENT_ROAMING_CHANGED = 1;
    private static final int EVENT_SERVICE_CHANGED = 2;
    private static final int EVENT_CALL_STATE_CHANGED = 3;
    private static final int EVENT_CALL_DISCONNECTED = 4;
    private static final int EVENT_TIMEOUT_SET_TDD_LTE = 5;
    private static final int EVENT_TIMEOUT_SET_TDD_LTE_AGAIN = 6;
    private static final int EVENT_SET_TDD_LTE_DONE = 7;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_ROAMING_CHANGED:
                    Log.d(AppGlobals.TAG, "on EVENT_ROAMING_CHANGED");
                    if (getResources().getBoolean(R.bool.feature_acq)) {
                        updatePrefNetwork((Integer) ((AsyncResult) msg.obj).result);
                    }
                    break;
                case EVENT_SERVICE_CHANGED:
                    int slotId = (Integer) ((AsyncResult) msg.obj).result;
                    int dds = SubscriptionManager.getSlotId(PhoneFactory.getDataSubscription());
                    Log.d(AppGlobals.TAG, "EVENT_SERVICE_CHANGED on slot=" + slotId + ", dds="
                            + dds);
                    if (slotId == dds) {
                        ApnController.getInstance(FeatureService.this).updateApn();
                    }
                    break;
                case EVENT_CALL_STATE_CHANGED:
                    onPhoneStateChanged();
                    break;
                case EVENT_CALL_DISCONNECTED:
                    onCallDisconnected((AsyncResult) msg.obj);
                    break;
                case EVENT_TIMEOUT_SET_TDD_LTE:
                case EVENT_TIMEOUT_SET_TDD_LTE_AGAIN:
                    int serviceState = AppGlobals.getInstance().mPhones[(Integer) msg.obj]
                            .getServiceState().getDataRegState();
                    Log.d(AppGlobals.TAG, "on EVENT_TIMEOUT_SET_TDD_LTE, band="
                            + getBand((Integer) msg.obj) + ", data_service_state=" + serviceState);
                    if (getBand((Integer) msg.obj) == Constants.NW_BAND_LTE_TDD
                            && serviceState != ServiceState.STATE_IN_SERVICE) {
                        alertNotRegisteredInTDD((Integer) msg.obj,
                                msg.what == EVENT_TIMEOUT_SET_TDD_LTE);
                    }
                    break;

                case EVENT_SET_TDD_LTE_DONE:
                    Log.d(AppGlobals.TAG, "on EVENT_SET_TDD_LTE_DONE, band=" + getBand(msg.arg1));
                    if (getBand(msg.arg1) == Constants.NW_BAND_LTE_TDD) {
                        sendMessageDelayed(obtainMessage(EVENT_TIMEOUT_SET_TDD_LTE, msg.arg1),
                                TDD_REGISTER_TIMEOUT);
                    }
                    Message callback = null;
                    if (msg.obj != null) {
                        callback = (Message) ((Bundle) msg.obj)
                                .getParcelable(FeatureProvider.EXTRA_USEROBJ);
                    }
                    Constants.response(null, callback);
                    break;
            }
        }

    };

    @Override
    public void onCreate() {
        super.onCreate();
        mSimDetector = new SimDetector(this);
        if (getResources().getBoolean(R.bool.feature_roaming_bind_data)) {
            mRoamingSetting = new RoamingSetting(this);
        }
        if (!isProvision()) {
            provision();
        }
        AppGlobals.getInstance().mServiceMonitor.registerRoamingStateChanged(mHandler,
                EVENT_ROAMING_CHANGED, null);
        AppGlobals.getInstance().mServiceMonitor.registerServiceStateChanged(mHandler,
                EVENT_SERVICE_CHANGED, null);
        CallManager.getInstance().registerForPreciseCallStateChanged(mHandler,
                EVENT_CALL_STATE_CHANGED, null);
        CallManager.getInstance().registerForDisconnect(mHandler, EVENT_CALL_DISCONNECTED, null);
    }

    protected void onCallDisconnected(AsyncResult r) {
        Connection c = (Connection) r.result;
        showCallDurationIfNeed(c);
    }

    protected void showCallDurationIfNeed(Connection c) {
        if (c == null) {
            Log.d(AppGlobals.TAG, "not need show duration, connection is null");
            return;
        }

        int cause = c.getDisconnectCause();
        if (cause == DisconnectCause.INCOMING_MISSED
                || cause == DisconnectCause.CALL_BARRED
                || cause == DisconnectCause.FDN_BLOCKED
                || cause == DisconnectCause.CS_RESTRICTED
                || cause == DisconnectCause.CS_RESTRICTED_EMERGENCY
                || cause == DisconnectCause.CS_RESTRICTED_NORMAL
                || cause == DisconnectCause.NOT_VALID
                || ((cause == DisconnectCause.INCOMING_REJECTED) && getResources().getBoolean(
                        R.bool.reject_call_as_missed_call))
                || cause == DisconnectCause.CALL_BLACKLISTED) {
            Log.d(AppGlobals.TAG, "not need show duration, caused by " + cause);
            return;
        } else if (Settings.System.getInt(getContentResolver(), "show_call_duration", 1) != 1) {
            Log.d(AppGlobals.TAG, "not need show duration, setting is disabled ");
            return;
        }

        if (mCallDurationDialog != null) {
            // Safe even if it is already dismissed
            mCallDurationDialog.dismiss();
            mCallDurationDialog = null;
        }

        long duration = c.getDurationMillis() / 1000;
        long minutes = 0;
        long seconds = 0;

        if (duration >= 60) {
            minutes = duration / 60;
        }
        seconds = duration % 60;

        mCallDurationDialog = new AlertDialog.Builder(this)
                .setTitle(R.string.title_dialog_duration)
                .setMessage(getString(R.string.duration_format, minutes, seconds))
                .create();
        mCallDurationDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        mCallDurationDialog.show();
        new Handler().postDelayed(new Runnable() {
            public void run() {
                if (mCallDurationDialog != null) {
                    mCallDurationDialog.dismiss();
                    mCallDurationDialog = null;
                }
            }
        }, 1000);
    }

    protected void onPhoneStateChanged() {
        Call.State state = CallManager.getInstance().getActiveFgCallState();
        if (mLastFgCallState.isDialing() && state == Call.State.ACTIVE) {
            vibrateAfterCallConnected();
        }
        mLastFgCallState = state;
    }

    private void vibrateAfterCallConnected() {
        if (Settings.System.getInt(getContentResolver(), "vibrate_on_accepted", 1) == 1) {
            Vibrator mSystemVibrator = new SystemVibrator();
            int nVibratorLength = 100;
            mSystemVibrator.vibrate(nVibratorLength);
            SystemClock.sleep(nVibratorLength);
            mSystemVibrator.cancel();
        }
    }

    private void provision() {
        provisionNetwork();
        setProvision();
    }

    private void provisionNetwork() {
        int network = getResources().getInteger(R.integer.provision_network);
        Log.d(AppGlobals.TAG, "provision network " + network);
        // save the network firstly, because even if the operation is failed,
        // phone process will set the network to RIL later
        Constants.saveIntSetting(this, 0, Settings.Global.PREFERRED_NETWORK_MODE, network);
        setPrefNetwork(0, network, null, null);
    }

    private boolean isProvision() {
        return PreferenceManager.getDefaultSharedPreferences(this).getBoolean(SP_PROVISION, false);
    }

    private boolean setProvision() {
        return PreferenceManager.getDefaultSharedPreferences(this).edit()
                .putBoolean(SP_PROVISION, true).commit();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null && ACTION_ENABLE_TDD_LTE.equals(intent.getAction())) {
            int slot = intent.getIntExtra(PhoneConstants.SLOT_KEY, 0);
            int band = intent.getIntExtra(FeatureProvider.EXTRA_BAND,
                    Constants.NW_BAND_LTE_DEFAULT);
            try {
                int network = intent.getIntExtra(
                        FeatureProvider.EXTRA_NETWORK,
                        band == Constants.NW_BAND_LTE_TDD ? Phone.NT_MODE_LTE_ONLY
                                : TelephonyManager.getIntAtIndex(getContentResolver(),
                                        Constants.SETTING_DEFAULT_PREF_NETWORK_MODE, slot));
                setPrefNetwork(slot, network, band,
                        (Message) intent.getParcelableExtra(FeatureProvider.EXTRA_CALLBACK));
            } catch (SettingNotFoundException e) {
                Log.e(AppGlobals.TAG, "onStartCommand: can not get previous network", e);
            }
        }
        return super.onStartCommand(intent, flags, startId);
    }

    private int getBand(int slot) {
        int band = Constants.NW_BAND_LTE_NV;
        try {
            band = TelephonyManager.getIntAtIndex(getContentResolver(),
                    Constants.SETTING_NETWORK_BAND, slot);
        } catch (SettingNotFoundException e) {
        }
        return band;
    }

    private void alertNotRegisteredInTDD(final int slot, final boolean notifyAgin) {
        DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (which == DialogInterface.BUTTON_NEGATIVE) {
                    int prefNetwork = Phone.NT_MODE_GSM_ONLY;
                    try {
                        prefNetwork = TelephonyManager.getIntAtIndex(getContentResolver(),
                                Constants.SETTING_DEFAULT_PREF_NETWORK_MODE, slot);
                    } catch (SettingNotFoundException e) {
                        Log.e(AppGlobals.TAG, "onClick: can not get previous network", e);
                        return;
                    }
                    setPrefNetwork(slot, prefNetwork, Constants.NW_BAND_LTE_FDD, null);
                } else if (notifyAgin) {
                    mHandler.sendMessageDelayed(
                            mHandler.obtainMessage(EVENT_TIMEOUT_SET_TDD_LTE_AGAIN, slot),
                            TDD_REGISTER_TIMEOUT);
                }
            }
        };
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        AlertDialog dialog = builder.setTitle(R.string.alert_title)
                .setMessage(R.string.message_not_registered_in_tdd)
                .setNegativeButton(R.string.choose_yes, listener)
                .setNeutralButton(R.string.choose_no, listener).create();
        dialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        dialog.show();
    }

    private void updatePrefNetwork(int slot) {
        int prefNetwork = Phone.NT_MODE_GSM_ONLY;
        try {
            prefNetwork = TelephonyManager.getIntAtIndex(getContentResolver(),
                    Settings.Global.PREFERRED_NETWORK_MODE, slot);
        } catch (SettingNotFoundException e) {
            Log.e(AppGlobals.TAG, "updatePrefNetwork: failed to get network on slot" + slot, e);
            return;
        }
        int band = getBand(slot);
        if (band == Constants.NW_BAND_LTE_NV || band == Constants.NW_BAND_LTE_DEFAULT) {
            setPrefNetwork(slot, prefNetwork, null, null);
        } else {
            setPrefNetwork(slot, prefNetwork, band, null);
        }
    }

    private void setPrefNetwork(int slot, int network, Integer band, Message callback) {
        mHandler.removeMessages(EVENT_TIMEOUT_SET_TDD_LTE);
        mHandler.removeMessages(EVENT_TIMEOUT_SET_TDD_LTE_AGAIN);
        Message msg = callback;
        int prefNetwork = network;
        if (band != null && band == Constants.NW_BAND_LTE_TDD) {
            msg = mHandler.obtainMessage(EVENT_SET_TDD_LTE_DONE, callback);
            msg.arg1 = slot;
        }
        PrefNetworkRequest request = new PrefNetworkRequest(this, slot, prefNetwork, msg);
        // ACQ is available for all networks, so keep it not be changed.
        try {
            request.setAcqOrder(TelephonyManager.getIntAtIndex(getContentResolver(),
                    Constants.SETTING_ACQ, slot));
        } catch (SettingNotFoundException e) {
        }
        if (band != null) {
            request.setBand(band);
        }
        request.loop();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mSimDetector != null) {
            mSimDetector.dispose();
        }
        if (mRoamingSetting != null) {
            mRoamingSetting.dispose();
        }
        CallManager.getInstance().unregisterForDisconnect(mHandler);
        CallManager.getInstance().unregisterForPreciseCallStateChanged(mHandler);
        AppGlobals.getInstance().mServiceMonitor.unregisterRoamingStateChanged(mHandler);
        AppGlobals.getInstance().mServiceMonitor.unregisterServiceStateChanged(mHandler);
    }
}
