/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.backup;

import com.android.backup.IBackupCallback;

interface IRemoteBackupService {
    void init(String packageName);
    void registerCallback(String packageName, IBackupCallback cb);
    void unregisterCallback(String packageName, IBackupCallback cb);
    void setCancelBackupFlag(boolean flag);
}
