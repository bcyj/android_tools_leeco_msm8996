/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.service;

import java.util.List;

import com.suntek.mway.rcs.client.aidl.constant.BroadcastConstants;
import com.suntek.mway.rcs.client.aidl.provider.model.GroupChatModel;
import com.suntek.mway.rcs.client.aidl.provider.model.GroupChatUser;
import com.suntek.mway.rcs.client.api.im.impl.MessageApi;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.client.api.util.log.LogHelper;
import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.model.ConferenceEvent;
import com.suntek.mway.rcs.nativeui.model.OnAliasUpdateEvent;
import com.suntek.mway.rcs.nativeui.model.OnChairmanChangeEvent;
import com.suntek.mway.rcs.nativeui.model.OnDisbandEvent;
import com.suntek.mway.rcs.nativeui.model.OnGroupDeletedEvent;
import com.suntek.mway.rcs.nativeui.model.OnMemberJoinEvent;
import com.suntek.mway.rcs.nativeui.model.OnMemberKickedEvent;
import com.suntek.mway.rcs.nativeui.model.OnMemberQuitEvent;
import com.suntek.mway.rcs.nativeui.model.OnOverMaxCountEvent;
import com.suntek.mway.rcs.nativeui.model.OnReferErrorEvent;
import com.suntek.mway.rcs.nativeui.model.OnRemarkChangeEvent;
import com.suntek.mway.rcs.nativeui.model.OnSubjectChangeEvent;
import com.suntek.mway.rcs.nativeui.ui.RcsGroupChatAssignAsChairmanNotification;
import com.suntek.mway.rcs.nativeui.ui.RcsGroupChatInviteNotification;
import com.suntek.mway.rcs.nativeui.utils.RcsContactUtils;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.IBinder;
import android.text.TextUtils;
import android.util.Log;

public class RcsNotificationsService extends Service {
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        IntentFilter filter = new IntentFilter();
        filter.addAction(BroadcastConstants.UI_GROUP_MANAGE_NOTIFY);
        filter.addAction(BroadcastConstants.UI_INVITE_TO_JOIN_GROUP);
        filter.addAction(BroadcastConstants.UI_JOIN_GROUP_INVITE_TIMEOUT);
        filter.addAction(BroadcastConstants.UI_SHOW_GROUP_REFER_ERROR);
        filter.addAction(BroadcastConstants.UI_SHOW_GROUP_TRANSFER_CHAIRMAN_CONFIRM);
        registerReceiver(mRcsServiceCallbackReceiver, filter);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private boolean isChairman(List<GroupChatUser> list) throws ServiceDisconnectedException {
        // Get my phone number.
        String myPhoneNumber = RcsApiManager.getAccountApi().getRcsUserProfileInfo().getUserName();
        if (TextUtils.isEmpty(myPhoneNumber)) {
            return false;
        }
        // Find 'me' in the group.
        boolean isChairman = false;
        for (GroupChatUser user : list) {
            if (myPhoneNumber.endsWith(user.getNumber())) {
                if (GroupChatUser.ROLE_ADMIN.equals(user.getRole())) {
                    isChairman = true;
                }
                break;
            }
        }
        return isChairman;
    }

    private BroadcastReceiver mRcsServiceCallbackReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i("RCS_UI", "onReceive(): action=" + action);

            Bundle extras = intent.getExtras();
            if (extras != null) {
                for (String key : extras.keySet()) {
                    Log.i("RCS_UI", "onReceive(): extra: " + key + "=" + extras.get(key));
                }
            }

