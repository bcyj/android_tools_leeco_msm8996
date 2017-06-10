/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.data;

public interface PageNode {
    public Page findPage(String key);
    public Page findPage(int id);
}
