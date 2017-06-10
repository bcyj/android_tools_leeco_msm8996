/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.http.service;

import com.suntek.mway.rcs.publicaccount.controller.ThreadPool;
import com.suntek.mway.rcs.publicaccount.http.service.PAHttpService.Request;
import android.os.Handler.Callback;

public class CommonHttpRequest {


    private static CommonHttpRequest mCommonHttpRequest;

    public static CommonHttpRequest getInstance(){
        if(mCommonHttpRequest == null){
            mCommonHttpRequest = new CommonHttpRequest();
        }
        return mCommonHttpRequest;
    }

    public void downloadFile(String url, int fileType, Callback callback) {
        Request req = new Request();
        req.callback = callback;
        req.downloadFileType = fileType;
        req.requestUrl = url;
        PAHttpService service = new ProgressFileDownloadService(req);
        ThreadPool.getInstance().execute(service);
    }

}
