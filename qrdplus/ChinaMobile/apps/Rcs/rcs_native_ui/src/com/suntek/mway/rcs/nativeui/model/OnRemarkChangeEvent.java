/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.model;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;

public class OnRemarkChangeEvent extends ConferenceEvent {

    private String remark;

    public OnRemarkChangeEvent(String groupId, String remark) {
        this.remark = remark;
    }
    @Override
    public String getActionType() {
        return BroadcastConstants.ACTION_TYPE_UPDATE_REMARK;
    }

    public String getRemark() {
        return remark;
    }




}
