/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool;

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

import java.util.HashMap;

public class Operation extends AsyncQueryHandler {
    private static final String TAG = "OperationModel";

    public static final String EMPTY = "";

    private static final String OP_CONTENT = "(EditContent)";
    private static final String COL_RESULT = "result";

    private static Operation sInstance = null;

    private ContentResolver mContentResolver;
    private HashMap<OperationData, OnOpCompleteListener> mListeners;

    public static Operation getInstance(ContentResolver cr) {
        if (sInstance == null) {
            sInstance = new Operation(cr);
        }
        return sInstance;
    }

    public static String buildParams(String paramString, String content) {
        if (TextUtils.isEmpty(paramString) || TextUtils.isEmpty(content)) {
            return null;
        }

        if (paramString.contains(OP_CONTENT)) {
            return paramString.replace(OP_CONTENT, content);
        } else {
            return paramString;
        }
    }

    private Operation(ContentResolver cr) {
        super(cr);
        mContentResolver = cr;
        mListeners = new HashMap<OperationData, OnOpCompleteListener>();
    }

    public void doOperationAsync(String function, String params, OnOpCompleteListener listener) {
        OperationData opData = OperationData.get(function, params);
        if (opData == null) {
            // Ignore this operation.
            return;
        }

        if (listener != null) {
            mListeners.put(opData, listener);
        }
        startQuery(opData.hashCode(), opData,
                opData._opUri, null, opData._function, opData._params, null);
    }

    public String doOperation(String function, String params) {
        OperationData opData = OperationData.get(function, params);
        if (opData == null) {
            // Ignore this operation.
            return null;
        }

        Cursor cursor = mContentResolver.query(opData._opUri, null,
                opData._function, opData._params, null);
        try {
            if (cursor != null && cursor.moveToFirst()) {
                String result = cursor.getString(cursor.getColumnIndexOrThrow(COL_RESULT));
                return result;
            } else {
                return EMPTY;
            }
        } finally {
            if (cursor != null) cursor.close();
        }
    }

    @Override
    protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
        if (cookie == null) {
            throw new IllegalArgumentException("The query's cookie is null.");
        }

        OnOpCompleteListener listener = mListeners.get(cookie);
        if (listener == null) return;

        try {
            if (cursor != null && cursor.moveToFirst()) {
                String result = cursor.getString(cursor.getColumnIndexOrThrow(COL_RESULT));
                listener.onComplete(result);
            } else {
                listener.onComplete(EMPTY);
            }
        } finally {
            if (cursor != null) cursor.close();
        }
        mListeners.remove(cookie);
    }

    public interface OnOpCompleteListener {
        public void onComplete(String result);
    }

    private static class OperationData {
        private static final String OP_SEP_COMMA = ",";
        private static final String OP_SEP_BRACKET_LEFT = "{";
        private static final String OP_SEP_BRACKET_RIGHT = "}";

        private static final String SPACE_SYSTEM = "system";
        private static final String SPACE_PHONE = "phone";

        private static final String AUTHORITY = "com.qualcomm.qti.engineertool.operation";
        private static final String AUTHORITY_SYSTEM = AUTHORITY + "." + SPACE_SYSTEM;
        private static final String AUTHORITY_PHONE = AUTHORITY + "." + SPACE_PHONE;
        private static final Uri CONTENT_URI_SYSTEM = Uri.parse("content://" + AUTHORITY_SYSTEM);
        private static final Uri CONTENT_URI_PHONE = Uri.parse("content://" + AUTHORITY_PHONE);

        public long _time;

        public Uri _opUri;
        public String _function;
        public String[] _params;

        private OperationData(long curTime, Uri opUri, String function, String[] params) {
            _opUri = opUri;
            _function = function;
            _params = params;
            _time = curTime;
        }

        @Override
        public boolean equals(Object o) {
            if (o instanceof OperationData) {
                OperationData another = (OperationData) o;
                return this._opUri.equals(another._opUri)
                        && this._function.equals(another._function)
                        && this._params.equals(another._params)
                        && this._time == another._time;
            }
            return super.equals(o);
        }


        public static OperationData get(String function, String paramString) {
            if (TextUtils.isEmpty(function)) return null;

            String[] params = null;
            if (TextUtils.isEmpty(paramString)) {
                params = new String[] { EMPTY };
            } else if (paramString.contains(OP_SEP_COMMA)) {
                params = paramString.split(OP_SEP_COMMA);
            } else {
                params = new String[] { paramString };
            }

            // Get the operation's function and space.
            Uri opUri = null;
            String opSpace = null;
            if (function.contains(OP_SEP_BRACKET_LEFT)
                    && function.contains(OP_SEP_BRACKET_RIGHT)) {
                String[] funs = function.split(OP_SEP_BRACKET_RIGHT);
                opSpace = funs[0].replace(OP_SEP_BRACKET_LEFT, "");
                function = funs[1];
                if (SPACE_PHONE.equals(opSpace)) {
                    opUri = CONTENT_URI_PHONE;
                } else {
                    if (!SPACE_SYSTEM.equals(opSpace)) {
                        Log.e(TAG, "Do not support this space now. opSpace = " + opSpace);
                    }
                    opUri = CONTENT_URI_SYSTEM;
                }
            } else {
                opSpace = EMPTY;
                opUri = CONTENT_URI_SYSTEM;
            }

            return new OperationData(System.currentTimeMillis(), opUri, function, params);
        }
    }
}
