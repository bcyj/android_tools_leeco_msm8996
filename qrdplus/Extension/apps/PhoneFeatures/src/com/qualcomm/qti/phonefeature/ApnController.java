/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.phonefeature;

import java.util.ArrayList;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.Telephony;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.PhoneFactory;

public class ApnController {

    private static final String TAG = "ApnController";

    private static final String EMPTY_NUMERIC = "00000";

    private static final String COLUMN_PPP_NUMBER = "ppp_number";
    private static final String COLUMN_LOCALIZED_NAME = "localized_name";
    private static final String COLUMN_VISIT_AREA = "visit_area";

    private static final Uri CONTENT_URI_PREF_APN = Telephony.Carriers.CONTENT_URI.buildUpon()
            .appendPath("preferapn").build();
    private static final String APN_ID = "apn_id";

    private static ApnController sApnController;

    private final ContentResolver mContentResolver;

    private ApnController(Context context) {
        mContentResolver = context.getContentResolver();
    }

    public static ApnController getInstance(Context context) {
        synchronized (ApnController.class) {
            if (sApnController == null) {
                sApnController = new ApnController(context);
            }
            return sApnController;
        }
    }

    /**
     * set next apn as preferred apn, if current apn is in the last position, set first apn as
     * preferred
     *
     * @param currentId
     * @return
     */
    public boolean setNextAsPref(long currentId) {
        Log.d(TAG, "setNextAsPref: current preferred apn id " + currentId);
        int slotId = SubscriptionManager.getSlotId(PhoneFactory.getDataSubscription());
        if (!isSlotValid(slotId)){
            Log.d(TAG, "setNextAsPref: invalid slot, " + slotId);
            return false;
        }
        String iccNumeric = Constants.getIccOperatorNumeric(slotId);
        if (!isNumericValid(iccNumeric)) {
            Log.d(TAG, "updateApn: invalid icc numeric, " + iccNumeric);
            return false;
        }
        String where = Telephony.Carriers.NUMERIC + "=\"" + iccNumeric + "\" and "
                + Telephony.Carriers.CARRIER_ENABLED + " = 1";
        Cursor cursor = mContentResolver.query(Telephony.Carriers.CONTENT_URI, null,
                where, null, Telephony.Carriers._ID);
        if (null == cursor) {
            return false;
        }
        try {
            long id = -1;
            // find next apn with bigger apn id.
            while (cursor.moveToNext()
                    && (id = cursor.getLong(cursor.getColumnIndex(Telephony.Carriers._ID)))
                    > currentId) {
                return setPreferredAPN(id);
            }
            // not find bigger apn id, just set the first one as preferred
            if (cursor.moveToFirst() && cursor.moveToNext()) {
                id = cursor.getLong(cursor.getColumnIndex(Telephony.Carriers._ID));
                if (id != currentId) {
                    return setPreferredAPN(id);
                }
            }
        } finally {
            cursor.close();
        }
        return false;
    }

    private boolean setPreferredAPN(long id) {
        Log.d(TAG, "setPreferredAPN: apn id " + id);
        ContentValues values = new ContentValues();
        values.put(APN_ID, id);
        return mContentResolver.insert(CONTENT_URI_PREF_APN, values) != null;
    }

    /**
     * update all apns by spn and plmn, if not found, create a new apn, and if the visit area is
     * mismatch/match, enable/disbale apns
     *
     * @return
     */
    public boolean updateApn() {
        int slotId = SubscriptionManager.getSlotId(PhoneFactory.getDataSubscription());
        if (!isSlotValid(slotId)){
            Log.d(TAG, "updateApn: invalid slot, " + slotId);
            return false;
        }
        String spn = Constants.getIccOperatorNumeric(slotId);
        if (!isNumericValid(spn)) {
            Log.d(TAG, "updateApn: invalid icc numeric, " + spn);
            return false;
        }
        boolean updated = false;
        String where = Telephony.Carriers.NUMERIC + "=\"" + spn + "\"";
        Cursor cursor = mContentResolver.query(Telephony.Carriers.CONTENT_URI, null,
                where, null, Telephony.Carriers.DEFAULT_SORT_ORDER);
        if (null != cursor && cursor.getCount() > 0) {
            String plmn = TelephonyManager.getDefault()
                   .getNetworkOperatorForPhone(slotId);
            updated = updateApnByArea(cursor, plmn);
        } else {
            updated = addDefaultApn(spn);
        }
        if (null != cursor) {
            cursor.close();
        }
        return updated;
    }

    private boolean isNumericValid(String numeric) {
        if (TextUtils.isEmpty(numeric) || numeric.length() < 4 || EMPTY_NUMERIC.equals(numeric)) {
            return false;
        }
        return true;
    }

    private boolean isSlotValid(int slot) {
        if (slot < 0 || slot >= AppGlobals.getInstance().mPhones.length) {
            return false;
        }
        return true;
    }

