/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.RecoverySystem;
import android.os.RemoteException;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;
import com.qualcomm.qti.accesscache.ICarrierAccessCacheService;
import com.qualcomm.qti.carrierconfigure.Carrier.SwitchData;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Locale;

public class Actions {
    public interface ActionCallback {
        public void onActionFinished(int requestCode);
        public void onActionError(int requestCode, int resultCode, Exception ex);
    }

    public static final int REQUEST_SWITCH_CARRIER = 0;
    public static final int REQUEST_UPDATE_NV_ITEMS = 1;
    public static final int REQUEST_UPDATE_ROW_NV_ITEMS = 2;

    public static final int RESULT_OK = 0;
    public static final int RESULT_FAILURE = -1;
    public static final int RESULT_NO_MBN = -2;

    /**
     * This service will be running in the phone process as it used to edit the sim mode prop.
     */
    public static class UpdateSimMode extends Service {
        public static final String ACTION_SIM_MODE_UPDATE =
                "com.qualcomm.qti.carrierconfigure.simmode.update";
        public static final String EXTRA_NEW_MODE = "new_mode";

        @Override
        public int onStartCommand(Intent intent, int flags, int startId) {
            if (ACTION_SIM_MODE_UPDATE.equals(intent.getAction())) {
                String newValue = intent.getStringExtra(EXTRA_NEW_MODE);
                Utils.setSimMode(newValue);
                Log.i("UpdateSimModeService", "Update the sim mode to " + newValue);

                PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
                pm.reboot("Reboot for change sim mode to: " + newValue);
            }
            return super.onStartCommand(intent, flags, startId);
        }

        @Override
        public IBinder onBind(Intent intent) {
            return null;
        }
    }

    /**
     * This AsyncTask used to set the MBN file to update the NV items.
     */
    public static class UpdateNVItemsTask extends AsyncTask<Carrier, Void, Void> {
        private static final String TAG = "UpdateNVItemsTask";

        // These values need same as the definition in the QcrilMsgTunnelSocket.
        // As them will be send by the QcrilMsgTunnelSocket.
        private static final String EXTRA_PDC_ACTIVE = "active";
        private static final String ACTION_PDC_DATA_RECEIVED =
                "qualcomm.intent.action.ACTION_PDC_DATA_RECEIVED";
        private static final String ACTION_PDC_CONFIGS_CLEARED =
                "qualcomm.intent.action.ACTION_PDC_CONFIGS_CLEARED";
        private static final String MBN_CONFIG_ID_PREFIX = "Regional_";
        private static final String ROW_MBN_CONFIG_ID_PREFIX = MBN_CONFIG_ID_PREFIX + "ROW_";
        // The result for these two actions.
        private static final int RESULT_SUCCESS = 0;
        private static final int RESULT_FAILURE = -1;

        private static final int RESULT_NEED_CLEAN = -2;
        private static final int RESULT_ALREADY_CLEAN = -3;

        // The messages will be handled.
        private static final int MSG_READY_CONFIG = 0;
        private static final int MSG_DEACTIVATE_CONFIG = MSG_READY_CONFIG + 1;
        private static final int MSG_SET_CONFIG = MSG_DEACTIVATE_CONFIG + 1;
        private static final int MSG_ACTION_TIMEOUT = MSG_SET_CONFIG + 1;
        private static final int MSG_ACTION_FINISHED = MSG_ACTION_TIMEOUT + 1;

        // Set this update action's timeout is 10 seconds.
        private static final int TIMEOUT_MILLIS = 10 * 1000;

        // The max retry time if the action is failure.
        private static final int MAX_RETRY_TIMES = 5;

        private static final int MODE_NONE = 0;
        private static final int MODE_DEACTIVATE = MODE_NONE + 1;
        private static final int MODE_SET = MODE_DEACTIVATE + 1;

        private int mRetryTimes = 0;
        private boolean mReady = false;
        private int mCurActionMode = MODE_NONE;

        private int mRequestCode = 0;
        private int mResultCode = RESULT_OK;
        private Context mContext = null;
        private Carrier mCarrier = null;
        private int mSubMask = 0;
        private String mConfigId = null;
        private Message mCurrentMsg = null;
        private QcRilHook mQcRilHook = null;
        private ActionCallback mCallback = null;

