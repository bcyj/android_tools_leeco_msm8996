/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import java.util.ArrayList;

import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.Loader;
import android.content.Loader.OnLoadCompleteListener;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.qualcomm.qti.accesscache.ICarrierAccessCacheService;
import com.qualcomm.qti.carrierconfigure.Actions.ActionCallback;
import com.qualcomm.qti.carrierconfigure.Actions.SwitchCarrierTask;
import com.qualcomm.qti.carrierconfigure.Actions.UpdateNVItemsTask;
import com.qualcomm.qti.carrierconfigure.Carrier.CarriersStorage;
import com.qualcomm.qti.carrierconfigure.Carrier.SwitchData;
import com.qualcomm.qti.carrierconfigure.Utils.MyAlertDialog;
import com.qualcomm.qti.carrierconfigure.Utils.WaitDialog;
import com.qualcomm.qti.loadcarrier.ILoadCarrierService;

public class CarrierConfigFragment extends RadioPreferenceFragment
        implements MyAlertDialog.OnAlertDialogButtonClick, ActionCallback {
    private static final String TAG = "CarrierConfigFragment";

    private static final int MSG_START_SWITCH = MSG_BASE + 1;

    private static final int REQUEST_UPDATE_NV_ITEMS = 0;
    private static final int REQUEST_SWITCH_CARRIER = 1;

    private CarriersLoader mLoader = null;
    private Carrier mSwitchToDefaultCarrier = null;
    private OnLoadCompleteListener<CarriersStorage> mLoaderListener =
            new OnLoadCompleteListener<CarriersStorage>() {
        @Override
        public void onLoadComplete(Loader<CarriersStorage> loader, CarriersStorage storage) {
            if (storage != null) {
                buildPreferences(storage);
            }
        }
    };

    private ILoadCarrierService mService = null;
    private ICarrierAccessCacheService mAccessCacheService = null;
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            if (Utils.DEBUG) Log.i(TAG, "Service Connected to LoadCarrierService" + name.getShortClassName());
            if (name.getShortClassName().equals(".LoadCarrierService")) {
                mService = ILoadCarrierService.Stub.asInterface(service);
                if (mLoader != null) {
                    mLoader.notifyCarrierLoadServiceChanged(mService);
                }
            } else if (name.getShortClassName().equals(".CarrierAccessCacheService")) {
                mAccessCacheService = ICarrierAccessCacheService.Stub.asInterface(service);
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            if (name.getShortClassName().equals(".LoadCarrierService")) {
                mService = null;
                if (mLoader != null) {
                    mLoader.notifyCarrierLoadServiceChanged(null);
                }
            } else if (name.getShortClassName().equals(".CarrierAccessCacheService")) {
                mAccessCacheService = null;
            }
        }
    };

    public CarrierConfigFragment() {
        super();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (mServiceConnection != null) {
            if (mService == null) {
                // Bind the service to get the carriers stored in SD card.
                Intent intent = new Intent(ILoadCarrierService.class.getName());
                intent.setPackage(ILoadCarrierService.class.getPackage().getName());
                getActivity().bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
            }
            if (mAccessCacheService == null) {
                // Bind the service to access cache dir.
                Intent intent = new Intent(ICarrierAccessCacheService.class.getName());
                intent.setPackage(ICarrierAccessCacheService.class.getPackage().getName());
                getActivity().bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
            }
        }

        mLoader = new CarriersLoader(getActivity(), mService);
        mLoader.registerListener(0, mLoaderListener);
        mLoader.startLoading();

        ArrayList<String> carriers = Carrier.getCurrentCarriers();
        mCurrentPreferenceKey = Carrier.getCurrentCarriersName(carriers);
        mSelectedPreferenceKey = mCurrentPreferenceKey;

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.carrier_config_preference);
    }

    @Override
    public void onResume() {
        super.onResume();
        mLoader.startLoading();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if (mLoader != null) {
            mLoader.unregisterListener(mLoaderListener);
            mLoader.stopLoading();
            mLoader.reset();
        }

        if (mServiceConnection != null) {
            getActivity().unbindService(mServiceConnection);
        }
    }

    @Override
    public void onAlertDialogButtonClick(int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                // For user press OK, then try to switch the carrier.
                sendEmptyMessage(MSG_START_SWITCH);
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                // For user press CANCEL, then reset the value and update the display.
                resetSelection();
                break;
        }
    }

    @Override
    protected void onSelectedChanged() {
        // Show the dialog to alert the user.
        MyAlertDialog dialog = MyAlertDialog.newInstance(this, R.string.alert_switch_title,
                R.string.alert_switch_text);
        dialog.show(getFragmentManager(), MyAlertDialog.TAG_LABEL);
    }

    @Override
    protected void handleMessage(Message msg) {
        if (msg.what == MSG_START_SWITCH) {
            if (Utils.DEBUG) Log.d(TAG, "Start switch! For user press yes.");
            // Show the "Please wait ..." dialog.
            WaitDialog wait = WaitDialog.newInstance();
            wait.show(getFragmentManager(), WaitDialog.TAG_LABEL);

            // Get the intent to start the action service to handle the switch action.
            Carrier info = (Carrier) mCategory.getCheckedPreference().getTag();
            UpdateNVItemsTask updateTask = new UpdateNVItemsTask(getActivity(),
                    REQUEST_UPDATE_NV_ITEMS, this);
            updateTask.execute(info);
            onActionFinished(REQUEST_UPDATE_NV_ITEMS);
        }
    }

    @Override
    public void onActionFinished(int requestCode) {
        switch (requestCode) {
            case REQUEST_UPDATE_NV_ITEMS:
                // After the update NV items action finished, start the switch action.
                try {
                    Carrier info = (Carrier) mCategory.getCheckedPreference().getTag();
                    SwitchData data = info.getSwitchData(mService);
                    SwitchCarrierTask switchTask = new SwitchCarrierTask(getActivity(),
                            REQUEST_SWITCH_CARRIER, this, mAccessCacheService);
                    if (mSwitchToDefaultCarrier == null
                            || info.getBaseCarrierName().equals(mCurrentPreferenceKey))
                        switchTask.execute(data);
                    else switchTask.execute(mSwitchToDefaultCarrier.getSwitchData(mService), data);
                } catch (IllegalArgumentException ex) {
                    Log.e(TAG, "Catch the IllegalArgumentException: " + ex.getMessage());
                    onActionError();
                }
                break;
            case REQUEST_SWITCH_CARRIER:
                getActivity().finish();
                break;
        }
    }

    @Override
    public void onActionError(int requestCode, int resultCode, Exception ex) {
        Log.e(TAG, "Get the request[ " + requestCode + "] error, ex is " + ex.getMessage());
        onActionError();
    }

    private void onActionError() {
        // There is some error when we get the switch intent, prompt one toast.
        ((WaitDialog) getFragmentManager().findFragmentByTag(WaitDialog.TAG_LABEL)).dismiss();
        Toast.makeText(getActivity(),R.string.alert_switch_error,Toast.LENGTH_LONG).show();
        // Reset the value and display.
        resetSelection();
    }

    private void buildPreferences(CarriersStorage storage) {
        if (Utils.DEBUG) Log.i(TAG, "Try to build the preferences.");

        if (storage == null || storage.isEmpty()) {
            Log.w(TAG, "Try to build the preferences, but couldn't find any carrier.");
            return;
        }

        if (mCategory == null) return;

        // Clear items.
        mCategory.removeAll();
        mSwitchToDefaultCarrier = null;

        travelCarrierStorage(storage, travelCarrierStorage(storage, mCurrentPreferenceKey));

        // Set the checked value.
        mCategory.setCheckedPreference(mSelectedPreferenceKey);
    }

    // add carrieries into mCategory by currentCarrier param
    private String travelCarrierStorage(CarriersStorage storage, String fullName) {
        if (fullName == null) return null;
         // Build the items with the cursor.
        for (Carrier carrier : storage.mListCarrier) {
            // Don't show Regional package in this list
            if (mSwitchToDefaultCarrier == null) {
                if (!TextUtils.isEmpty(carrier.getHierarchy()))
                    if (!(carrier.mName.equals(fullName)
                            || (carrier.getSwitchCarrierName().equals("Default")
                            && carrier.mType.equals(Carrier.TYPE_OTA_ZIP_FILE))))
                        continue;
                if (!Carrier.getSwitchCarrierName(fullName).equals(carrier.getBaseCarrierName())
                        && !fullName.equals(carrier.mName))
                    continue;
            } else {
                if (!TextUtils.isEmpty(carrier.getHierarchy())
                        || carrier.mType.equals(Carrier.TYPE_DIRECTORY)) {
                    continue;
                }
                if (!Carrier.getSwitchCarrierName(fullName).equals(carrier.getBaseCarrierName()))
                    continue;
            }
            if (mSwitchToDefaultCarrier != null && carrier.mName.equals("Default"))
                continue;
            // Create the preference for this carrier.
            RadioButtonPreference perference = new RadioButtonPreference(getActivity());
            perference.setKey(carrier.mName);
            perference.setTitle(carrier.getTopCarrierTitle());
            perference.setTag(carrier);

            // Add the preference to category.
            mCategory.addPreference(perference);

            if (mSwitchToDefaultCarrier == null && "Default".equals(carrier.getSwitchCarrierName())
                    && !fullName.equals("Default")) {
                mSwitchToDefaultCarrier = carrier;
            }
        }
        if (mSwitchToDefaultCarrier != null)
            return mSwitchToDefaultCarrier.mName;
        else return null;
    }
}
