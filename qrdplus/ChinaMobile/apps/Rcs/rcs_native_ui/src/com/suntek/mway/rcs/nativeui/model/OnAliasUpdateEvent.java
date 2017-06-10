/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.model;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;

public class OnAliasUpdateEvent extends ConferenceEvent {

    private String phoneNumber;
    private String alias;

    public OnAliasUpdateEvent(String groupId, String phoneNumber, String alias) {
        this.phoneNumber = phoneNumber;
        this.alias = alias;
    }

    @Override
    public String getActionType() {
        return BroadcastConstants.ACTION_TYPE_UPDATE_ALIAS;
    }

    public String getPhoneNumber() {
        return phoneNumber;
    }

    public String getAlias() {
        return alias;
    }

}
