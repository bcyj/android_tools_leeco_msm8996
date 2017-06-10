/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.service;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Message;
import android.util.Log;

public class DeviceInfoProvider extends ContentProvider {
    private static final String TAG = DeviceInfoProvider.class.getSimpleName();

    private static final String EXTRA_CALLBACK = "callback";
    private static final String RESULT = "result";
    private static final boolean DBG = false;
    private DeviceInfoPool mDeviceInfoPool = null;

    @Override
    public boolean onCreate() {
        if (DBG) {
            Log.d(TAG, "onCreate");
        }
        mDeviceInfoPool = DeviceInfoPool.getInstance(getContext());
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
    public Bundle call(String method, String nodeParam, Bundle extras) {
        Bundle bundle = new Bundle();
        Message msg = extras.getParcelable(EXTRA_CALLBACK);
        bundle.putBoolean(RESULT, mDeviceInfoPool.dispatchNodeOperation(nodeParam, msg));
        return bundle;
    }

}
