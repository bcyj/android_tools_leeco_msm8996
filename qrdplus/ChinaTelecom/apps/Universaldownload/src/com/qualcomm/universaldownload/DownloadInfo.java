/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/
package com.qualcomm.universaldownload;

public class DownloadInfo extends UpdateInfo{
    public static final int NOT_DOWNLOAD = 0;
    public static final int DOWNLOADED = 1;
    private int mDownloadState = NOT_DOWNLOAD;
    private String mDownloadFile = null;
    private int mDownloadSize = 0;

    public DownloadInfo(String url, int size) {
        this.setAddr(url);
        this.mDownloadSize = size;
    }

    public DownloadInfo(String capability, String url, int size) {
        this.setCapibility(capability);
        this.setAddr(url);
        this.mDownloadSize = size;
    }

    public DownloadInfo(UpdateInfo info) {
        this.setCapibility(info.getCapability());
        this.setAddr(info.getAddr());
        this.setMd5(info.getMd5());
        this.setVersion(info.getVersion());
    }

    public int getDownloadSize() {
        return mDownloadSize;
    }

    public void setDownloadSize(int downloadSize) {
        this.mDownloadSize = downloadSize;
    }


    public int getDownloadState() {
        return mDownloadState;
    }

    public void setDownloadState(int state) {
        this.mDownloadState = state;
    }

    public String getDownloadFile() {
        return mDownloadFile;
    }

    public void setDownloadFile(String path) {
        this.mDownloadFile = path;
    }
}
