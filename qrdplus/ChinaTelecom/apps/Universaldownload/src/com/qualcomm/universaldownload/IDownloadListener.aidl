/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

interface IDownloadListener {
    void onUpdateComplete(boolean success);
    void onDownloadProgressUpdate(int progress);
    void onOneFileComplete(int num);
    void onAllComplete();
}
