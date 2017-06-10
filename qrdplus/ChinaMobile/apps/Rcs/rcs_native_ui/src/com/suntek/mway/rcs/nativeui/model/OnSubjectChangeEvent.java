/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.model;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;

public class OnSubjectChangeEvent extends ConferenceEvent {

    private String newSubject;

    public OnSubjectChangeEvent(String groupId, String newSubject) {
        this.groupId = groupId;
        this.newSubject = newSubject;
    }

    @Override
    public String getActionType() {
        return BroadcastConstants.ACTION_TYPE_UPDATE_SUBJECT;
    }

    public String getNewSubject() {
        return newSubject;
    }

}
