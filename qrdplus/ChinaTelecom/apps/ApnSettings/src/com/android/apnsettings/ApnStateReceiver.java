/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.apnsettings;

import java.util.ArrayList;

import android.content.BroadcastReceiver;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.OperationApplicationException;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.telephony.MSimTelephonyManager;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.MSimConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccController;

public class ApnStateReceiver extends BroadcastReceiver {
    private static final String TAG = "ApnStateReceiver";
    private static final String APN_ID = "apn_id";
    private static final String PREFERRED_APN_URI =
            "content://telephony/carriers/preferapn";

    private static final String PPP_NUMBER = "ppp_number";
    private static final String LOCALIZED_NAME = "localized_name";
    private static final String VISIT_AREA = "visit_area";

    private static final String SP_OPERATOR_NUMERIC = "operator_numeric";
    private static final String SP_SIM_OPERATOR_NUMERIC = "sim_operator_numeric";

    private Uri mPreferApnUri;
    private Context mContext;
    ContentResolver mResolver;
    private String mIccOperatorNumeric;
    private String mOperatorNumeric;
    static final Uri CARRIERS_TABLE_URI = Telephony.Carriers.CONTENT_URI;
    private static final String ACTION_APN_RESRORE_COMPLETE =
            "com.android.apnsettings.RESRORE_COMPLETE";

