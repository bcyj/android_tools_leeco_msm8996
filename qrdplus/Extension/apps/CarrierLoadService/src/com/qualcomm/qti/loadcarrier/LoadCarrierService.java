/**
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.HashMap;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.text.TextUtils;
import android.util.Log;

import com.qualcomm.qti.loadcarrier.ILoadCarrierService;

public class LoadCarrierService extends Service {
    private static final String TAG = "LoadCarrierService";

    public LoadCarrierService() {
    }

    public HashMap<String, String> getCarrierList() {
        // Get the sdcard path.
        String sdcardPath = Utils.getPath(Utils.FLAG_STORAGE_EXTERNAL);
        String secSdcardPath = Utils.getPath(Utils.FLAG_STORAGE_SECONDARY);
        if (Utils.DEBUG) Log.d(TAG, "The external storage directory: " + sdcardPath);
        if (Utils.DEBUG) Log.d(TAG, "The secondary storage directory: " + secSdcardPath);

        HashMap<String, String> carrierList = new HashMap<String, String>();
        if (!TextUtils.isEmpty(sdcardPath))
            Utils.getCarrierList(carrierList, sdcardPath);
        if (!TextUtils.isEmpty(secSdcardPath))
            Utils.getCarrierList(carrierList, secSdcardPath);

        return carrierList;
    }

    public String copyToData(String srcFilePath) {
        if (TextUtils.isEmpty(srcFilePath)) return null;

        // Check if the original file exist.
        File srcFile = new File(srcFilePath);
        if (srcFile == null || !srcFile.exists()) return null;
        srcFilePath = "/sdcard/" + srcFile.getName();

        //String dstFilePath = getCacheDir().getAbsolutePath() + "/" + srcFile.getName();
        //File dstFile = new File(dstFilePath);
        //if (Utils.copyFile(srcFile, dstFile)) return dstFilePath;
        return srcFilePath;
    }

    public String downloadToData(String url) {
        String srcFilePath = Utils.downloadFile(url);
        return copyToData(srcFilePath);
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

    private static class ServiceStub extends ILoadCarrierService.Stub {
        WeakReference<LoadCarrierService> mService;

        ServiceStub(LoadCarrierService service) {
            mService = new WeakReference<LoadCarrierService>(service);
        }

        @Override
        public HashMap<String, String> getCarrierList() {
            return mService.get().getCarrierList();
        }

        @Override
        public String copyToData(String srcFilePath) {
            return mService.get().copyToData(srcFilePath);
        }

        @Override
        public String downloadToData(String url) {
            return mService.get().downloadToData(url);
        }
    }

}
