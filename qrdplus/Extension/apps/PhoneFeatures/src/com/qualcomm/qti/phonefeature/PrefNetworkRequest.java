/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Log;

import com.android.internal.telephony.Phone;

public class PrefNetworkRequest extends SyncQueue.SyncRequest {

    private static final String TAG = "pref_network_request";

    private static final void logd(String msg) {
        Log.d(TAG, msg);
    }

    private static final SyncQueue sSyncQueue = new SyncQueue();

    private final Message mCallback;
    private final List<PrefNetworkSetCommand> commands;
    private final Context mContext;
    private Integer mAcqOrder;
    private Integer mBand;

    private static final int EVENT_SET_PREF_NETWORK_DONE = 1;
    private static final int EVENT_GET_PREF_NETWORK_DONE = 2;
    private static final int EVENT_LOAD_QCRIL_DONE = 3;
    private static final int EVENT_START_REQUEST = 4;

    private Handler mHandler = new Handler(Looper.getMainLooper()) {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SET_PREF_NETWORK_DONE:
                    handleSetPreferredNetwork(msg);
                    break;
                case EVENT_GET_PREF_NETWORK_DONE:
                    handleGetPreferredNetwork(msg);
                    break;
                case EVENT_LOAD_QCRIL_DONE:
                    AppGlobals.getInstance().unregisterQcRilHookLoaded(this);
                    request((Integer) ((AsyncResult) msg.obj).userObj);
                    break;
                case EVENT_START_REQUEST:
                    request((Integer) msg.obj);
                    break;
            }
        }

    };

    private class PrefNetworkSetCommand {
        private final int mSlot;
        private final int mPrefNetwork;

        private PrefNetworkSetCommand(int slot, int prefNetwork) {
            mSlot = slot;
            mPrefNetwork = prefNetwork;
        }
    }

    private void request(final int index) {
        final PrefNetworkSetCommand command = commands.get(index);
        int acq = Constants.NW_ACQ_ORDER_LTE_TDS_GSM;
        if (AppGlobals.getInstance().mServiceMonitor.isRoaming(command.mSlot)) {
            acq = Constants.NW_ACQ_ORDER_LTE_UMTS_GSM;
        } else if (mAcqOrder != null) {
            acq = mAcqOrder;
        }
        // only need to set ACQ for target slot(last position)
        boolean acqEnabled = mContext.getResources().getBoolean(R.bool.feature_acq);
        int band = Constants.NW_BAND_LTE_DEFAULT;
        if (mBand != null) {
            band = mBand;
        }
        boolean needLoadQcRil = index == commands.size() - 1 && (acqEnabled ||
                band != Constants.NW_BAND_LTE_NV);
        if (needLoadQcRil && !AppGlobals.getInstance().isQcRilHookLoaded()) {
            AppGlobals.getInstance().registerQcRilHookLoaded(mHandler,
                    EVENT_LOAD_QCRIL_DONE, index);
            return;
        }
        if (needLoadQcRil) {
            if (acqEnabled) {
                boolean success = AppGlobals.getInstance().mQcRilHook
                        .qcRilSetPreferredNetworkAcqOrder(acq, command.mSlot);
                logd("set acq " + acq + " for slot" + command.mSlot + " done, " + success);
                if (mAcqOrder != null && success) {
                    Constants.saveIntSetting(mContext, command.mSlot, Constants.SETTING_ACQ,
                            mAcqOrder);
                }
            }
            if (band != Constants.NW_BAND_LTE_NV) {
                boolean success = AppGlobals.getInstance().mQcRilHook
                        .qcRilSetPreferredNetworkBandPref(band, command.mSlot);
                logd("set band " + band + " for slot" + command.mSlot + " done, " + success);
                if (success) {
                    Constants.saveIntSetting(mContext, command.mSlot,
                            Constants.SETTING_NETWORK_BAND, band);
                    if (mBand != null) {
                        Constants.saveIntSetting(mContext, command.mSlot,
                                Constants.SETTING_PREF_NETWORK_BAND, band);
                    }
                }
            }
        }
        logd("set " + command.mPrefNetwork + " for slot" + command.mSlot);
        AppGlobals.getInstance().mPhones[command.mSlot].setPreferredNetworkType(
                command.mPrefNetwork,
                mHandler.obtainMessage(EVENT_SET_PREF_NETWORK_DONE, index));
    }

    private void handleGetPreferredNetwork(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        int index = (Integer) ar.userObj;
        PrefNetworkSetCommand command = commands.get(index);
        if (ar.exception == null) {
            int modemNetworkMode = ((int[]) ar.result)[0];
            Constants.saveIntSetting(mContext, command.mSlot,
                    Settings.Global.PREFERRED_NETWORK_MODE, modemNetworkMode);
            if (isPersistPrefNetwork()) {
                Constants.saveIntSetting(mContext, command.mSlot,
                        Constants.SETTING_DEFAULT_PREF_NETWORK_MODE, modemNetworkMode);
            }
        }
        logd("get perferred network for slot" + command.mSlot + " done, " + ar.exception);
        if (++index < commands.size()) {
            request(index);
        } else {
            Constants.response(null, mCallback);
            end();
        }
    }

    private void handleSetPreferredNetwork(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        int index = (Integer) ar.userObj;
        PrefNetworkSetCommand command = commands.get(index);
        logd("set " + command.mPrefNetwork + " for slot" + command.mSlot + " done, "
                + ar.exception);
        if (ar.exception == null) {
            Constants.saveIntSetting(mContext, command.mSlot,
                    Settings.Global.PREFERRED_NETWORK_MODE, command.mPrefNetwork);
            if (isPersistPrefNetwork()) {
                Constants.saveIntSetting(mContext, command.mSlot,
                        Constants.SETTING_DEFAULT_PREF_NETWORK_MODE, command.mPrefNetwork);
            }
        } else {
            AppGlobals.getInstance().mPhones[command.mSlot].getPreferredNetworkType(
                    mHandler.obtainMessage(EVENT_GET_PREF_NETWORK_DONE, index));
            return;
        }
        if (++index < commands.size()) {
            request(index);
        } else {
            Constants.response(null, mCallback);
            end();
        }
    }

    private boolean isPersistPrefNetwork() {
        return mBand == null;
    }

    public PrefNetworkRequest(Context context, int slot, int networkMode, Message callback) {
        super(sSyncQueue);
        mContext = context;
        mCallback = callback;
        commands = new ArrayList<PrefNetworkSetCommand>();
        if (networkMode != Phone.NT_MODE_GSM_ONLY) {
            for (int index = 0; index < Constants.PHONE_COUNT; index++) {
                if (index != slot)
                    commands.add(new PrefNetworkSetCommand(index, Phone.NT_MODE_GSM_ONLY));
            }
        }
        if (slot >= 0 && slot < Constants.PHONE_COUNT) {
            commands.add(new PrefNetworkSetCommand(slot, networkMode));
        }
    }

    public void setAcqOrder(int acq) {
        mAcqOrder = acq;
    }

    public void setBand(int band) {
        mBand = band;
    }

    protected void start() {
        if (commands.isEmpty()) {
            logd("no command sent");
            Constants.response(null, mCallback);
            end();
        } else {
            PrefNetworkSetCommand command = commands.get(commands.size() - 1);
            logd("try to set network=" + command.mPrefNetwork + ", acq=" + mAcqOrder
                    + ", band=" + mBand + " on slot" + command.mSlot);
            mHandler.obtainMessage(EVENT_START_REQUEST, 0).sendToTarget();
        }
    }
}