    public void onReceive(Context context, Intent intent) {
        mContext = context;
        mResolver = context.getContentResolver();
        String action = intent.getAction();
        if (ACTION_APN_RESRORE_COMPLETE.equals(action)) {
            Log.d(TAG, "reset all apn after restore complete");
            setSPValue(SP_SIM_OPERATOR_NUMERIC, null);
            setSPValue(SP_OPERATOR_NUMERIC, null);
            apnUpdate();
            return;
        }
        int sub = intent.getIntExtra(MSimConstants.SUBSCRIPTION_KEY, 0);
        if (sub != 0) {
            Log.d(TAG, "ignore service change on non-sub0");
            return;
        }

        if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(action)
                || TelephonyIntents.ACTION_SERVICE_STATE_CHANGED.equals(action)) {
            apnUpdate();
        }
    }

    private boolean isAreaMatch(String visit_area, String operatorNumber) {
        boolean isAreaMatch = false;
        if (null != operatorNumber && operatorNumber.length() >= 3) {
            String mcc = (String) operatorNumber.subSequence(0, 3);
            if (mcc.equals(visit_area)) {
                isAreaMatch = true;
            }
        }
        return isAreaMatch;
    }

    private void checkVisitArea(Cursor ret) {
        Log.i(TAG, "checkVisitArea");
        ret.moveToFirst();
        final ArrayList<ContentProviderOperation> operationList =
                new ArrayList<ContentProviderOperation>();
        while (ret.moveToNext()) {
            String visit_area = null;
            try {
                visit_area = ret.getString(ret.getColumnIndexOrThrow(VISIT_AREA));
            } catch (IllegalArgumentException e) {
                Log.w(TAG, e.getMessage());
            }
            // Empty visual area is always enabled, no need to update
            if (TextUtils.isEmpty(visit_area))
                continue;
            boolean match = isAreaMatch(visit_area, mOperatorNumeric);
            boolean enable = ret.getInt(ret.getColumnIndex(Telephony.Carriers.CARRIER_ENABLED)) == 1;
            if (match != enable) {
                ContentProviderOperation.Builder builder =
                        ContentProviderOperation.newUpdate(Telephony.Carriers.CONTENT_URI);
                builder.withValue(Telephony.Carriers.CARRIER_ENABLED, match);
                builder.withSelection(Telephony.Carriers._ID + "=?", new String[] {
                        String.valueOf(ret.getInt(0))
                });
                operationList.add(builder.build());
            }
        }
        if (!operationList.isEmpty()) {
            try {
                mResolver.applyBatch(Telephony.Carriers.CONTENT_URI.getAuthority(), operationList);
            } catch (RemoteException e) {
                Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
            } catch (OperationApplicationException e) {
                Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
            }
        }
        setSPValue(SP_OPERATOR_NUMERIC, mOperatorNumeric);
    }

    private boolean copyDefaultApn() {
        // begin add the same apn config as "46003",only modify the mumeric
        String where = "numeric=\"00000\"";
        Cursor ret = mResolver.query(Telephony.Carriers.CONTENT_URI, null, where, null,
                Telephony.Carriers.DEFAULT_SORT_ORDER);

        if (null == ret || ret.getCount() == 0) {
            Log.e(TAG, "newApnInit,null cursorNew or 0 count for 00000");
            return false;
        }

        // refer to SimRecord, mcc length is 3. mcc is the left;
        String mcc = mIccOperatorNumeric.substring(0, 3);
        String mnc = mIccOperatorNumeric.substring(3);
        Log.i(TAG, "newApnInit mcc:" + mcc + " mnc:" + mnc);

        while (ret.moveToNext()) {
            ContentValues values = new ContentValues();
            values.put(Telephony.Carriers.NAME, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.NAME)));
            values.put(Telephony.Carriers.NUMERIC, mIccOperatorNumeric);
            values.put(Telephony.Carriers.MCC, mcc);
            values.put(Telephony.Carriers.MNC, mnc);
            values.put(Telephony.Carriers.APN, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.APN)));
            values.put(Telephony.Carriers.USER, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.USER)));
            values.put(Telephony.Carriers.SERVER, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.SERVER)));
            values.put(Telephony.Carriers.PASSWORD, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.PASSWORD)));
            values.put(Telephony.Carriers.MMSPORT, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.MMSPORT)));
            values.put(Telephony.Carriers.MMSPROXY, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.MMSPROXY)));
            values.put(Telephony.Carriers.MMSC, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.MMSC)));
            values.put(Telephony.Carriers.TYPE, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.TYPE)));
            values.put(Telephony.Carriers.CURRENT, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.CURRENT)));
            values.put(Telephony.Carriers.MMSPROXY, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.MMSPROXY)));
            values.put(Telephony.Carriers.AUTH_TYPE, ret.getInt(ret.getColumnIndex(
                    Telephony.Carriers.AUTH_TYPE)));
            values.put(Telephony.Carriers.PROTOCOL, ret.getString(ret.getColumnIndex(
                    Telephony.Carriers.PROTOCOL)));
            values.put(Telephony.Carriers.ROAMING_PROTOCOL,
                    ret.getString(ret.getColumnIndex(
                            Telephony.Carriers.ROAMING_PROTOCOL)));
            values.put(Telephony.Carriers.CARRIER_ENABLED,
                    ret.getInt(ret.getColumnIndex(
                            Telephony.Carriers.CARRIER_ENABLED)));
            values.put(Telephony.Carriers.BEARER, ret.getInt(ret.getColumnIndex(
                    Telephony.Carriers.BEARER)));
            try {
                values.put(PPP_NUMBER, ret.getString(ret.getColumnIndexOrThrow(PPP_NUMBER)));
                values.put(VISIT_AREA, ret.getString(ret.getColumnIndexOrThrow(VISIT_AREA)));
                values.put(LOCALIZED_NAME, ret.getString(ret.getColumnIndexOrThrow(LOCALIZED_NAME)));
            } catch (IllegalArgumentException e) {
                Log.w(TAG, e.getMessage());
            }
            mResolver.insert(CARRIERS_TABLE_URI, values);
        }
        ret.close();
        return true;
    }

    private boolean isMccMncValid(String numeric) {
        if (TextUtils.isEmpty(numeric) || numeric.length() < 4 || "00000".equals(numeric)) {
            Log.e(TAG, "invalid numeric:" + numeric);
            return false;
        }
        return true;
    }

    private String getOperatorNumeric() {
        String operatorNumber = null;
        if (MSimTelephonyManager.getDefault().isMultiSimEnabled()) {
            operatorNumber = MSimTelephonyManager.getDefault().getNetworkOperator(
                    MSimConstants.SUB1);
        } else {
            operatorNumber = TelephonyManager.getDefault().getNetworkOperator();
        }
        Log.d(TAG, "operatorNumber = " + operatorNumber);
        return operatorNumber;
    }

    private String getIccOperatorNumeric() {
        String iccOperatorNumeric = null;
        int dataRat = PhoneFactory.getDefaultPhone().getServiceState().getRilDataRadioTechnology();
        if (dataRat != ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN) {
            int appFamily = UiccController.getFamilyFromRadioTechnology(dataRat);
            IccRecords iccRecords = UiccController.getInstance().getIccRecords(appFamily);
            if (iccRecords != null) {
                iccOperatorNumeric = iccRecords.getOperatorNumeric();
            }

        }
        Log.d(TAG, "iccOperatorNumeric = " + iccOperatorNumeric);
        return iccOperatorNumeric;
    }

    private String getSPValue(String key) {
        return PreferenceManager.getDefaultSharedPreferences(mContext).getString(key, null);
    }

    private boolean setSPValue(String key, String value) {
        return PreferenceManager.getDefaultSharedPreferences(mContext).edit().putString(key, value)
                .commit();
    }

    private void apnUpdate() {
        // get the slot1 numeric
        mIccOperatorNumeric = getIccOperatorNumeric();
        if (!isMccMncValid(mIccOperatorNumeric))
            return;
        mOperatorNumeric = getOperatorNumeric();
        String iccOperatorNumeric = getSPValue(SP_SIM_OPERATOR_NUMERIC);
        String operatorNumeric = getSPValue(SP_OPERATOR_NUMERIC);
        Log.d(TAG, "sp icc operator numeric " + iccOperatorNumeric);
        Log.d(TAG, "sp operator numeric " + operatorNumeric);
        String where = "numeric=\"" + mIccOperatorNumeric + "\"";
        Cursor ret = mResolver.query(Telephony.Carriers.CONTENT_URI, null, where, null,
                Telephony.Carriers.DEFAULT_SORT_ORDER);
        if (null != ret && ret.getCount() > 0) {
            if (isMccMncValid(mOperatorNumeric)
                    && !TextUtils.equals(mOperatorNumeric, operatorNumeric)) {
                checkVisitArea(ret);
                setSPValue(SP_OPERATOR_NUMERIC, mOperatorNumeric);
            }
        } else if (!TextUtils.equals(mIccOperatorNumeric, iccOperatorNumeric)) {
            copyDefaultApn();
            setSPValue(SP_SIM_OPERATOR_NUMERIC, mIccOperatorNumeric);
        }
        if (null != ret) {
            ret.close();
        }
    }
}
