/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.model;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;

public class OnMemberKickedEvent extends ConferenceEvent {

    private String memberNumber;

    public OnMemberKickedEvent(String groupId, String memberNumber) {
        this.groupId = groupId;
        this.memberNumber = memberNumber;
    }
    @Override
    public String getActionType() {
        return BroadcastConstants.ACTION_TYPE_BOOTED;
    }

    public String getMemberNumber() {
        return memberNumber;
    }


}
