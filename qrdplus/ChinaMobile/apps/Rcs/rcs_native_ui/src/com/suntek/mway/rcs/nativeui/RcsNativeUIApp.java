/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui;

import com.suntek.mway.rcs.nativeui.database.DB;
import com.suntek.mway.rcs.nativeui.service.RcsNotificationsService;

import android.app.Application;
import android.content.Context;
import android.content.Intent;

public class RcsNativeUIApp extends Application {

    private static Context context;
    private static boolean sNeedClosePs;
    private static boolean sNeedOpenWifi;
    public static Context getContext() {
        return context;
    }
    
    public static void setNeedClosePs(boolean needClosePs){
        sNeedClosePs = needClosePs;
    }

    public static boolean isNeedClosePs(){
        return sNeedClosePs;
    }

    public static void setNeedOpenWifi(boolean needOpenWifi){
        sNeedOpenWifi = needOpenWifi;
    }

    public static boolean isNeedOpenWifi(){
        return sNeedOpenWifi;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        context = getBaseContext();
        RcsApiManager.init(this);
        startService(new Intent(this, RcsNotificationsService.class));

        DB.init(this);
    }
}
