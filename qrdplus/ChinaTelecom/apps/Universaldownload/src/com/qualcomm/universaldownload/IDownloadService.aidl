/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import com.qualcomm.universaldownload.UpdateInfo;
import com.qualcomm.universaldownload.IDownloadListener;

interface IDownloadService {
    void checkUpdate(in String requestUrl);
    UpdateInfo getUpdateInfo(in String capabilityName);
    void cancelUpdate();
    void downloadData(in List<UpdateInfo> infos);
    void cancelDownload();
    void registerListener(IDownloadListener listener);
    void unregisterListener();
}
