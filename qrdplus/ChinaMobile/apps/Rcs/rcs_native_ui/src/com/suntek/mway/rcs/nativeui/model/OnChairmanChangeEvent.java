/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.model;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;

public class OnChairmanChangeEvent extends ConferenceEvent {

    private String chairmanNumber;

    public OnChairmanChangeEvent(String groupId, String chaimanNumber) {
        this.chairmanNumber = chaimanNumber;
    }
    @Override
    public String getActionType() {
        return BroadcastConstants.ACTION_TYPE_UPDATE_CHAIRMAN;
    }

    public String getChairmanNumber() {
        return chairmanNumber;
    }


}
