/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.accesscache;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.ref.WeakReference;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.text.TextUtils;
import android.util.Log;

import com.qualcomm.qti.accesscache.ICarrierAccessCacheService;

public class CarrierAccessCacheService extends Service {
    private static final String TAG = "CarrierAccessCacheService";


    private static File CACHE_RECOVERY_DIR = new File("/cache/recovery");
    private static File CACHE_COMMAND_FILE = new File(CACHE_RECOVERY_DIR, "command");
    private static File CACHE_LOG_FILE = new File(CACHE_RECOVERY_DIR, "log");
    private static File CACHE_ACTION_FILE = new File("/cache/action");

    public CarrierAccessCacheService() {
    }

    public boolean writeActionFile(String contents) {
        if (TextUtils.isEmpty(contents)) return false;

        try {
            CACHE_ACTION_FILE.delete();
            FileWriter actionFile = new FileWriter(CACHE_ACTION_FILE);
            try {
                actionFile.write(contents);
                actionFile.write("\n");
            } finally {
                actionFile.close();
            }
        } catch (IOException e) {
            Log.d(TAG, "write action file failed " + e);
            return false;
        }

        return true;
    }

    public boolean writeCommandFile(String filePath, String locale) {

        try {
            CACHE_RECOVERY_DIR.mkdirs();
            CACHE_COMMAND_FILE.delete();
            CACHE_LOG_FILE.delete();

            FileWriter commandFile = new FileWriter(CACHE_COMMAND_FILE);
            try {
                if (!TextUtils.isEmpty(filePath)) {
                    commandFile.write(filePath);
                    commandFile.write("\n");
                }
                if (!TextUtils.isEmpty(locale)) {
                    commandFile.write(locale);
                    commandFile.write("\n");
                }
            } finally {
                commandFile.close();
            }
        } catch (IOException e) {
            Log.d(TAG, "write CommandFile failed " + e);
            return false;
        }
        return true;
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    private final IBinder mBinder = new ServiceStub(this);

    private static class ServiceStub extends ICarrierAccessCacheService.Stub {
        WeakReference<CarrierAccessCacheService> mService;

        ServiceStub(CarrierAccessCacheService service) {
            mService = new WeakReference<CarrierAccessCacheService>(service);
        }

        @Override
        public boolean writeActionFile(String contents) {
            return mService.get().writeActionFile(contents);
        }

        @Override
        public boolean writeCommandFile(String filepathStr, String localeStr) {
            return mService.get().writeCommandFile(filepathStr, localeStr);
        }
    }

}
