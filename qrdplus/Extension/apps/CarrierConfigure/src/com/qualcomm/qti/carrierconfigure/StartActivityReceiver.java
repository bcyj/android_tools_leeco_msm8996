/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import static com.android.internal.telephony.TelephonyIntents.SECRET_CODE_ACTION;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

import com.qualcomm.qti.carrierconfigure.Actions.SwitchCarrierTask;
import com.qualcomm.qti.carrierconfigure.Carrier.CarriersStorage;
import com.qualcomm.qti.carrierconfigure.Carrier.SwitchData;

import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.qti.accesscache.ICarrierAccessCacheService;
import com.qualcomm.qti.carrierconfigure.Actions.ActionCallback;
import com.qualcomm.qti.carrierconfigure.Actions.SwitchCarrierTask;
import com.qualcomm.qti.carrierconfigure.Actions.UpdateNVItemsTask;
import com.qualcomm.qti.carrierconfigure.Carrier.EmptyPathException;
import com.qualcomm.qti.carrierconfigure.Carrier.NullServiceException;
import com.qualcomm.qti.carrierconfigure.Carrier.SwitchData;
import com.qualcomm.qti.carrierconfigure.Utils;
import com.qualcomm.qti.loadcarrier.ILoadCarrierService;

import java.util.ArrayList;
import java.util.HashMap;

public class StartActivityReceiver extends BroadcastReceiver {
    private static final String TAG = "StartActivityReceiver";
    private static final String ACTION_PHONE_READY = "com.android.phone.ACTION_PHONE_READY";

    private static final String CODE_SPEC_SWITCH = "2277437234";

