/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Message;
import android.util.Log;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

public class FeatureProvider extends ContentProvider {

    private static final String TAG = "feature_provider";

    private static final String METHOD_GET_SMSC = "get_smsc";
    private static final String METHOD_SET_SMSC = "set_smsc";
    private static final String METHOD_GET_PRIMARY_SUB = "get_primary_sub";
    private static final String METHOD_GET_PREF_PRIMARY_SUB = "get_pref_primary_sub";
    private static final String METHOD_IS_PRIMARY_SETABLE = "is_primary_setable";
    private static final String METHOD_IS_PRIMARY_ENABLED = "is_primary_enabled";
    private static final String METHOD_SET_PREF_NETWORK = "set_pref_network";
    private static final String METHOD_START_CALL_RECORD = "start_call_record";
    private static final String METHOD_STOP_CALL_RECORD = "stop_call_record";
    private static final String METHOD_IS_CALL_RECORD_RUNNING = "is_call_record_running";
    private static final String METHOD_IS_CALL_RECORD_AVAILABLE = "is_call_record_available";
    private static final String METHOD_GET_CALL_RECORD_DURATION = "get_call_record_duration";
    private static final String METHOD_UPDATE_APN_BY_AREA = "update_apn_by_area";
    private static final String METHOD_SET_NEXT_APN_AS_PREF = "set_next_apn_as_pref";

    static final String EXTRA_SMSC = "smsc";
    static final String EXTRA_NETWORK = "network";
    static final String EXTRA_ACQ_ORDER = "acq_order";
    static final String EXTRA_BAND = "band";
    static final String EXTRA_CALLBACK = "callback";
    static final String EXTRA_CURRENT_APN = "current_apn";
    static final String EXTRA_USEROBJ = "userobj";
    static final String EXTRA_EXCEPTION = "exception";
    static final String EXTRA_RESULT = "result";

    @Override
    public boolean onCreate() {
        return false;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        return null;
    }

    @Override
    public String getType(Uri uri) {
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        return null;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        return 0;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        return 0;
    }

    @Override
    public Bundle call(String method, String arg, Bundle extras) {
        logd("method:" + method + ", arg:" + arg + ", extra:" + extras);
        Bundle bundle = new Bundle();
        if (METHOD_IS_PRIMARY_ENABLED.equals(method)) {
            bundle.putBoolean(EXTRA_RESULT, PrimarySubPolicy.getInstance(getContext())
                    .isPrimaryEnabled());
        } else if (METHOD_IS_PRIMARY_SETABLE.equals(method)) {
            bundle.putBoolean(EXTRA_RESULT, PrimarySubPolicy.getInstance(getContext())
                    .isPrimarySetable());
        } else if (METHOD_GET_PREF_PRIMARY_SUB.equals(method)) {
            bundle.putInt(EXTRA_RESULT, PrimarySubPolicy.getInstance(getContext())
                    .getPrefPrimarySub());
        } else if (METHOD_GET_PRIMARY_SUB.equals(method)) {
            bundle.putInt(EXTRA_RESULT, PrimarySubPolicy.getInstance(getContext())
                    .getPrimarySub());
        } else if (METHOD_SET_PREF_NETWORK.equals(method) && extras != null) {
            if (extras.containsKey(EXTRA_BAND)) {
                Intent bandSetIntent = new Intent(getContext(), FeatureService.class);
                bandSetIntent.setAction(FeatureService.ACTION_ENABLE_TDD_LTE);
                bandSetIntent.putExtras(extras);
                getContext().startService(bandSetIntent);
            } else {
                int slot = extras.getInt(PhoneConstants.SLOT_KEY, 0);
                int network = extras.getInt(EXTRA_NETWORK, Phone.NT_MODE_GSM_ONLY);
                Message message = (Message) extras.getParcelable(EXTRA_CALLBACK);
                PrefNetworkRequest request =
                        new PrefNetworkRequest(getContext(), slot, network, message);
                if (extras.containsKey(EXTRA_ACQ_ORDER)) {
                    request.setAcqOrder(extras.getInt(EXTRA_ACQ_ORDER,
                            Constants.NW_ACQ_ORDER_LTE_TDS_GSM));
                }
                request.loop();
            }
        } else if (METHOD_GET_SMSC.equals(method) && extras != null) {
            int slot = extras.getInt(PhoneConstants.SLOT_KEY, 0);
            Message message = (Message) extras.getParcelable(EXTRA_CALLBACK);
            new Smsc(slot, message).get();
        } else if (METHOD_SET_SMSC.equals(method) && extras != null) {
            int slot = extras.getInt(PhoneConstants.SLOT_KEY, 0);
            String smsc = extras.getString(EXTRA_SMSC);
            Message message = (Message) extras.getParcelable(EXTRA_CALLBACK);
            new Smsc(slot, message).set(smsc);
        } else if (METHOD_START_CALL_RECORD.equals(method)) {
            CallRecorder.getInstance(getContext()).start();
        } else if (METHOD_STOP_CALL_RECORD.equals(method)) {
            CallRecorder.getInstance(getContext()).stop();
        } else if (METHOD_IS_CALL_RECORD_RUNNING.equals(method)) {
            bundle.putBoolean(EXTRA_RESULT, CallRecorder.getInstance(getContext()).isRecording());
        } else if (METHOD_IS_CALL_RECORD_AVAILABLE.equals(method)) {
            bundle.putBoolean(EXTRA_RESULT, CallRecorder.getInstance(getContext()).isAvailable());
        } else if (METHOD_GET_CALL_RECORD_DURATION.equals(method)) {
            bundle.putLong(EXTRA_RESULT, CallRecorder.getInstance(getContext()).getDuration());
        } else if (METHOD_UPDATE_APN_BY_AREA.equals(method)) {
            boolean updated = ApnController.getInstance(getContext()).updateApn();
            bundle.putBoolean(EXTRA_RESULT, updated);
        } else if (METHOD_SET_NEXT_APN_AS_PREF.equals(method) && extras != null) {
            long currentId = bundle.getLong(EXTRA_CURRENT_APN, -1);
            boolean updated = ApnController.getInstance(getContext()).setNextAsPref(currentId);
            bundle.putBoolean(EXTRA_RESULT, updated);
        }
        return bundle;
    }

    private void logd(String msg) {
        Log.d(TAG, msg);
    }

}
