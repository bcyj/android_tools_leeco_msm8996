/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.Framework;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.SystemProperties;
import android.util.Log;
import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;

public class PhoneProcessAgent extends Service {
    String TAG = "PhoneProcessAgent";

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        configMultiSim();
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    private void configMultiSim() {
        logd("");
        /*
        Todo: when getCurrentSubscription() API is ready in framwork
        int MAX_SUB = 2;
        SubscriptionManager mSubscriptionManager = SubscriptionManager.getInstance();
        CardSubscriptionManager mCardSubscriptionManager = CardSubscriptionManager.getInstance();
        logd(SystemProperties.get("persist.dsds.enabled"));
        if (mSubscriptionManager == null || mCardSubscriptionManager == null
        // || !TelephonyManager.getDefault().isMultiSimEnabled()
                || !"dsds".equals(Utilities.getSystemProperties(Values.PROP_MULTISIM, null))) {
            loge("mSubscriptionManager=" + mSubscriptionManager + " mCardSubscriptionManager"
                    + mCardSubscriptionManager);
            return;
        }
        logd(mSubscriptionManager);
        SubscriptionData mCurSubscriptionData = new SubscriptionData(MAX_SUB);
        for (int i = 0; i < MAX_SUB; i++) {
            Subscription sub = mSubscriptionManager.getCurrentSubscription(i);
            mCurSubscriptionData.subscription[i].copyFrom(sub);
        }

        SubscriptionData mSubscriptionData = new SubscriptionData(MAX_SUB);
        mSubscriptionData.copyFrom(mCurSubscriptionData);
        SubscriptionData[] mCardSubscrInfo = new SubscriptionData[MAX_SUB];
        for (int i = 0; i < MAX_SUB; i++) {

            mCardSubscrInfo[i] = mCardSubscriptionManager.getCardSubscriptions(i);
        }

        for (int i = 0; i < MAX_SUB; i++) {
            logd(i + "<=========");
            logd("card2>>>" + mCardSubscrInfo[i]);
            if (mCardSubscrInfo[i] == null)
                continue;
            logd("card2>>>" + mCardSubscrInfo[i].subscription[0]);
            logd("sub1>>>" + mSubscriptionData);
            logd("sub1>>>" + mSubscriptionData.subscription[i]);
            mSubscriptionData.subscription[i].copyFrom(mCardSubscrInfo[i].subscription[0]);
            logd(i + "=========>");
        }

        for (int i = 0; i < MAX_SUB; i++) {
            mSubscriptionData.subscription[i].subId = i;
            // mSubscriptionData.subscription[i].slotId = i;
            // mSubscriptionData.subscription[i].m3gpp2Index = 0;
            mSubscriptionData.subscription[i].subStatus = Subscription.SubscriptionStatus.SUB_ACTIVATE;
        }
        logd(mSubscriptionManager.setSubscription(mSubscriptionData));
        */
    }

    private void logd(Object s) {

        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

    private void loge(Object s) {

        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.e(TAG, s + "");
    }

}
