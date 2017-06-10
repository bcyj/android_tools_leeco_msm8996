/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.http.service;

import com.suntek.mway.rcs.publicaccount.util.CommonUtil;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;

import android.os.SystemClock;

public class ProgressFileDownloadService extends PAHttpService {

    public ProgressFileDownloadService(Request req) {
        super(req);
    }

    @Override
    public void service() {
        String absFilePath = CommonUtil.getFileCacheLocalPath(request.downloadFileType,
                request.requestUrl);
//        CommonUtil.createFolderIfNotExist(absFilePath);
        try {
            HttpURLConnection conn = PAHttpConnection.loopDownloadFile(request.requestUrl);
            readDataAndUpdateProgress(conn, absFilePath);
            SystemClock.sleep(100);
            response.state = Response.SECCESS;
            response.responseObject = absFilePath;
        } catch (Exception e) {
            response.state = Response.FAIL;
            response.responseObject = e;
            e.printStackTrace();
        }
        callback();
    }

    private boolean readDataAndUpdateProgress(HttpURLConnection conn, String absFilePath)
            throws Exception {

        InputStream inputStream = conn.getInputStream();

        if (inputStream == null)
            return false;

        int bufferSize = 1024;
        byte[] buffer = new byte[bufferSize];

//        CommonUtil.createFolderIfNotExist(absFilePath);
        File file = CommonUtil.createFile(absFilePath);
        FileOutputStream fos = new FileOutputStream(file);

        float fileSize = conn.getContentLength();
        int readCount = 0;
        int length = -1;

        int index = 0;
        while ((length = inputStream.read(buffer)) != -1) {
            fos.write(buffer, 0, length);
            readCount += length;
            index++;
            if ((index & 0x3f) == 0) {
                int percent = (int)((readCount / fileSize) * 100);
                upateProgress(percent);
            }
        }

        SystemClock.sleep(300);
        fos.flush();
        fos.close();
        inputStream.close();
        PAHttpConnection.closeConnection(conn);
        upateProgress(100);
        return true;
    }

    private void upateProgress(int progress) {
        feedback(String.valueOf(progress));
    }

    private void feedback(String message) {
        response.state = Response.PROGRESS;
        response.feedback = message;
        callback();
    }
}
