/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.client.api.autoconfig.RcsAccountApi;
import com.suntek.mway.rcs.client.api.impl.groupchat.ConfApi;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.service.RcsNotificationList;

import android.content.Context;
import android.util.Log;

public class RcsGroupChatInviteNotification extends RcsNotification {

    private String groupId;
    private String subject;
    private String contributionId;
    private String conversationId;
    private String chatUri;
    private String numberData;
    private long inviteTime;
    private boolean isChairmanChange;

    public void setIsChairmanChange(boolean isChairmanChange) {
        this.isChairmanChange = isChairmanChange;
    }

    public String getGroupId() {
        return groupId;
    }

    public void setGroupId(String groupId) {
        this.groupId = groupId;
    }

    public String getSubject() {
        return subject;
    }

    public void setSubject(String subject) {
        this.subject = subject;
    }

    public String getContributionId() {
        return contributionId;
    }

    public void setContributionId(String contributionId) {
        this.contributionId = contributionId;
    }

    public String getConversationId() {
        return conversationId;
    }

    public void setConversationId(String conversationId) {
        this.conversationId = conversationId;
    }

    public String getChatUri() {
        return chatUri;
    }

    public void setChatUri(String chatUri) {
        this.chatUri = chatUri;
    }

    public String getNumberData() {
        return numberData;
    }

    public void setNumberData(String numberData) {
        this.numberData = numberData;
    }

    public long getInviteTime() {
        return inviteTime;
    }

    public void setInviteTime(long inviteTime) {
        this.inviteTime = inviteTime;
    }

    public void agreeToJoinGroup() throws ServiceDisconnectedException {
        ConfApi confApi = RcsApiManager.getConfApi();
        confApi.agreeToJoinGroup(conversationId, contributionId, chatUri, subject, numberData,
                inviteTime);
    }

    public void refuseToJoinGroup() throws ServiceDisconnectedException {
        ConfApi confApi = RcsApiManager.getConfApi();
        confApi.refuseToJoinGroup(contributionId);
    }

    @Override
    public String getText(Context context) {
        if (isChairmanChange) {
            return getNumberData();
        } else {
            return context.getString(R.string.invite_group_chat);
        }
    }
    
    @Override
    public boolean getIsChairmanChange(){
        return isChairmanChange;
    }

    @Override
    public void onPositiveButtonClicked() {
        try {
            RcsAccountApi accountApi = RcsApiManager.getAccountApi();
            if (!accountApi.isOnline()) {
                Log.d("RCS_UI", "agreeToJoinGroup() aborded due to RCS offline");
                return;
            }

            agreeToJoinGroup();
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onNegativeButtonClicked() {
        try {
            // Refuse to join the group chat.
            refuseToJoinGroup();

            // Delete notification.
            RcsNotificationList.getInstance().remove(this);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }
}