            if (BroadcastConstants.UI_GROUP_MANAGE_NOTIFY.equals(action)
                    || BroadcastConstants.UI_SHOW_GROUP_REFER_ERROR.equals(action)) {
                String actionType = extras.getString(BroadcastConstants.BC_VAR_MSG_ACTION_TYPE);
                if (BroadcastConstants.ACTION_TYPE_CREATE.equals(actionType)) {
                    // Remove the notification once the group chat is created.
                    String chatUri = extras.getString(BroadcastConstants.BC_VAR_CHARURI);
                    RcsNotificationList.getInstance().removeInviteNotificationByChatUri(chatUri);
                    String groupId = intent
                            .getStringExtra(BroadcastConstants.BC_VAR_MSG_GROUP_ID);
                    long rcsThreadId = intent.getLongExtra("threadId", -1);
                    Log.i("RCS_UI", "onReceive(): rcsThreadId=" + rcsThreadId);
                    try {
                        GroupChatModel model = RcsApiManager.getMessageApi()
                                .getGroupChatByThreadId(rcsThreadId);

                        if (model != null) {
                            String groupTitle = TextUtils.isEmpty(model
                                    .getRemark()) ? model.getSubject()
                                    : model.getRemark();
                            RcsContactUtils.insertGroupChat(context, groupId, groupTitle);
                            Log.i("RCS_UI", "onReceive(): insertGroupChat=" + rcsThreadId);
                        }
                    } catch (ServiceDisconnectedException e) {
                        Log.i("RCS_UI", "GroupChatMessage" + e);
                    }
                } else if (BroadcastConstants.ACTION_TYPE_UPDATE_CHAIRMAN.equals(actionType)){
                    String groupId = extras.getString(BroadcastConstants.BC_VAR_GROUP_ID);
                    long rcsThreadId = intent.getLongExtra("threadId", -1);
                    try {
                        MessageApi messageApi = RcsApiManager.getMessageApi();
                        GroupChatModel model = messageApi
                                .getGroupChatByThreadId(rcsThreadId);
                        if (model != null) {
                            RcsGroupChatInviteNotification notification = new RcsGroupChatInviteNotification();
                            notification.setChatUri(model.getChatUri());
                            notification.setGroupId(groupId);
                            notification.setInviteTime(System
                                    .currentTimeMillis());
                            notification.setSubject(model.getSubject());
                            notification.setIsChairmanChange(true);
                            boolean isChairman = isChairman(model.getUserList());
                            if (isChairman) {
                                notification.setNumberData(context.getString(
                                        R.string.chairman_change_to_me,
                                        model.getSubject()));
                            } else {
                                notification.setNumberData(context.getString(
                                        R.string.chairman_change_notification,
                                        model.getSubject()));
                            }
                            RcsNotificationList.getInstance().add(notification);
                        }
                    } catch (ServiceDisconnectedException e) {
                    }
                }
                onConferenceManage(context, intent, actionType);
            } else if (BroadcastConstants.UI_INVITE_TO_JOIN_GROUP.equals(action)) {
                String groupId = extras.getString(BroadcastConstants.BC_VAR_GROUP_ID);
                String subject = extras.getString(BroadcastConstants.BC_VAR_GROUP_SUBJECT);
                String contributionId = extras.getString(BroadcastConstants.BC_VAR_CONTRIBUTION_ID);
                String conversationId = extras.getString(BroadcastConstants.BC_VAR_CONVERSATION_ID);
                String chatUri = extras.getString(BroadcastConstants.BC_VAR_CHARURI);
                String numberData = extras.getString(BroadcastConstants.BC_VAR_NUMBER_DATA);
                long inviteTime = extras.getLong(BroadcastConstants.BC_VAR_INVITE_TIME);

                RcsGroupChatInviteNotification notification = new RcsGroupChatInviteNotification();
                notification.setChatUri(chatUri);
                notification.setContributionId(contributionId);
                notification.setConversationId(conversationId);
                notification.setGroupId(groupId);
                notification.setInviteTime(inviteTime);
                notification.setNumberData(numberData);
                notification.setSubject(subject);

                RcsNotificationList.getInstance().add(notification);
            } else if (BroadcastConstants.UI_JOIN_GROUP_INVITE_TIMEOUT.equals(action)) {
                String subject = extras.getString(BroadcastConstants.BC_VAR_GROUP_SUBJECT);
                RcsNotificationList.getInstance().removeInviteNotificationBySubject(subject);
            } else if (BroadcastConstants.UI_SHOW_GROUP_TRANSFER_CHAIRMAN_CONFIRM.equals(action)) {
                String subject = extras.getString(BroadcastConstants.BC_VAR_GROUP_SUBJECT);
                String contributionId = extras.getString(BroadcastConstants.BC_VAR_CONTRIBUTION_ID);
                String conversationId = extras.getString(BroadcastConstants.BC_VAR_CONVERSATION_ID);
                String chatUri = extras.getString(BroadcastConstants.BC_VAR_CHARURI);
                long inviteTime = extras.getLong(BroadcastConstants.BC_VAR_INVITE_TIME);

                RcsGroupChatAssignAsChairmanNotification notification = new RcsGroupChatAssignAsChairmanNotification();
                notification.setChatUri(chatUri);
                notification.setContributionId(contributionId);
                notification.setConversationId(conversationId);
                notification.setInviteTime(inviteTime);
                notification.setSubject(subject);

                RcsNotificationList.getInstance().add(notification);
            }
        }

        private void onConferenceManage(Context context, Intent intent, String actionType) {
            String groupId = intent.getStringExtra(BroadcastConstants.BC_VAR_MSG_GROUP_ID);
            ConferenceEvent event = null;
            if (BroadcastConstants.UI_SHOW_GROUP_REFER_ERROR.equals(intent.getAction())) {
                String referType = intent.getStringExtra(BroadcastConstants.BC_VAR_REFER_TYPE);
                event = new OnReferErrorEvent(groupId, referType);
            } else {
                String member = intent.getStringExtra(BroadcastConstants.BC_VAR_MSG_PHONE);
                if (BroadcastConstants.ACTION_TYPE_UPDATE_SUBJECT.equals(actionType)) {
                    // Group chat subject is updated.
                    String newSubject = intent
                            .getStringExtra(BroadcastConstants.BC_VAR_GROUP_SUBJECT);
                    LogHelper.i("onReceive Action = " + intent.getAction() + " , groupId = "
                            + groupId + " , newSubject = " + newSubject);
                    event = new OnSubjectChangeEvent(groupId, newSubject);
                } else if (BroadcastConstants.ACTION_TYPE_UPDATE_REMARK.equals(actionType)) {
                    String remark = intent.getStringExtra(BroadcastConstants.BC_VAR_GROUP_REMARK);
                    LogHelper.i("onReceive Action = " + intent.getAction() + " , groupId = "
                            + groupId + " , remark = " + remark);
                    event = new OnRemarkChangeEvent(groupId, remark);
                    if (!TextUtils.isEmpty(remark)) {
                        RcsContactUtils.UpdateGroupChatSubject(context, groupId, remark);
                    }
                } else if (BroadcastConstants.ACTION_TYPE_UPDATE_ALIAS.equals(actionType)) {
                    String phoneNumber = intent.getStringExtra(BroadcastConstants.BC_VAR_MSG_PHONE);
                    String alias = intent.getStringExtra(BroadcastConstants.BC_VAR_ALIAS);
                    event = new OnAliasUpdateEvent(groupId, phoneNumber, alias);
                } else if (BroadcastConstants.ACTION_TYPE_UPDATE_CHAIRMAN.equals(actionType)) {
                    String number = intent.getStringExtra(BroadcastConstants.BC_VAR_MSG_PHONE);
                    event = new OnChairmanChangeEvent(groupId, number);
                } else if (BroadcastConstants.ACTION_TYPE_DELETED.equals(actionType)) {
                    event = new OnDisbandEvent(groupId);
                    if (groupId != null) {
                        RcsContactUtils.deleteGroupChat(context, groupId);
                    }
                } else if (BroadcastConstants.ACTION_TYPE_DEPARTED.equals(actionType)) {
                    event = new OnMemberQuitEvent(groupId, member);
                    if (groupId != null) {
                        RcsContactUtils.deleteGroupChat(context, groupId);
                    }
                } else if (BroadcastConstants.ACTION_TYPE_BOOTED.equals(actionType)) {
                    event = new OnMemberKickedEvent(groupId, member);
                } else if (BroadcastConstants.ACTION_TYPE_CONNECTED.equals(actionType)) {
                    event = new OnMemberJoinEvent(groupId, member);
                } else if (BroadcastConstants.REFER_TYPE_QUIT.equals(actionType)) {
                    event = new OnGroupDeletedEvent(groupId);
                    if (groupId != null) {
                        RcsContactUtils.deleteGroupChat(context, groupId);
                    }
                } else if (BroadcastConstants.ACTION_TYPE_OVER_MAXCOUNT.equals(actionType)) {
                    long threadId = intent.getLongExtra(BroadcastConstants.BC_VAR_MSG_THREAD_ID, -1);
                    int maxCount = intent.getIntExtra(BroadcastConstants.BC_VAR_MAXCOUNT, 0);
                    event = new OnOverMaxCountEvent(groupId, threadId, maxCount);
                }
            }
            RcsConferenceListener.getInstance().notifyChanged(groupId, event);
        }
    };
}
