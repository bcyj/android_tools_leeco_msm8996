/**
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.customerservice;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Vector;

import com.qualcomm.customerservice.R;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.telephony.SmsManager;
import android.util.Log;

public class MsmUtil {
    class MsmElement {
        String mTarget;
        String mText;

        public MsmElement(String target, String text) {
            mTarget = target;
            mText = text;
        }

        boolean equals(MsmElement m) {
            if (m.mTarget.equals(mTarget) && m.mText.equals(mText))
                return true;
            return false;
        }
    }

    private final String TAG = "MsmUtil";
    private static final String TARGET_SMS = "SMS_TARGET";
    private static final String TEXT_SMS = "SMS_TEXT";
    private static final String ACTION_SMS = "com.qualcomm.customerservice.action.sms";
    private static Object mSyncObj = new Object();
    private static Vector<MsmElement> mMsmList = new Vector<MsmElement>();
    private SmsManager mSmsManager = SmsManager.getDefault();
    private SMSReceiver mSendReceiver = null;
    private Context mContext;

    public MsmUtil(Context contex) {
        mContext = contex;
        mSendReceiver = new SMSReceiver();
        IntentFilter sendFilter = new IntentFilter();
        sendFilter.addAction(ACTION_SMS);
        contex.registerReceiver(mSendReceiver, sendFilter);
    }

    private boolean contains(String t, String txt) {
        Iterator<MsmElement> it = mMsmList.iterator();
        while (it.hasNext()) {
            MsmElement m = it.next();
            if (m.mTarget.equals(t) && m.mText.equals(txt))
                return true;
        }
        return false;
    }

    private void remove(String t, String txt) {
        Iterator<MsmElement> it = mMsmList.iterator();
        int i = -1;
        while (it.hasNext()) {
            i++;
            MsmElement m = it.next();
            if (m.mTarget.equals(t) && m.mText.equals(txt)) {
                mMsmList.remove(i);
                return;
            }
        }
    }

    public void sendMsmDelay(Context context, String target, String text) {
        if (contains(target, text)) {
            ToastHint.makeText(context,
                    context.getResources().getString(R.string.sms_sending))
                    .show();
            Log.d(TAG, "this message exists!");
            return;
        }

        synchronized (mSyncObj) {
            mMsmList.add(new MsmElement(target, text));
        }

        Intent it = new Intent();
        it.setAction(ACTION_SMS);
        it.putExtra(TARGET_SMS, target);
        it.putExtra(TEXT_SMS, text);
        Log.d(TAG, "target=" + target + " text = " + text);
        ArrayList<String> texts = mSmsManager.divideMessage(text);
        PendingIntent mPI = PendingIntent.getBroadcast(context, 0, it,
                PendingIntent.FLAG_UPDATE_CURRENT);

        for (String txt : texts) {
            Log.d(TAG, "sendMsmDelay text=" + txt);
            mSmsManager.sendTextMessage(target, null, txt, mPI, null);
        }
    }

    public void release() {
        Log.v(TAG, "release");
        mContext.unregisterReceiver(mSendReceiver);
    }

    private class SMSReceiver extends BroadcastReceiver {
        public void onReceive(Context context, Intent intent) {
            int resultCode = getResultCode();
            Bundle bundle = intent.getExtras();
            if (bundle == null)
                return;
            String tar = (String) bundle.getString(TARGET_SMS);
            String txt = (String) bundle.getString(TEXT_SMS);
            synchronized (mSyncObj) {
                remove(tar, txt);
            }

            switch (resultCode) {
            case Activity.RESULT_OK:
                Log.d(TAG, "send Successed, target  = " + tar + " , text = "
                        + txt);
                String success = context.getResources().getString(
                        R.string.sms_send_success);
                success = String.format(success, tar);
                ToastHint.makeText(context, success).show();
                break;
            default:
                Log.d(TAG, "send faild, target  = " + tar + " , text = " + txt);
                String fail = context.getResources().getString(
                        R.string.sms_send_failed);
                fail = String.format(fail, tar);
                ToastHint.makeText(context, fail).show();
            }
        }
    }
}
