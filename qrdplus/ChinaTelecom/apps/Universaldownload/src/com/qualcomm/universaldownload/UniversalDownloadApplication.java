/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import android.app.Application;
import android.content.Intent;

public class UniversalDownloadApplication extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        startService(new Intent(this, DownloadService.class));
    }
}
