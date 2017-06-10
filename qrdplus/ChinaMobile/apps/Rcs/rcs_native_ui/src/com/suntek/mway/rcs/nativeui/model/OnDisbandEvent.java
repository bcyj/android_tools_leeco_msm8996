/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.model;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;

public class OnDisbandEvent extends ConferenceEvent {

    public OnDisbandEvent(String groupId) {
        this.groupId = groupId;
    }
    @Override
    public String getActionType() {
        return BroadcastConstants.ACTION_TYPE_DELETED;
    }


}
