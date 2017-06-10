/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.qualcomm.launcherunreadservice;

import android.app.Service;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.Handler;
import android.os.IBinder;
import android.provider.BaseColumns;
import android.provider.CallLog.Calls;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.util.Log;

import java.util.HashSet;
import java.util.Iterator;

import static com.google.android.mms.pdu.PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND;
import static com.google.android.mms.pdu.PduHeaders.MESSAGE_TYPE_RETRIEVE_CONF;

public class LauncherUnreadService extends Service {
    private static final String TAG = "LauncherUnreadService";
    private static final boolean DEBUG = false;
    private static final int CONTENT_OBSERVER_HANDLER_DELAY_MS = 1000;
    private Handler mHandler = new Handler();
    private ComponentName mMmsComponentName = new
            ComponentName("com.android.mms", "com.android.mms.ui.ConversationList");
    private ComponentName mDialerComponentName = new
            ComponentName("com.android.dialer", "com.android.dialer.DialtactsActivity");

    private static final String[] SMS_PROJECTION = new String[] {
        BaseColumns._ID,
        Sms.THREAD_ID,
    };
    private static final String[] MMS_PROJECTION = new String[] {
        BaseColumns._ID,
        Mms.THREAD_ID,
    };
    private long mMsgId = -1;
    private long mThreadId = -1;
    private String mMsgType = "";

    private final ContentObserver mSMSContentObserver = new ContentObserver(new Handler()) {

        private int lastUnreadCount = -1;
        private long lastMsgId = -1;
        private Runnable changeHandler = new Runnable() {

            @Override
            public void run() {
                int unreadCount = getMissMmsCount();
                if (lastUnreadCount != unreadCount || lastMsgId != mMsgId) {
                    lastUnreadCount = unreadCount;
                    lastMsgId = mMsgId;
                    sendUnreadChangedBroadcast(mMmsComponentName, unreadCount);
                }
            }
        };

        @Override
        public void onChange(boolean selfChange) {
            Log.i(TAG, "mms content changed");
            mHandler.removeCallbacks(changeHandler);
            mHandler.postDelayed(changeHandler, CONTENT_OBSERVER_HANDLER_DELAY_MS);
        }
    };

