/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.data;

public interface SetupDataCallbacks {
    void onPageLoaded(Page page);
    void onPageTreeChanged();
    void onPageFinished(Page page);
    Page getPage(String key);
    Page getPage(int key);
}
