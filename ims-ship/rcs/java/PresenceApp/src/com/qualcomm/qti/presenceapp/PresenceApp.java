/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import android.app.Application;
import android.util.Log;

public class PresenceApp extends Application {
    final String TAG = "PresenceApp";
    @Override
        public void onCreate() {
            Log.d(TAG, "onCreate()");
            super.onCreate();
        }

    @Override
        public void onTerminate() {
            Log.d(TAG, "onTerminate()");
            super.onTerminate();
        }


}
