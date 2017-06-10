/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.model;

public abstract class ConferenceEvent {
    protected String groupId;

    public abstract String getActionType();

    public String getGroupId() {
        return groupId;
    };
}