    private final ContentObserver mCallContentObserver = new ContentObserver(new Handler()) {
        private int lastMissedCount = -1;
        private Runnable changeHandler = new Runnable() {

            @Override
            public void run() {
                int missedCount = getMissCallCount();
                if (lastMissedCount != missedCount) {
                    lastMissedCount = missedCount;
                    sendUnreadChangedBroadcast(mDialerComponentName, missedCount);
                }
            }
        };

        @Override
        public void onChange(boolean selfChange) {
            Log.i(TAG, "Call log content changed");
            mHandler.removeCallbacks(changeHandler);
            mHandler.postDelayed(changeHandler, CONTENT_OBSERVER_HANDLER_DELAY_MS);
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand " + intent);
        ContentResolver resolver = getContentResolver();
        resolver.registerContentObserver(MmsSms.CONTENT_URI, true, mSMSContentObserver);
        resolver.registerContentObserver(Calls.CONTENT_URI, true, mCallContentObserver);
        sendUnreadChangedBroadcast(mMmsComponentName, getMissMmsCount());
        sendUnreadChangedBroadcast(mDialerComponentName, getMissCallCount());
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        ContentResolver resolver = getContentResolver();
        resolver.unregisterContentObserver(mCallContentObserver);
        resolver.unregisterContentObserver(mSMSContentObserver);
    }

    private int getMissMmsCount() {
        int missSmsCount = 0;
        int missMmsCount = 0;
        int missMsgCount = 0;
        HashSet<Long> threads = new HashSet<Long>(4);
        Cursor cursorSMS = null;
        Cursor cursorMMS = null;
        try {
            cursorSMS = getContentResolver().query(
                    Uri.parse("content://sms/inbox/"), SMS_PROJECTION, "(read=0)",
                    null, null);
            cursorMMS = getContentResolver().query(
                    Uri.parse("content://mms/inbox/"), MMS_PROJECTION, "(read=0 AND (" +
                            Mms.MESSAGE_TYPE + "=" + MESSAGE_TYPE_NOTIFICATION_IND + " OR " +
                            Mms.MESSAGE_TYPE + "=" + MESSAGE_TYPE_RETRIEVE_CONF+ "))", null,
                    null);
        } catch (SQLiteException e) {
            return missSmsCount;
        }
        if (cursorSMS != null) {
            missSmsCount = cursorSMS.getCount();
        }
        if (cursorMMS != null) {
            missMmsCount = cursorMMS.getCount();
        }
        missMsgCount = missSmsCount + missMmsCount;
        Log.i(TAG, "getMissMmsCount " + missMsgCount);
        mThreadId = -1;
        mMsgId = -1;
        mMsgType = "";
        try {
            if (missMsgCount == 1) {
                if (missSmsCount == 1 && cursorSMS != null && cursorSMS.moveToFirst()) {
                    mThreadId = cursorSMS.getLong(cursorSMS.getColumnIndex(Sms.THREAD_ID));
                    mMsgId = cursorSMS.getLong(cursorSMS.getColumnIndex(BaseColumns._ID));
                    mMsgType = "sms";
                } else if (cursorMMS != null && cursorMMS.moveToFirst()) {
                    mThreadId = cursorMMS.getLong(cursorMMS.getColumnIndex(Mms.THREAD_ID));
                    mMsgId = cursorMMS.getLong(cursorMMS.getColumnIndex(BaseColumns._ID));
                    mMsgType = "mms";
                }
            } else if (missMsgCount > 1) {
                if (cursorSMS != null) {
                    while (cursorSMS.moveToNext() && threads.size() < 2) {
                        threads.add(cursorSMS.getLong(cursorSMS.getColumnIndex(Sms.THREAD_ID)));
                    }
                }
                if (cursorMMS != null) {
                    while (cursorMMS.moveToNext() && threads.size() < 2) {
                        threads.add(cursorMMS.getLong(cursorMMS.getColumnIndex(Mms.THREAD_ID)));
                    }
                }
                if (threads.size() == 1) {
                    Iterator iterator = threads.iterator();
                    if (iterator.hasNext()) {
                        mThreadId = (Long) iterator.next();
                    }
                }
            }
            if (DEBUG) Log.d(TAG, "mThreadId: " + mThreadId + " mMsgId: " + mMsgId +
                    " mMsgType: " + mMsgType);
        } finally {
            if (cursorSMS != null) {
                cursorSMS.close();
            }
            if (cursorMMS != null) {
                cursorMMS.close();
            }
        }
        return missMsgCount;
    }

    private int getMissCallCount() {
        int missCallCount = 0;
        Uri missingCallUri = Calls.CONTENT_URI;
        String where = Calls.TYPE + " = '" + Calls.MISSED_TYPE + "'"
                + " AND new=1";
        Cursor cursorCall = null;
        try {
            cursorCall = getContentResolver().query(missingCallUri,
                    null, where, null, null);
        } catch (SQLiteException e) {
            return missCallCount;
        }

        if (cursorCall != null) {
            missCallCount = cursorCall.getCount();
            cursorCall.close();
        }
        Log.i(TAG, "getMissCallCount: " + missCallCount);
        return missCallCount;
    }

    private void sendUnreadChangedBroadcast(ComponentName componentName, int unreadCount) {
        Intent intent = new Intent("com.android.launcher.action.UNREAD_CHANGED");
        intent.putExtra("unread_number", unreadCount);
        intent.putExtra("component_name", componentName);
        if(DEBUG) Log.d(TAG, "sendUnreadChangedBroadcast()");
        if (mMmsComponentName.equals(componentName) && unreadCount > 0) {
            intent.putExtra("thread_id", mThreadId);
            intent.putExtra("_id", mMsgId);
            intent.putExtra("type", mMsgType);
        }
        sendBroadcast(intent);
    }
}
