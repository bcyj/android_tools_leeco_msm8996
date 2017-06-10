/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import android.app.Application;

public class GestureApp extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        GesturesStore.create(this);
        GestureManagerService.init(this);
        GestureSilentController.init(this);
    }

}