    private boolean updateApnByArea(Cursor cursor, String plmn) {
        if (!isNumericValid(plmn)) {
            Log.d(TAG, "updateApn: invalid numeric, " + plmn);
            return false;
        }
        cursor.moveToFirst();
        final ArrayList<ContentProviderOperation> operationList =
                new ArrayList<ContentProviderOperation>();
        while (cursor.moveToNext()) {
            String visitArea = null;
            try {
                visitArea = cursor.getString(cursor.getColumnIndexOrThrow(COLUMN_VISIT_AREA));
                if (TextUtils.isEmpty(visitArea)) {
                    continue;
                }
            } catch (IllegalArgumentException e) {
                Log.d(TAG, "updateApnByArea: there is no " + COLUMN_VISIT_AREA + " column exists");
                return false;
            }
            boolean match = plmn.startsWith(visitArea);
            boolean enable = cursor.getInt(cursor
                    .getColumnIndex(Telephony.Carriers.CARRIER_ENABLED)) == 1;
            if (match != enable) {
                ContentProviderOperation.Builder builder = ContentProviderOperation
                        .newUpdate(Telephony.Carriers.CONTENT_URI);
                builder.withValue(Telephony.Carriers.CARRIER_ENABLED, match);
                builder.withSelection(Telephony.Carriers._ID + "=?",
                        new String[] {
                            String.valueOf(cursor.getInt(cursor
                                    .getColumnIndex(Telephony.Carriers._ID)))
                        });
                operationList.add(builder.build());
            }
        }
        if (!operationList.isEmpty()) {
            try {
                mContentResolver.applyBatch(
                        Telephony.Carriers.CONTENT_URI.getAuthority(), operationList);
                return true;
            } catch (RemoteException e) {
                Log.d(TAG, "updateApnByArea: failed to apply batch", e);
            } catch (OperationApplicationException e) {
                Log.d(TAG, "updateApnByArea: failed to apply batch", e);
            }
        }
        return false;
    }

    private boolean addDefaultApn(String spn) {
        boolean inserted = false;
        Cursor cursor = mContentResolver.query(Telephony.Carriers.CONTENT_URI, null,
                Telephony.Carriers.NUMERIC + "=\"" + EMPTY_NUMERIC + "\"", null,
                Telephony.Carriers.DEFAULT_SORT_ORDER);
        if (cursor != null && cursor.moveToNext()) {
            ContentValues values = new ContentValues();
            values.put(Telephony.Carriers.NAME, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.NAME)));
            values.put(Telephony.Carriers.NUMERIC, spn);
            values.put(Telephony.Carriers.MCC, spn.substring(0, 3));
            values.put(Telephony.Carriers.MNC, spn.substring(3));
            values.put(Telephony.Carriers.APN, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.APN)));
            values.put(Telephony.Carriers.USER, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.USER)));
            values.put(Telephony.Carriers.SERVER, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.SERVER)));
            values.put(Telephony.Carriers.PASSWORD, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.PASSWORD)));
            values.put(Telephony.Carriers.MMSPORT, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.MMSPORT)));
            values.put(Telephony.Carriers.MMSPROXY, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.MMSPROXY)));
            values.put(Telephony.Carriers.MMSC, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.MMSC)));
            values.put(Telephony.Carriers.TYPE, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.TYPE)));
            values.put(Telephony.Carriers.CURRENT, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.CURRENT)));
            values.put(Telephony.Carriers.MMSPROXY, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.MMSPROXY)));
            values.put(Telephony.Carriers.AUTH_TYPE, cursor.getInt(cursor.getColumnIndex(
                    Telephony.Carriers.AUTH_TYPE)));
            values.put(Telephony.Carriers.PROTOCOL, cursor.getString(cursor.getColumnIndex(
                    Telephony.Carriers.PROTOCOL)));
            values.put(Telephony.Carriers.ROAMING_PROTOCOL,
                    cursor.getString(cursor.getColumnIndex(Telephony.Carriers.ROAMING_PROTOCOL)));
            values.put(Telephony.Carriers.CARRIER_ENABLED,
                    cursor.getInt(cursor.getColumnIndex(Telephony.Carriers.CARRIER_ENABLED)));
            values.put(Telephony.Carriers.BEARER,
                    cursor.getInt(cursor.getColumnIndex(Telephony.Carriers.BEARER)));
            try {
                values.put(COLUMN_PPP_NUMBER,
                        cursor.getString(cursor.getColumnIndexOrThrow(COLUMN_PPP_NUMBER)));
            } catch (IllegalArgumentException e) {
            }
            try {
                values.put(COLUMN_VISIT_AREA,
                        cursor.getString(cursor.getColumnIndexOrThrow(COLUMN_VISIT_AREA)));
            } catch (IllegalArgumentException e) {
            }
            try {
                values.put(COLUMN_LOCALIZED_NAME,
                        cursor.getString(cursor.getColumnIndexOrThrow(COLUMN_LOCALIZED_NAME)));
            } catch (IllegalArgumentException e) {
            }
            inserted = mContentResolver.insert(Telephony.Carriers.CONTENT_URI, values) != null;
        } else {
            Log.d(TAG, "addDefaultApn: there is no apn with numeric " + EMPTY_NUMERIC);
        }
        if (cursor != null) {
            cursor.close();
        }
        return inserted;
    }

}
