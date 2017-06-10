/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.receiver;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountsDetail;
import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.client.aidl.provider.model.ChatMessage;
import com.suntek.mway.rcs.client.api.im.impl.MessageApi;
import com.suntek.mway.rcs.client.api.publicaccount.callback.PublicAccountCallback;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.publicaccount.PublicAccountApplication;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.RemoteException;
import android.text.TextUtils;

public class RcsMessageReceiver extends BroadcastReceiver {

    private static final String RCS_MESSAGE_NOTIFY_ACTION = "com.suntek.mway.rcs.ACTION_UI_SHOW_MESSAGE_NOTIFY";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent != null) {
            if (RCS_MESSAGE_NOTIFY_ACTION.equals(intent.getAction())) {
                MessageApi messageApi = RcsApiManager.getMessageApi();
                try {
                    long threadId = intent.getLongExtra("threadId", 0);
                    ChatMessage chatMessage = messageApi.getTheLastMessage(threadId);
                    if (chatMessage.getChatType() != SuntekMessageData.CHAT_TYPE_PUBLIC)
                        return;
                    if (PublicAccountApplication.getInstance().getNowThreadId() != threadId) {
                        String uuid = intent.getStringExtra("contact");
                        loadPublicAccountDetail(uuid, threadId);
                    } else {
                        try {
                            RcsApiManager.getMessageApi().removeUnreadMessageByThreadId(
                                    threadId + "");
                        } catch (ServiceDisconnectedException e) {
                            e.printStackTrace();
                        }
                        PublicAccountApplication.getInstance().vibrator(300);
                    }
                } catch (ServiceDisconnectedException e1) {
                    e1.printStackTrace();
                }
            }
        }
    }

    private void loadPublicAccountDetail(String uuid, final long threadId) {
        try {
            PublicAccountsDetail mPublicAccountsDetail = RcsApiManager.getPublicAccountApi()
                    .getPublicDetailCache(uuid);
            if (mPublicAccountsDetail != null && TextUtils.isEmpty(mPublicAccountsDetail.getName())
                    && TextUtils.isEmpty(mPublicAccountsDetail.getPaUuid())) {
                RcsApiManager.getPublicAccountApi().getPublicDetail(uuid, new RcsPublicAccountCallback(threadId));
                return;
            }
            RcsApiManager.getPublicAccountApi().getPublicDetail(uuid, new RcsPublicAccountCallback(threadId));
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    private class RcsPublicAccountCallback extends PublicAccountCallback{
        private long mThreadId;
        
        public RcsPublicAccountCallback(long threadId){
            mThreadId = threadId;
        }

        @Override
        public void respSetAcceptStatus(boolean arg0, String arg1) throws RemoteException {
        }

        @Override
        public void respGetPublicDetail(boolean arg0, final PublicAccountsDetail arg1)
                throws RemoteException {
            try {
                RcsApiManager.getPublicAccountApi().unregisterCallback(this);
            } catch (ServiceDisconnectedException e) {
                e.printStackTrace();
            }
            if (arg1 != null) {
                RcsNotifyManager.getInstance().showNewMessageNotif(arg1, mThreadId + "",
                        true);
            }
        }
    }

}
