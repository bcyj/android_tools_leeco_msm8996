/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.imstestrunner;

import android.app.Application;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

public class ImsTestRunner extends Application {
    public ImsTestRunner() {
    }

    public void onCreate() {
        Log.d("ImsTestRunner", "onCreate");
        boolean testMode = SystemProperties.getBoolean("persist.qualcomm.imstestrunner", false);
        if (testMode) {
            Intent i = new Intent(this, FakeImsResponseService.class);
            startService(i);
        }
    }
}