        private Handler mHandler = new Handler() {
            private static final int RETRY_DELAY_MILLIS = 100;
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_READY_CONFIG:
                        if (mReady && mQcRilHook != null) {
                            // Register the receiver to handle the result.
                            IntentFilter filter = new IntentFilter();
                            filter.addAction(ACTION_PDC_DATA_RECEIVED);
                            filter.addAction(ACTION_PDC_CONFIGS_CLEARED);
                            mContext.registerReceiver(mReceiver, filter);

                            Message newMsg = new Message();
                            newMsg.what = MSG_DEACTIVATE_CONFIG;
                            mHandler.sendMessage(newMsg);
                        } else {
                            sendMessageDelayed(msg, RETRY_DELAY_MILLIS);
                        }
                        break;
                    case MSG_DEACTIVATE_CONFIG:
                        if (mReady && mQcRilHook != null) {
                            TelephonyManager telephonyManager = (TelephonyManager) mContext.
                                    getSystemService(Context.TELEPHONY_SERVICE);
                            int phoneCount = telephonyManager.getPhoneCount();
                            switch (mRequestCode) {
                                case REQUEST_UPDATE_ROW_NV_ITEMS:
                                    for (int i = 0; i < phoneCount; i++) {
                                        String currentConfigId = mQcRilHook.qcRilGetConfig(
                                                SubscriptionManager.getPhoneId(
                                                SubscriptionManager.getSubId(i)[0]));
                                        Log.d(TAG, "currentConfigId=" + currentConfigId);
                                        if ((!TextUtils.isEmpty(currentConfigId))
                                                && currentConfigId.startsWith(MBN_CONFIG_ID_PREFIX)) {
                                            if (mCarrier.getROWMBNFileName() != null) {
                                                boolean result = mQcRilHook.qcRilDeactivateConfigs();
                                                Log.d(TAG, "Deactivate result: result = " + result);
                                                sendSetConfigMessage();
                                            } else {
                                                Log.d(TAG, "No ROW mbn file. So do not deactivate.");
                                                mResultCode = RESULT_NO_MBN;
                                                mHandler.sendEmptyMessage(MSG_ACTION_FINISHED);
                                            }
                                            return;
                                        }
                                    }
                                    Log.d(TAG, "No active regional mbn. So do nothing.");
                                    mResultCode = RESULT_NO_MBN;
                                    mHandler.sendEmptyMessage(MSG_ACTION_FINISHED);
                                    break;
                                case REQUEST_UPDATE_NV_ITEMS:
                                    for (int i = 0; i < phoneCount; i++) {
                                        String currentConfigId = mQcRilHook.qcRilGetConfig(
                                                SubscriptionManager.getPhoneId(
                                                SubscriptionManager.getSubId(i)[0]));
                                        Log.d(TAG, "currentConfigId=" + currentConfigId);
                                        if ((!TextUtils.isEmpty(currentConfigId))
                                                && currentConfigId.startsWith(MBN_CONFIG_ID_PREFIX)) {
                                            boolean result = mQcRilHook.qcRilDeactivateConfigs();
                                            Log.d(TAG, "Deactivate result: result = " + result);
                                            break;
                                        }
                                    }
                                    sendSetConfigMessage();
                                    break;
                            }
                        } else {
                            sendMessageDelayed(msg, RETRY_DELAY_MILLIS);
                        }
                        break;
                    case MSG_SET_CONFIG:
                        if (mReady && mQcRilHook != null) {
                            String name = (String) msg.obj;
                            mCurActionMode = MODE_SET;
                            if (!TextUtils.isEmpty(name)) {
                                // If the path is null, we will clean the configure.
                                Log.d(TAG, "Try to set the configuration. MBN file path:" + name);
                                Log.d(TAG, "mSubMask=" + mSubMask);
                                String[] availableConfigs =
                                        mQcRilHook.qcRilGetAvailableConfigs(null);
                                boolean hasLoad = false;
                                if ((availableConfigs != null) && (availableConfigs.length > 0)) {
                                    for (String config : availableConfigs) {
                                        if (mConfigId.equals(config)) {
                                            hasLoad = true;
                                            break;
                                        }
                                    }
                                }
                                if (hasLoad) {
                                    Log.d(TAG, mConfigId + " has been loaded before.");
                                    boolean result = mQcRilHook.qcRilSelectConfig(mConfigId,
                                            mSubMask);
                                    Log.d(TAG, "Select result: result = " + result);
                                } else {
                                    boolean result = mQcRilHook.qcRilSetConfig(name,
                                            mConfigId, mSubMask);
                                    Log.d(TAG, "Set result: result = " + result);
                                }
                            } else {
                                Log.d(TAG, "No mbn files.");
                                mResultCode = RESULT_NO_MBN;
                                mHandler.sendEmptyMessage(MSG_ACTION_FINISHED);
                            }
                        } else {
                            sendMessageDelayed(msg, RETRY_DELAY_MILLIS);
                        }
                        break;
                    case MSG_ACTION_TIMEOUT:
                        Log.w(TAG, "This action is timeout.");
                    case MSG_ACTION_FINISHED:
                        Log.d(TAG, "Update action finished.");
                        if (mCurActionMode != MODE_NONE) {
                            // Unregister the receiver as the action finished.
                            mContext.unregisterReceiver(mReceiver);
                            // Reset the value.
                            mCurActionMode = MODE_NONE;
                        }

                        // Remove all message.
                        removeMessages(MSG_SET_CONFIG);
                        removeMessages(MSG_ACTION_TIMEOUT);
                        removeMessages(MSG_ACTION_FINISHED);

                        // Reset the value.
                        mRetryTimes = 0;

                        if (mCallback != null) {
                            if (mResultCode == RESULT_OK) {
                                mCallback.onActionFinished(mRequestCode);
                            } else {
                                mCallback.onActionError(mRequestCode, mResultCode, null);
                            }
                        }

                        if (mQcRilHook != null) {
                            mQcRilHook.dispose();
                            mQcRilHook = null;
                        }
                        break;
                }
            }
        };

        private BroadcastReceiver mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                Log.i(TAG, "Receive the action: " + action);

                int actionMode = MODE_NONE;
                if (ACTION_PDC_CONFIGS_CLEARED.equals(action)) {
                    actionMode = MODE_DEACTIVATE;
                } else if (ACTION_PDC_DATA_RECEIVED.equals(action)) {
                    actionMode = MODE_SET;
                }

                if (actionMode != mCurActionMode) {
                    // We need wait for the current action finish.
                    return;
                }

                byte[] activeData = intent.getByteArrayExtra(EXTRA_PDC_ACTIVE);
                if (activeData == null) {
                    Log.e(TAG, "The action didn't contains the active data!");
                    mHandler.sendEmptyMessage(MSG_ACTION_FINISHED);
                    return;
                }

                ByteBuffer payload = ByteBuffer.wrap(activeData);
                payload.order(ByteOrder.nativeOrder());
                int res = payload.get();
                Log.i(TAG, "Get the payload's result = " + res);

                switch (res) {
                    case RESULT_SUCCESS:
                    case RESULT_ALREADY_CLEAN:
                        if (actionMode == MODE_DEACTIVATE) {
                            sendSetConfigMessage();
                            return;
                        } else if (actionMode == MODE_SET) {
                            // It means mbn set successfully. so send finished message.
                            Log.i(TAG, "Set the MBN file successfully");
                        }
                        break;
                    case RESULT_FAILURE:
                    case RESULT_NEED_CLEAN:
                        if (mCurrentMsg != null && mRetryTimes <= MAX_RETRY_TIMES) {
                            Message msg = new Message();
                            msg.what = mCurrentMsg.what;
                            msg.obj= mCurrentMsg.obj;
                            mHandler.sendMessage(msg);
                            mRetryTimes = mRetryTimes + 1;
                            return;
                        } else {
                            // TODO: We don't handle the error message now. If we need
                            // handle the error message, we could call the callback.
                        }
                        // Will send the finished message.
                        break;
                    default:
                        Log.w(TAG, "We do not handle this result.");
                        // Will send the finished message.
                        break;
                }
                mHandler.sendEmptyMessage(MSG_ACTION_FINISHED);
            }
        };

        public UpdateNVItemsTask(Context context, int requestCode, ActionCallback callback,
                int subMask, String configId) {
            mContext = context;
            mRequestCode = requestCode;
            mCallback = callback;
            mSubMask = subMask;
            switch (mRequestCode) {
                case REQUEST_UPDATE_NV_ITEMS:
                    mConfigId = MBN_CONFIG_ID_PREFIX + configId;
                    break;
                case REQUEST_UPDATE_ROW_NV_ITEMS:
                    mConfigId = ROW_MBN_CONFIG_ID_PREFIX + configId;
                    break;
            }

            if (mQcRilHook == null) {
                mQcRilHook = new QcRilHook(mContext, new QcRilHookCallback() {
                    @Override
                    public void onQcRilHookReady() {
                        if (Utils.DEBUG) Log.i(TAG, "The qcrilhook has been ready.");
                        mReady = true;
                    }
                    @Override
                    public void onQcRilHookDisconnected() {
                        if (Utils.DEBUG) Log.i(TAG, "The qcrilhook has been disconnected.");
                        mReady = false;
                    }
                });
            }
        }

        public UpdateNVItemsTask(Context context, int requestCode, ActionCallback callback) {
            this(context, requestCode, callback, 0, "");
        }

        @Override
        protected void finalize() throws Throwable {
            super.finalize();
            if (mQcRilHook != null) {
                mQcRilHook.dispose();
                mQcRilHook = null;
            }
        }

        @Override
        protected Void doInBackground(Carrier... params) {
            mCarrier = params[0];

            mCurrentMsg = new Message();
            mCurrentMsg.what = MSG_READY_CONFIG;
            mHandler.sendMessage(mCurrentMsg);

            // Send the timeout message as delayed.
            mHandler.sendEmptyMessageDelayed(MSG_ACTION_TIMEOUT, TIMEOUT_MILLIS);
            return null;
        }

        private void sendSetConfigMessage() {
            String mbnFileName = null;
            switch (mRequestCode) {
                case REQUEST_UPDATE_NV_ITEMS:
                    mbnFileName = mCarrier.getMBNFileName();
                    break;
                case REQUEST_UPDATE_ROW_NV_ITEMS:
                    mbnFileName = mCarrier.getROWMBNFileName();
                    break;
                default:
                    break;
            }
            Message newMsg = new Message();
            newMsg.what = MSG_SET_CONFIG;
            newMsg.obj= mbnFileName;
            mHandler.sendMessage(newMsg);
        }
    }

    /**
     * This task used to start the switch action.
     */
    public static class SwitchCarrierTask extends AsyncTask<SwitchData, Void, Void> {
        private static final String TAG = "SwitchCarrierTask";

        // The string used by switch action.
        private static final String ACTION_FILE_NAME    = "action";
        private static final String SWITCH_FILE_NAME    = "ota.zip";
        private static final String NEW_CARRIER_STR_PRE = "newCarrier";
        private static final String NEW_PACK_COUNT_STR_PRE = "newPackCount=";

        private Context mContext = null;
        private int mRequestCode = 0;
        private ActionCallback mCallback = null;
        private ICarrierAccessCacheService mAccessCacheService;

        public SwitchCarrierTask(Context context, int requestCode, ActionCallback callback,
                ICarrierAccessCacheService service) {
            mContext = context;
            mCallback = callback;
            mRequestCode = requestCode;
            mAccessCacheService = service;
        }

        @Override
        protected Void doInBackground(SwitchData... params) {
            try {
                if (params.length > 1)
                    switchToNewCarrier(mContext, params[0], params[1]);
                else switchToNewCarrier(mContext, params[0], null);
            } catch (IOException e) {
                Log.e(TAG, "Something error when switch the carrier, ex = " + e.getMessage());
                if (mCallback != null) mCallback.onActionError(mRequestCode, RESULT_FAILURE, e);
            }
            if (mCallback != null) mCallback.onActionFinished(mRequestCode);
            return null;
        }

        private void switchToNewCarrier(Context context, SwitchData data, SwitchData data2) throws IOException {
            if (data == null) throw new IOException("Switch data is null.");
            Log.i(TAG, "Try to switch to new carrier: " + data._name);
            Log.i(TAG, "Or try to switch with this file: " + data._path);

            // Try to use the given new carrier path to switch the carrier.
            if (data2 != null && !TextUtils.isEmpty(data2._path)) {
                Log.d(TAG,"write in action file path=" + data2._path);
                boolean result = false;
                try {
                    String contents = NEW_CARRIER_STR_PRE + "=" + data2._path + "\n";
                    result = mAccessCacheService.writeActionFile(contents);
                } catch (RemoteException e) {
                    Log.e(TAG, "write the action file error.");
                    result = false;
                }
                if (!result) throw new IOException("write action file failed.");
            }

            // Try to use the given new carrier path to switch the carrier.
            if (!TextUtils.isEmpty(data._path)) {
                Log.i(TAG,"installPackage filepath="+data._path);
                installPackage(context, data._path);
            }
        }

        private void installPackage(Context context, String filePath) throws IOException {

            final String filepathStr = "--update_package=" + filePath;
            final String localeStr = "--locale=" + Locale.getDefault().toString();
            boolean result = false;
            try {
                result = mAccessCacheService.writeCommandFile(filepathStr, localeStr);
            } catch (RemoteException e) {
                Log.e(TAG, "write the command file error.");
                result = false;
            }
            if (!result) throw new IOException("write command file failed.");

            PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            pm.reboot(PowerManager.REBOOT_RECOVERY);

            throw new IOException("Reboot failed (no permissions?)");
        }
    }
}