    public static final String CODE_SPEC_SWITCH_L = "73446625234";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (SECRET_CODE_ACTION.equals(intent.getAction())) {
            if (Utils.DEBUG) {
                Log.i(TAG, "Try to start QRD feature settings activity.");
            }

            String host = intent.getData() != null ? intent.getData().getHost() : null;
            if (CODE_SPEC_SWITCH.equals(host)) {
                Intent i = new Intent();
                i.setAction(Intent.ACTION_MAIN);
                i.setClass(context, ConfigurationActivity.class);
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(i);
            } else if (CODE_SPEC_SWITCH_L.equals(host)) {
                Intent i = new Intent();
                i.setAction(Intent.ACTION_MAIN);
                i.setClass(context, ConfigurationActivity.class);
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                i.addCategory(CODE_SPEC_SWITCH_L);
                context.startActivity(i);
            }
        } else if (Utils.ACTION_TRIGGER.equals(intent.getAction())) {
            Log.d(TAG, "ACTION_TRIGGER received. Time="+System.currentTimeMillis());
            // Get the carriers from the intent.
            Bundle bundle = intent.getExtras();
            ArrayList<String> pathList =
                    bundle == null ? null : bundle.getStringArrayList(Utils.EXTRA_PATH_LIST);
            if (pathList == null || pathList.size() < 1) return;

            // Get the do not triggered carriers.
            HashMap<String, String> carriers = new HashMap<String, String>();
            for (String path : pathList) {
                String contents = bundle.getString(path);
                carriers.put(path, contents);
            }
            CarriersStorage storage = new CarriersStorage();
            Carrier.parseCarriers(carriers, storage);

            String currentCarrierName = Carrier.getCurrentCarriersName(Carrier.getCurrentCarriers());
            Log.d(TAG, "currentCarrierName="+currentCarrierName);
            ArrayList<Carrier> triggeredCarriers = new ArrayList<Carrier>();
            ArrayList<String> notTriggeredCarriers = Carrier.getNotTriggeredCarriersName();
            for (Carrier carrier : storage.mListCarrier) {
                if (currentCarrierName.equals(carrier.mName)
                        || notTriggeredCarriers == null
                        || notTriggeredCarriers.size() < 1
                        || !notTriggeredCarriers.contains(carrier.mName)) {
                    triggeredCarriers.add(carrier.findLatestCarrier());
                }
            }

            // If the triggered carrier list size is 0, do nothing.
            if (triggeredCarriers.size() == 0) {
                Log.i(TAG, "There isn't any carrier need triggered, do nothing.");
                return;
            }

            if (triggeredCarriers.size() == 1) {
                // If triggered carriers is same with current carriers do nothing.
                if (triggeredCarriers.get(0).mName.equals(currentCarrierName)) {
                    Log.i(TAG, "Trigger carrier is same with current carrier do nothing.");
                    return;
                }
                if (triggeredCarriers.get(0).mName.endsWith(Utils.CARRIER_TO_DEFAULT_NAME)) {
                    Log.i(TAG, "Trigger carrier is only Default package.");
                    return;
                }
                if (triggeredCarriers.get(0).mName.equals(Utils.SPECIAL_ROW_PACKAGE_NAME)) {
                    startSpecialInstallService(context, triggeredCarriers);
                    context.stopService(getServiceIntent(context));
                    return;
                }
            }

            if (triggeredCarriers.size() == 2) {
                if ((triggeredCarriers.get(0).mName.equals(currentCarrierName)
                        && triggeredCarriers.get(1).mName.equals(currentCarrierName
                                + Utils.CARRIER_TO_DEFAULT_NAME))
                        || (triggeredCarriers.get(0).mName.equals(currentCarrierName
                                + Utils.CARRIER_TO_DEFAULT_NAME)
                        && triggeredCarriers.get(1).mName.equals(currentCarrierName))) {
                    Log.i(TAG, "Trigger carrier is same with current carrier, do nothing2");
                    return;
                }
                if ((triggeredCarriers.get(0).mName.equals(Utils.SPECIAL_ROW_PACKAGE_NAME)
                        && triggeredCarriers.get(1).mName.equals(currentCarrierName
                                + Utils.CARRIER_TO_DEFAULT_NAME))
                        || (triggeredCarriers.get(0).mName.equals(currentCarrierName
                                + Utils.CARRIER_TO_DEFAULT_NAME)
                        && triggeredCarriers.get(1).mName.equals(Utils.SPECIAL_ROW_PACKAGE_NAME))) {
                    startSpecialInstallService(context, triggeredCarriers);
                    context.stopService(getServiceIntent(context));
                    return;
                }
            }

            if (!currentCarrierName.equals("Default")) {
                boolean has2DefaultPackage = false;
                for (Carrier carrier : triggeredCarriers) {
                    if (carrier.mName.equals(currentCarrierName + Utils.CARRIER_TO_DEFAULT_NAME)) {
                        has2DefaultPackage = true;
                        break;
                    }
                }
                if (!has2DefaultPackage) {
                    Log.i(TAG, "Need " + currentCarrierName + Utils.CARRIER_TO_DEFAULT_NAME);
                    return;
                }
            }

            Intent i = new Intent();
            i.setAction(Utils.ACTION_TRIGGER_WELCOME);
            i.setClass(context, ConfigurationActivity.class);
            i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            i.putParcelableArrayListExtra(Utils.EXTRA_CARRIER_LIST, triggeredCarriers);
            context.startActivity(i);
            context.stopService(getServiceIntent(context));
        } else if (ACTION_PHONE_READY.equals(intent.getAction())) {
            context.startService(getServiceIntent(context));
        }
    }

    private void startSpecialInstallService(Context context, ArrayList<Carrier> triggeredCarriers) {
        Log.i(TAG, "Start TriggerSpecialInstallService.");
        Intent intent = new Intent();
        intent.setClass(context, TriggerSpecialInstallService.class);
        intent.putParcelableArrayListExtra(Utils.EXTRA_CARRIER_LIST, triggeredCarriers);
        context.startService(intent);
    }

    private Intent getServiceIntent(Context context) {
        Intent registTriggerIntent = new Intent();
        registTriggerIntent.setClass(context, RegistTriggerService.class);
        return registTriggerIntent;
    }

    public static class RegistTriggerService extends Service {
        private final static String TAG = "RegistTriggerService";

        private StartActivityReceiver mReceiver = null;
        @Override
        public IBinder onBind(Intent intent) {
            return null;
        }

        @Override
        public void onCreate() {
            super.onCreate();
            IntentFilter filter = new IntentFilter();
            filter.addAction(Utils.ACTION_TRIGGER);
            filter.setPriority(1000);
            mReceiver = new StartActivityReceiver();
            registerReceiver(mReceiver, filter);
            Log.i(TAG,"register StartActivityReceiver");
        }

        @Override
        public void onDestroy() {
            if (mReceiver != null) {
                unregisterReceiver(mReceiver);
                mReceiver = null;
            }
            Log.i(TAG,"unregister StartActivityReceiver");
            super.onDestroy();
        }
    }

    public static class TriggerSpecialInstallService extends Service implements ActionCallback,
            OnClickListener {
        private static final String TAG = "TriggerSpecialInstallService";

        private static final String RE_MNC = "(\\d{2,3})?";
        private static final String SEP_COMMA = ",";

        private static final int RETRY_DELAY = 500;
        private static final int RETRY_COUNT = 10;

        private static final int MSG_UPDATE_NV_ITEMS = 0;
        private static final int MSG_SHOW_ALERT_DIALOG = MSG_UPDATE_NV_ITEMS + 1;
        private static final int MSG_SHOW_NOTICE_DIALOG = MSG_SHOW_ALERT_DIALOG + 1;
        private static final int MSG_START_SWITCH = MSG_SHOW_NOTICE_DIALOG + 1;
        private static final int MSG_SWITCH_ERROR = MSG_START_SWITCH + 1;
        private static final int MSG_FINISH = MSG_SWITCH_ERROR + 1;

        private static final int DIALOG_ALERT = 1;
        private static final int DIALOG_NOTICE = 2;

        private int mDialogId = 0;
        private int mSubMask = 0;
        private String mConfigId = null;

        private int mRetryTimes = 0;
        private Carrier mSwitchCarrier = null;
        private Carrier mSwitchToDefaultCarrier = null;
        private ArrayList<Carrier> mCarriers;

        private ILoadCarrierService mService = null;
        private ICarrierAccessCacheService mAccessCacheService = null;
        private ServiceConnection mServiceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                if (Utils.DEBUG) Log.i(TAG, "Service Connected to " + name.getShortClassName());
                if (name.getShortClassName().equals(".LoadCarrierService")) {
                    mService = ILoadCarrierService.Stub.asInterface(service);
                } else if (name.getShortClassName().equals(".CarrierAccessCacheService")) {
                    mAccessCacheService = ICarrierAccessCacheService.Stub.asInterface(service);
                }
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {
                if (name.getShortClassName().equals(".LoadCarrierService")) {
                    mService = null;
                } else if (name.getShortClassName().equals(".CarrierAccessCacheService")) {
                    mAccessCacheService = null;
                }
            }
        };

        private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_UPDATE_NV_ITEMS:
                        if (Utils.DEBUG) Log.d(TAG, "Update NV Items start.");
                        // Before start the switch action, need update the NV Items first. And after
                        // the update action finished, will start the switch action.
                        findActiveSubMask(mSwitchCarrier);
                        updateNVItems();
                        break;
                    case MSG_SHOW_ALERT_DIALOG:
                        showAlertDialog();
                        break;
                    case MSG_SHOW_NOTICE_DIALOG:
                        showNoticeDialog();
                        break;
                    case MSG_START_SWITCH:
                        if (Utils.DEBUG) Log.d(TAG, "Switch action start.");
                        startSwitchAction();
                        break;
                    case MSG_SWITCH_ERROR:
                        Toast.makeText(TriggerSpecialInstallService.this,
                                R.string.alert_switch_error, Toast.LENGTH_LONG).show();
                        break;
                    case MSG_FINISH:
                        if (Utils.DEBUG) Log.d(TAG, "TriggerSpecialInstallService finished.");
                        if (mServiceConnection != null) {
                            unbindService(mServiceConnection);
                        }
                        stopSelf();
                        break;
                    default:
                        break;
                }
            }
        };

        @Override
        public void onCreate() {
            super.onCreate();

            // Try to bind the service.
            if (mServiceConnection != null) {
                if (mService == null) {
                    // Bind the service to get the carriers stored in SD card.
                    Intent intent = new Intent(ILoadCarrierService.class.getName());
                    intent.setPackage(ILoadCarrierService.class.getPackage().getName());
                    bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
                }
                if (mAccessCacheService == null) {
                    // Bind the service to access cache dir.
                    Intent intent = new Intent(ICarrierAccessCacheService.class.getName());
                    intent.setPackage(ICarrierAccessCacheService.class.getPackage().getName());
                    bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
                }
            }

        }

        @Override
        public void onStart(Intent intent, int startId) {
            super.onStart(intent, startId);
            Log.d(TAG, "TriggerSpecialInstallService start.");
            mCarriers = intent.getParcelableArrayListExtra(Utils.EXTRA_CARRIER_LIST);
            if (mCarriers == null || mCarriers.size() < 1) {
                Log.d(TAG, "No carrier. Do nothing.");
                mHandler.sendEmptyMessage(MSG_FINISH);
                return;
            } else if ((mCarriers.size() == 1) && (!mCarriers.get(0).mName.endsWith(
                    Utils.CARRIER_TO_DEFAULT_NAME))) {
                mSwitchCarrier = mCarriers.get(0);
                Log.d(TAG, "mSwitchCarrier = " + mSwitchCarrier);
            } else if (mCarriers.size() == 2) {
                for (Carrier carrier : mCarriers) {
                    Log.d(TAG, "carrier.mName=" + carrier.mName);
                    if (carrier.mName.endsWith(Utils.CARRIER_TO_DEFAULT_NAME)) {
                        mSwitchToDefaultCarrier = carrier;
                    } else {
                        mSwitchCarrier = carrier;
                    }
                }
                if ((mSwitchCarrier == null) || (mSwitchToDefaultCarrier == null)) {
                    Log.d(TAG, "mSwitchCarrier = " + mSwitchCarrier
                            + "mSwitchToDefaultCarrier = " + mSwitchToDefaultCarrier);
                    mHandler.sendEmptyMessage(MSG_FINISH);
                    return;
                }
            } else if (mCarriers.size() > 2) {
                Log.d(TAG, "Too many carriers. Do nothing now.");
                mHandler.sendEmptyMessage(MSG_FINISH);
                return;
            } else {
                Log.d(TAG, "Wrong Carriers.");
                mHandler.sendEmptyMessage(MSG_FINISH);
                return;
            }

            if (mSwitchCarrier != null) {
                mHandler.sendEmptyMessage(MSG_UPDATE_NV_ITEMS);
            } else {
                Log.d(TAG, "No target carrier. Do nothing");
                mHandler.sendEmptyMessage(MSG_FINISH);
            }
        }

        @Override
        public IBinder onBind(Intent intent) {
            return null;
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
        }

        @Override
        public void onActionFinished(int requestCode) {
            switch (requestCode) {
                case Actions.REQUEST_UPDATE_NV_ITEMS:
                    if (Utils.DEBUG) Log.d(TAG, "Update NV items finished.");
                    Carrier.saveAsNotTriggeredCarrier(mCarriers);
                    String mode = mSwitchCarrier.getMode();
                    if (mode.equals(Carrier.ITEM_MODE_AP_MP)) {
                        mHandler.sendEmptyMessage(MSG_SHOW_ALERT_DIALOG);
                    } else {
                        mHandler.sendEmptyMessage(MSG_SHOW_NOTICE_DIALOG);
                    }
                    break;
                case Actions.REQUEST_UPDATE_ROW_NV_ITEMS:
                    // Do nothing now.
                    break;
                case Actions.REQUEST_SWITCH_CARRIER:
                    if (Utils.DEBUG) Log.d(TAG, "Switch action finished.");
                    mHandler.sendEmptyMessage(MSG_FINISH);
                    break;
            }
        }

        @Override
        public void onActionError(int requestCode, int resultCode, Exception ex) {
            switch (requestCode) {
                case Actions.REQUEST_UPDATE_NV_ITEMS:
                    if (resultCode == Actions.RESULT_NO_MBN) {
                        Log.d(TAG, "No mbn files.");
                        Carrier.saveAsNotTriggeredCarrier(mCarriers);
                        String mode = mSwitchCarrier.getMode();
                        if (mode.equals(Carrier.ITEM_MODE_AP)) {
                            mHandler.sendEmptyMessage(MSG_SHOW_ALERT_DIALOG);
                            break;
                        }
                    }
                    Log.e(TAG, "Error when updating NV items");
                    mHandler.sendEmptyMessage(MSG_FINISH);
                    break;
                case Actions.REQUEST_UPDATE_ROW_NV_ITEMS:
                    // Do nothing now.
                    break;
                default:
                    Log.e(TAG, "Get the request[ " + requestCode + "] error, ex is "
                            + ex.getMessage());
                    mHandler.sendEmptyMessage(MSG_SWITCH_ERROR);
                    break;
            }
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            switch (mDialogId) {
                case DIALOG_ALERT:
                    switch (which) {
                        case DialogInterface.BUTTON_POSITIVE:
                            // For user press OK, then try to switch the carrier.
                            mHandler.sendEmptyMessage(MSG_START_SWITCH);
                            break;
                        case DialogInterface.BUTTON_NEGATIVE:
                            mHandler.sendEmptyMessage(MSG_FINISH);
                            break;
                    }
                    break;
                case DIALOG_NOTICE:
                    mHandler.sendEmptyMessage(MSG_FINISH);
                    break;
            }
        }

        private void findActiveSubMask(Carrier carrier) {
            TelephonyManager telephonyManager = (TelephonyManager) getSystemService(
                    Context.TELEPHONY_SERVICE);
            int phoneCount = telephonyManager.getPhoneCount();
            mSubMask = 0;
            for (int i = 0; i < phoneCount; i++) {
                String mccMnc = telephonyManager.getSimOperator(SubscriptionManager.getSubId(i)[0]);
                Log.d(TAG, "mccmnc=" + mccMnc);
                if (!TextUtils.isEmpty(mccMnc)) {
                    mSubMask = mSubMask | ( 0x0001 << i);
                }
            }
            mConfigId = carrier.mName;
        }

        private void showNoticeDialog() {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(R.string.notice_row_title)
                    .setMessage(R.string.notice_row_text)
                    .setPositiveButton(android.R.string.ok, this);

            AlertDialog dialog = builder.create();
            dialog.setCanceledOnTouchOutside(false);
            dialog.getWindow().setType((WindowManager.LayoutParams.TYPE_SYSTEM_ALERT));
            dialog.show();

            mDialogId = DIALOG_NOTICE;
        }

        private void showAlertDialog() {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            builder.setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(R.string.notice_special_install_title)
                    .setMessage(R.string.notice_special_install_text)
                    .setPositiveButton(android.R.string.ok, this)
                    .setNegativeButton(android.R.string.cancel, this);

            AlertDialog dialog = builder.create();
            dialog.setCanceledOnTouchOutside(false);
            dialog.getWindow().setType((WindowManager.LayoutParams.TYPE_SYSTEM_ALERT));
            dialog.show();

            mDialogId = DIALOG_ALERT;
        }

        private void startSwitchAction() {
            try {
                SwitchData data = mSwitchCarrier.getSwitchData(mService);
                SwitchCarrierTask switchTask = new SwitchCarrierTask(this,
                        Actions.REQUEST_SWITCH_CARRIER, this, mAccessCacheService);
                if (mSwitchToDefaultCarrier != null && !mSwitchCarrier.getBaseCarrierName()
                        .equals(Carrier.getCurrentCarriersName(Carrier.getCurrentCarriers())))
                    switchTask.execute(mSwitchToDefaultCarrier.getSwitchData(mService), data);
                else switchTask.execute(data);
            } catch (NullServiceException ex) {
                Log.e(TAG, "Catch the NullServiceException: " + ex.getMessage());
                if (mRetryTimes <= RETRY_COUNT) {
                    mRetryTimes = mRetryTimes + 1;
                    mHandler.sendEmptyMessageDelayed(MSG_START_SWITCH, RETRY_DELAY);
                } else {
                    Log.e(TAG, "Already couldn't get the service, please check the status.");
                    mHandler.sendEmptyMessage(MSG_SWITCH_ERROR);
                }
            } catch (EmptyPathException ex) {
                Log.e(TAG, "Catch the EmptyPathException: " + ex.getMessage());
                // There is some error when we get the switch intent.
                // Send the switch error message, .
                mHandler.sendEmptyMessage(MSG_SWITCH_ERROR);
            }
        }

        private void updateNVItems() {
            // Get the intent to start the action service to handle the switch action.
            UpdateNVItemsTask updateTask = new UpdateNVItemsTask(this,
                    Actions.REQUEST_UPDATE_NV_ITEMS, this, mSubMask, mConfigId);
            updateTask.execute(mSwitchCarrier);
        }

        private void updateROWNVItems() {
            UpdateNVItemsTask updateTask = new UpdateNVItemsTask(this,
                    Actions.REQUEST_UPDATE_ROW_NV_ITEMS, this, mSubMask, mConfigId);
            updateTask.execute(mSwitchCarrier);
        }
    }
}
