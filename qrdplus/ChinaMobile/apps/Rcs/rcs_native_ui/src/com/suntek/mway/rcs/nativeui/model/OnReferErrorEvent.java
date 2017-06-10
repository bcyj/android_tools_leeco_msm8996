/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.model;

public class OnReferErrorEvent extends ConferenceEvent {

    public static final String ON_REFER_ERROR_ACTION = "ON_REFER_ERROR_ACTION";

    private String groupId;
    private String referErrorAction;

    public OnReferErrorEvent(String groupId, String referErrorAction) {
        this.referErrorAction = referErrorAction;
        this.groupId = groupId;

    }

    @Override
    public String getActionType() {
        return ON_REFER_ERROR_ACTION;
    }

    @Override
    public String getGroupId() {
        return super.getGroupId();
    }

    public String getReferErrorAction() {
        return referErrorAction;
    }

}
