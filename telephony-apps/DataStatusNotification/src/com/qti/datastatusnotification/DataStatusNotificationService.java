/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.qualcomm.datastatusnotification;

import java.lang.Exception;
import java.util.Arrays;
import java.util.List;

import android.app.Service;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.IBinder;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.provider.Telephony;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyProperties;
import com.qualcomm.qcrilhook.IQcRilHook;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;

public class DataStatusNotificationService extends Service {
    private static final String TAG = "QcDataStatusNotification Service";
    private static final boolean DBG = true;
    private static final String MOBILE_DATA = Settings.Global.MOBILE_DATA;
    private static final String DATA_ROAMING = Settings.Global.DATA_ROAMING;
    private static final String CARRIERS = "carriers";
    private static final Uri CARRIERS_URI = Telephony.Carriers.CONTENT_URI;
    private QcRilHook mQcRilHook;
    private int mPhoneCount = 0;
    private DataSettingsObserver mDataSettingsObserver;
    private String[] mDefaultApns;
    private String[] mImsApns;

    private class Apn {
        private String mType;
        private String mApn;

        public Apn(String type, String apn) {
            mType = type;
            mApn = apn;
        }

        public String getType() {
            return mType;
        }

        public String getApn() {
            return mApn;
        }

        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append(mType);
            sb.append(" ");
            sb.append(mApn);
            return sb.toString();
        }
    }

    private QcRilHookCallback mQcRilHookCb = new QcRilHookCallback() {
        public void onQcRilHookReady() {
            enableContentObservers();
        }
        public void onQcRilHookDisconnected() {
            // TODO: Handle onQcRilHookDisconnected
        }
    };

    private class DataSettingsObserver extends ContentObserver {
        DataSettingsObserver() {
            super(null);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            if (uri != null) {
                String authority = uri.getAuthority();
                String uriLastSegment = uri.getLastPathSegment();
                int phoneId = 0;
                if (authority.equals("settings")) { // authority for mobile_data and roaming
                    int uriLength = uriLastSegment.length();
                    phoneId = Character.getNumericValue(uriLastSegment.charAt(uriLength - 1));
                    uriLastSegment = uriLastSegment.substring(0, uriLength - 1);
                }
                if (DBG)
                    Log.d(TAG, "onChange():uri=" + uri.toString() + " authority=" + authority
                            + " path=" + uri.getPath() + " segments=" + uri.getPathSegments()
                            + " uriLastSegment=" + uriLastSegment);
                switch (uriLastSegment) {
                    case MOBILE_DATA:
                        int mobile_data_status = 0;
                        try {
                            mobile_data_status = Settings.Global.getInt(
                                    DataStatusNotificationService.this.
                                            getContentResolver(), MOBILE_DATA + phoneId);
                        } catch (SettingNotFoundException ex) {
                            Log.e(TAG, ex.getMessage());
                        }
                        if (DBG)
                            Log.d(TAG, "handleMessage: Data Enable changed to "
                                    + mobile_data_status
                                    + " on phoneId = " + phoneId);
                        mQcRilHook.qcRilSendDataEnableStatus(mobile_data_status, phoneId);
                        break;
                    case DATA_ROAMING:
                        int data_roaming_status = 0;
                        try {
                            data_roaming_status = Settings.Global.getInt(
                                    DataStatusNotificationService.this.
                                            getContentResolver(), DATA_ROAMING + phoneId);
                        } catch (SettingNotFoundException ex) {
                            Log.e(TAG, ex.getMessage());
                        }
                        if (DBG)
                            Log.d(TAG, "handleMessage: Data Roaming changed to "
                                    + data_roaming_status
                                    + " on phoneId = " + phoneId);
                        mQcRilHook.qcRilSendDataRoamingEnableStatus(data_roaming_status, phoneId);
                        break;
                    case CARRIERS:
                        for (int i = 0; i < mPhoneCount; i++) {
                            onApnChanged(i);
                        }
                        break;
                    default:
                        Log.e(TAG, "Received unsupported uri");
                }
            } else
                Log.e(TAG, "Received uri is null");
        }
    }

    private void enableContentObservers() {
        mPhoneCount = TelephonyManager.getDefault().getPhoneCount();
        // Store internet apns for each phoneId as TelephonyProvider does not call notifyChange for
        // APN changes specific to a phoneId. It calls notifyChange() for any change in carriers DB
        mDefaultApns = new String[mPhoneCount];
        mImsApns = new String[mPhoneCount];
        mDataSettingsObserver = new DataSettingsObserver();
        if (DBG)
            Log.d(TAG, "enableContentObservers: mPhoneCount = " + mPhoneCount);

        for (int i = 0; i < mPhoneCount; i++) {
            // Data Enable Content Observer
            Uri uri = Settings.Global.getUriFor(MOBILE_DATA + i);
            this.getContentResolver().registerContentObserver(uri, false, mDataSettingsObserver);
            // Explicitly trigger onChange() at start of service since modem needs to know
            // the values at start of service
            mDataSettingsObserver.onChange(false, uri);
            // Data Roaming Content Observer
            uri = Settings.Global.getUriFor(DATA_ROAMING + i);
            this.getContentResolver().registerContentObserver(uri, false, mDataSettingsObserver);
            mDataSettingsObserver.onChange(false, uri);
        }

        // Register content observer for carriers DB for changes in APN
        this.getContentResolver().registerContentObserver(CARRIERS_URI, false,
                mDataSettingsObserver);
        mDataSettingsObserver.onChange(false, CARRIERS_URI);
    }

    private void onApnChanged(int phoneId) {
        Apn newApn = null;
        Apn newImsApn = null;
        // Get subIds for a particular phoneId. Only first subId is relevant for now
        int[] subIdList = SubscriptionManager.getSubId(phoneId);
        if (subIdList == null) {
            Log.e(TAG, "subIdList is null. Bail out.");
            return;
        }
        int subId = subIdList[0];

        // Find operator of the subId where APN has changed
        String operator = TelephonyManager.getDefault().getIccOperatorNumeric(subId);
        if (DBG)
            Log.d(TAG, "onApnChanged: phoneId = " + phoneId + " subId = "
                    + subId + " operator = " + operator);

        // For operator, get cursor from DB
        if (operator != null && !operator.isEmpty()) {
            String selection = "numeric = '" + operator + "'";
            selection += " and carrier_enabled = 1";
            Cursor cursor = this.getContentResolver().query(
                    getUri(Telephony.Carriers.CONTENT_URI, subId), new String[] {
                            "apn", "numeric", "type", "carrier_enabled"
                    },
                    selection, null, null);
            if (cursor != null && cursor.getCount() > 0) {
                // Try to get preferredApn if preferred APN is set, can handle
                // default type and has correct operator
                // else get first APN that can handle default type
                if (DBG)
                    Log.d(TAG, "APN change URI is  "
                            + getUri(Telephony.Carriers.CONTENT_URI, subId).toString()
                            + " count = " + cursor.getCount());
                if ((newApn = getPreferredApn(subId, selection)) != null) {
                    if (DBG)
                        Log.d(TAG,
                                "Found preferred APN: " + newApn.toString());
                } else if ((newApn = getDefaultApn(cursor)) != null) {
                    if (DBG)
                        Log.d(TAG, "Found default APN: " + newApn.toString());
                } else {
                    Log.d(TAG,
                            "Could not find preferred or default APN");
                }
                // Call oemhook with newApn if it is not same as previously stored apn
                if (newApn != null && (!newApn.getApn().equals(mDefaultApns[phoneId]))) {
                    if (DBG)
                        Log.d(TAG, "Found new apn.Call oemhook on phoneId " + phoneId);
                    mDefaultApns[phoneId] = newApn.getApn();
                    mQcRilHook.qcRilSendApnInfo(newApn.getType(), newApn.getApn(), 1 /* enable */,
                            phoneId);
                }

                cursor.moveToFirst();
                newImsApn = getImsApn(cursor);
                if (newImsApn != null && (!newImsApn.getApn().equals(mImsApns[phoneId]))) {
                    if (DBG) Log.d(TAG, "Found new IMS apn.Call oemhook on phoneId " + phoneId);
                    mImsApns[phoneId] = newImsApn.getApn();
                    mQcRilHook.qcRilSendApnInfo(newImsApn.getType(), newImsApn.getApn(),
                            1 /* enable */, phoneId);
                }

                cursor.close();
            } else {
                Log.d(TAG,
                        "No rows in Carriers DB for current operator");
            }
        } else {
            Log.d(TAG, "Could not get current operator");
        }

        // The case where previous apn that was enabled is now disabled
        if (mDefaultApns[phoneId] != null && newApn == null) {
            if (DBG)
                Log.d(TAG, "Previous default apn has been removed.Call oemhook on phoneId "
                        + phoneId);
            mQcRilHook.qcRilSendApnInfo(PhoneConstants.APN_TYPE_DEFAULT, mDefaultApns[phoneId],
                    0 /* disable */, phoneId);
            mDefaultApns[phoneId] = null;
        }

        // The case where previous IMS apn that was enabled is now disabled
        if (mImsApns[phoneId] != null && newImsApn == null) {
            if (DBG)
                Log.d(TAG, "Previous IMS apn has been removed.Call oemhook on phoneId "
                        + phoneId);
            mQcRilHook.qcRilSendApnInfo(PhoneConstants.APN_TYPE_IMS, mImsApns[phoneId],
                    0 /* disable */, phoneId);
            mImsApns[phoneId] = null;
        }
    }

    private Apn getApnFound(Cursor cursor, String type) {
        Apn apn = null;
        String[] typesSupported = parseTypes(cursor.getString(cursor
                .getColumnIndexOrThrow(Telephony.Carriers.TYPE)));
        if (DBG) Log.d(TAG, "getApnFound: typesSupported = "
                            + Arrays.toString(typesSupported));
        for (String t : typesSupported) {
            if (t.equalsIgnoreCase(PhoneConstants.APN_TYPE_ALL)
                    || t.equalsIgnoreCase(type)) {
                apn = new Apn(
                        type,
                        cursor.getString(cursor
                                .getColumnIndexOrThrow(Telephony.Carriers.APN)));
                if (DBG) Log.d(TAG, "getApnFound: Apn = " + apn.toString());
                break;
            }
        }
        return apn;
    }

    private Apn getPreferredApn(int subId, String selection) {
        Uri PREFERAPN_URI = Uri.parse("content://telephony/carriers/preferapn");
        Apn preferredApn = null;
        Cursor cursor = this.getContentResolver().query(
                getUri(PREFERAPN_URI, subId), new String[] {
                        "apn", "numeric", "type", "carrier_enabled"
                }, selection, null, null);
        if (cursor != null && cursor.getCount() > 0) {
            cursor.moveToFirst();
            preferredApn = getApnFound(cursor, PhoneConstants.APN_TYPE_DEFAULT);
            cursor.close();
        }
        return preferredApn;
    }

    private Apn getDefaultApn(Cursor cursor) {
        Apn defaultApn = null;
        while (defaultApn == null && cursor.moveToNext()) {
            defaultApn = getApnFound(cursor, PhoneConstants.APN_TYPE_DEFAULT);
        }
        return defaultApn;
    }

    private Apn getImsApn(Cursor cursor) {
        Apn imsApn = null;
        while (imsApn == null && cursor.moveToNext()) {
            imsApn = getApnFound(cursor, PhoneConstants.APN_TYPE_IMS);
        }
        return imsApn;
    }

    private String[] parseTypes(String types) {
        String[] result;
        // If unset set to DEFAULT.
        if (TextUtils.isEmpty(types)) {
            result = new String[1];
            result[0] = PhoneConstants.APN_TYPE_ALL;
        } else {
            result = types.split(",");
        }
        return result;
    }

    private Uri getUri(Uri uri, int subId) {
        return Uri.withAppendedPath(uri, "subId/" + subId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DBG)
            Log.d(TAG, "onStartCommand");
        if(mQcRilHook == null) {
            mQcRilHook = new QcRilHook(this, mQcRilHookCb);
        }
        return START_STICKY;
    }
}
