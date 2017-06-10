/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import android.content.Context;

public abstract class RcsNotification {
    public abstract String getSubject();
    public abstract boolean getIsChairmanChange();
    public abstract String getText(Context context);
    public abstract void onPositiveButtonClicked();
    public abstract void onNegativeButtonClicked();
}
