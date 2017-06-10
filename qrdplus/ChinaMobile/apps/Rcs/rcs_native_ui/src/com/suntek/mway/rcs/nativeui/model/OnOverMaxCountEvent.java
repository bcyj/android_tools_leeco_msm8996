/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.model;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;

public class OnOverMaxCountEvent extends ConferenceEvent {

    private long mThreadId;
    private int mMaxCount;

    public OnOverMaxCountEvent(String groupId, long threadId, int maxCount) {
        this.groupId = groupId;
        mThreadId = threadId;
        mMaxCount = maxCount;
    }

    @Override
    public String getActionType() {
        return BroadcastConstants.ACTION_TYPE_OVER_MAXCOUNT;
    }

    public long getThreadId() {
        return mThreadId;
    }

    public int getMaxCount() {
        return mMaxCount;
    }
}
