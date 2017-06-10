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
import com.suntek.mway.rcs.nativeui.service.RcsNotificationsService;

import android.content.Context;
import android.util.Log;

public class RcsGroupChatAssignAsChairmanNotification extends RcsNotification {
    private String subject;
    private String contributionId;
    private String conversationId;
    private String chatUri;
    private long inviteTime;

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

    public long getInviteTime() {
        return inviteTime;
    }

    public void setInviteTime(long inviteTime) {
        this.inviteTime = inviteTime;
    }

    public void acceptAssignedAsChairman() throws ServiceDisconnectedException {
        ConfApi confApi = RcsApiManager.getConfApi();
        confApi.acceptAssignedAsChairman(chatUri, inviteTime, conversationId, contributionId);
    }

    public void refuseAssigedAsChairman() throws ServiceDisconnectedException {
        ConfApi confApi = RcsApiManager.getConfApi();
        confApi.refuseAssigedAsChairman(chatUri, inviteTime, conversationId, contributionId);
    }

    @Override
    public String getText(Context context) {
        return context.getString(R.string.invite_group_chat_chairman);
    }

    @Override
    public void onPositiveButtonClicked() {
        try {
            RcsAccountApi accountApi = RcsApiManager.getAccountApi();
            if (!accountApi.isOnline()) {
                Log.d("RCS_UI", "acceptAssignedAsChairman() aborded due to RCS offline");
                return;
            }

            acceptAssignedAsChairman();
            RcsNotificationList.getInstance().remove(this);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onNegativeButtonClicked() {
        try {
            RcsAccountApi accountApi = RcsApiManager.getAccountApi();
            if (!accountApi.isOnline()) {
                Log.d("RCS_UI", "refuseAssigedAsChairman() aborded due to RCS offline");
                return;
            }

            refuseAssigedAsChairman();
            RcsNotificationList.getInstance().remove(this);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    @Override
    public boolean getIsChairmanChange() {
        return false;
    }
}
