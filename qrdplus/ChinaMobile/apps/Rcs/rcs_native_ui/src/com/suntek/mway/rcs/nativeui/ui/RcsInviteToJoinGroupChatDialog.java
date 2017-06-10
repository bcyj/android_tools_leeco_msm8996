/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.client.aidl.provider.model.GroupChatModel;
import com.suntek.mway.rcs.client.aidl.provider.model.GroupChatUser;
import com.suntek.mway.rcs.client.api.exception.MemberFullException;
import com.suntek.mway.rcs.client.api.im.impl.MessageApi;
import com.suntek.mway.rcs.client.api.impl.groupchat.ConfApi;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

public class RcsInviteToJoinGroupChatDialog {
    public static void showInviteToJoinGroupChatDialog(Context context, final String number) {
        if (TextUtils.isEmpty(number)) {
            return;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(R.string.invate_to_join_group_chat);

        final MessageApi messageApi = RcsApiManager.getMessageApi();
        final ConfApi confApi = RcsApiManager.getConfApi();

        final List<GroupChatModel> allGroupChatList = new ArrayList<GroupChatModel>();
        try {
            allGroupChatList.addAll(messageApi.getAllGroupChat());
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }

        // Filter the group chat that this user is already in.
        final List<GroupChatModel> groupChatList = new ArrayList<GroupChatModel>();
        for (GroupChatModel groupChat : allGroupChatList) {
            boolean isExist = false;
            for (GroupChatUser user : groupChat.getUserList()) {
                if (number.equals(user.getNumber())) {
                    isExist = true;
                    break;
                }
            }
            if (!isExist) {
                groupChatList.add(groupChat);
            }
        }

        int size = groupChatList.size();
        String[] items = new String[groupChatList.size()];
        for (int i = 0; i < size; i++) {
            items[i] = groupChatList.get(i).getSubject();
        }

        if (items.length > 0) {
            builder.setItems(items, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    GroupChatModel groupChat = groupChatList.get(which);
                    try {
                        confApi.inviteToJoinGroupChat(String.valueOf(groupChat.getId()), number);
                    } catch (ServiceDisconnectedException e) {
                        Log.w("RCS_UI", e);
                    }
                }
            });
        } else {
            builder.setMessage(R.string.no_group_chat);
        }
        builder.setNegativeButton(android.R.string.no, null);
        AlertDialog dialog = builder.create();
        dialog.setCanceledOnTouchOutside(true);
        dialog.show();
    }
}
