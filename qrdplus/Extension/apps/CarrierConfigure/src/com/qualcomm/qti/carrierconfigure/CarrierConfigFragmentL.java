/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.Loader;
import android.content.Loader.OnLoadCompleteListener;
import android.content.ServiceConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

public class CarrierConfigFragmentL extends RadioPreferenceFragmentL
        implements MyAlertDialog.OnAlertDialogButtonClick, ActionCallback {
    private static final String TAG = "CarrierConfigFragmentL";

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
            if (Utils.DEBUG) Log.i(TAG, "Service Connected to " + name.getShortClassName());
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

    public CarrierConfigFragmentL() {
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

        mCurrentPreferenceKey = Carrier.getCurrentCarriers();
        Log.i(TAG,"mCurrentPreferenceKey = " + mCurrentPreferenceKey.size() + mCurrentPreferenceKey.get(0));

        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.carrier_config_preference_l);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Button beginConfig = (Button)getActivity().findViewById(R.id.config_begin_button);
        beginConfig.setVisibility(View.VISIBLE);
        beginConfig.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (getCheckedCarrierList().size() == 0 || getCheckedCarrierList().get(0).mName
                        .equals(Carrier.getCurrentCarriersName(mCurrentPreferenceKey))) {
                    Toast.makeText(getActivity(), R.string.alter_no_carrier_selected,
                            Toast.LENGTH_LONG).show();
                    return;
                }
                // Show the dialog to alert the user.
                MyAlertDialog dialog = MyAlertDialog.newInstance(CarrierConfigFragmentL.this,
                        R.string.alert_switch_title,
                        R.string.alert_switch_text);
                dialog.show(getFragmentManager(), MyAlertDialog.TAG_LABEL);
            }
        });
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
                mHandler.sendEmptyMessage(MSG_START_SWITCH);
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                // For user press CANCEL, then reset the value and update the display.
                resetSelection();
                break;
        }
    }

    @Override
    protected void onSelectedChanged() {
    }

    private ArrayList<Carrier> getCheckedCarrierList() {
        ArrayList<Carrier> list = new ArrayList<Carrier>();
        Iterator<Entry<String, RadioGroupPreferenceCategory>> iterator
                = mCategory.entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<String, RadioGroupPreferenceCategory> entry = iterator.next();
            if (!TextUtils.isEmpty(mSelectedPreferenceKey.get(TAG_HIERARCHY))) {
                RadioButtonPreference preference = entry.getValue().getCheckedPreference();
                if (preference != null) {
                    Carrier info = (Carrier)preference.getTag();
                    Log.i(TAG,"getCheckedCarrierList() info.mName="+info.mName);
                    list.add(info);
                }
            }
        }
        return list;
    }

    @Override
    protected void handleMessage(Message msg) {
        switch (msg.what) {
        case MSG_START_SWITCH:
            if (Utils.DEBUG)
                Log.d(TAG, "Start switch! For user press yes.");
            // Show the "Please wait ..." dialog.
            WaitDialog wait = WaitDialog.newInstance();
            wait.show(getFragmentManager(), WaitDialog.TAG_LABEL);

            // Get the intent to start the action service to handle the switch action.
            for (Carrier info : getCheckedCarrierList()) {
                Log.i(TAG,"handleMessage() = " + info.mName + info.getHierarchy());
                UpdateNVItemsTask updateTask = new UpdateNVItemsTask(getActivity(),
                        REQUEST_UPDATE_NV_ITEMS, this);
                updateTask.execute(info);
            }
            onActionFinished(REQUEST_UPDATE_NV_ITEMS);
            break;
        }
    }

    @Override
    public void onActionFinished(int requestCode) {
        switch (requestCode) {
            case REQUEST_UPDATE_NV_ITEMS:
                // After the update NV items action finished, start the switch action.
                try {
                    Carrier info = getCheckedCarrierList().get(0);
                    SwitchData data = info.getSwitchData(mService);
                    SwitchCarrierTask switchTask = new SwitchCarrierTask(getActivity(),
                            REQUEST_SWITCH_CARRIER, this, mAccessCacheService);
                    if (mSwitchToDefaultCarrier == null || info.getBaseCarrierName()
                            .equals(Carrier.getCurrentCarriersName(mCurrentPreferenceKey)))
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

        // clear items
        mPreferenceScreen.removeAll();
        mCategory.clear();
        mSwitchToDefaultCarrier = null;

        //Create one category for each hierarchy
        travelCarriersStorage(storage, travelCarriersStorage(storage,
                Carrier.getCurrentCarriersName(mCurrentPreferenceKey)));

        if (Utils.DEBUG) Log.i(TAG, "End build the preferences.");
        // Set each category's value
        Iterator<Entry<String, RadioGroupPreferenceCategory>> iterator
                = mCategory.entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<String, RadioGroupPreferenceCategory> entry = iterator.next();
            entry.getValue().setCheckedPreference(mSelectedPreferenceKey.get(TAG_HIERARCHY));
        }
    }

    private String travelCarriersStorage(CarriersStorage storage, String fullName) {
        if (fullName == null) return null;
        for (Carrier carrier : storage.mListCarrier) {
            // Don't show preset package in this list
            if (mSwitchToDefaultCarrier == null) {
                if (TextUtils.isEmpty(carrier.getHierarchy()))
                    if (!(carrier.mName.equals(fullName)
                            || (carrier.getSwitchCarrierName().equals("Default")
                            && carrier.mType.equals(Carrier.TYPE_OTA_ZIP_FILE)))) {
                        continue;
                    }
                if (!Carrier.getSwitchCarrierName(fullName).equals(carrier.getBaseCarrierName())
                        && !fullName.equals(carrier.mName))
                    continue;
            } else {
                if (TextUtils.isEmpty(carrier.getHierarchy())
                        || carrier.mType.equals(Carrier.TYPE_DIRECTORY)) {
                    continue;
                }
                if (!Carrier.getSwitchCarrierName(fullName).equals(carrier.getBaseCarrierName()))
                    continue;
            }
            if (mSwitchToDefaultCarrier != null && carrier.mName.equals("Default"))
                continue;

            // Create the category for this hierarchy.
            RadioGroupPreferenceCategory category = mCategory.get(TAG_HIERARCHY);
            if (category == null) {
                category = new RadioGroupPreferenceCategory(getActivity());
                category.setTitle(TAG_HIERARCHY);
                category.setKey(TAG_HIERARCHY);
                mPreferenceScreen.addPreference(category);
                mCategory.put(TAG_HIERARCHY, category);
            }

            addCarrierToCategory(carrier, fullName, category);
        }
        if (mSwitchToDefaultCarrier != null)
            return mSwitchToDefaultCarrier.mName;
        else return null;
    }

    private void addCarrierToCategory(Carrier carrier, String fullName,
            RadioGroupPreferenceCategory category) {
        //Create the preference of this carrier.
        RadioButtonPreference preference =
                new RadioButtonPreference(getActivity());
        preference.setKey(carrier.mName);
        preference.setTitle(carrier.getRegionalCarrierTitle());
        preference.setTag(carrier);
        category.addPreference(preference);

        if (Carrier.getCurrentCarriersName(mCurrentPreferenceKey).equals(carrier.mName)) {
            mSelectedPreferenceKey.put(TAG_HIERARCHY, carrier.mName);
        }
        if (mSwitchToDefaultCarrier == null && "Default".equals(carrier.getSwitchCarrierName())
                && !"Default".equals(fullName)) {
            mSwitchToDefaultCarrier = carrier;
        }
    }

}
