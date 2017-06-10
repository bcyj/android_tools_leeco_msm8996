/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.operation;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;

public class PhoneOperationHandler extends ContentProvider {
    private static final String TAG = "SystemOperationHandler";

    private static final String AUTHORITY = "com.qualcomm.qti.engineertool.operation.phone";
    private static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY);

    @Override
    public boolean onCreate() {
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String functionName, String[] params,
            String sortOrder) {
        if (!CONTENT_URI.equals(uri)) {
            Log.e(TAG, "Please use the correct uri to start the operation. uri=" + uri.toString());
            return null;
        }

        return OperationUtils.getResult(getContext(), functionName, params);
    }

    @Override
    public String getType(Uri uri) {
        // Do not support this function.
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        // Do not support this function.
        return null;
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        // Do not support this function.
        return 0;
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        // Do not support this function.
        return 0;
    }
}
