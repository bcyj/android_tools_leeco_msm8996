/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.backup;

interface IBackupCallback {
    void handleBackupMsg(int what, int type, int result);
}
