/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.receiver;

import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccountsDetail;
import com.suntek.mway.rcs.publicaccount.PublicAccountApplication;
import com.suntek.mway.rcs.publicaccount.R;
import com.suntek.mway.rcs.publicaccount.ui.PAConversationActivity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.text.TextUtils;

import java.util.HashMap;
import java.util.Iterator;

public class RcsNotifyManager {
    private static final int NOTIF_NEW_MESSSGE_ID = 20121237;

    private static RcsNotifyManager instance;

    private static NotificationManager notifManager;

    private HashMap<String, Integer> contactMsgIdMap;

    private static int existID = NOTIF_NEW_MESSSGE_ID;

    private Context mContext;

    private RcsNotifyManager() {
        mContext = PublicAccountApplication.getInstance().getApplicationContext();
        notifManager = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        contactMsgIdMap = new HashMap<String, Integer>();
    }

    public static RcsNotifyManager getInstance() {
        if (instance == null) {
            instance = new RcsNotifyManager();
        }
        return instance;
    }

    @SuppressWarnings("deprecation")
    public void showNewMessageNotif(PublicAccountsDetail publicAccountsDetail, String thread_id,
            boolean shouldPlaySound) {
        if (notifManager == null) {
            return;
        }
        Intent intent = new Intent(mContext, PAConversationActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra("PublicAccountUuid", publicAccountsDetail.getPaUuid());

        Integer id = contactMsgIdMap.get(thread_id);
        if (id == null) {
            id = existID;
            contactMsgIdMap.put(thread_id, id);
            existID++;
        }

        PendingIntent contentIntent = PendingIntent.getActivity(mContext, id, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        String text = publicAccountsDetail.getName();
        if (TextUtils.isEmpty(text)) {
            text = publicAccountsDetail.getNumber();
        }
        String tickerText = text
                + mContext.getString(R.string.have_new_message);
        final Notification notification = new Notification(R.drawable.calllog_tool_sms,
                tickerText, System.currentTimeMillis());
        notification.setLatestEventInfo(mContext,
                mContext.getString(R.string.new_public_account_msg),
                tickerText, contentIntent);

        if (shouldPlaySound) {
            notification.defaults = Notification.DEFAULT_SOUND | Notification.DEFAULT_VIBRATE;
        }
        notification.flags = Notification.FLAG_AUTO_CANCEL;
        notifManager.notify(id, notification);
    }

    public void cancelNewMessageNotif(String thread_id) {
        if (notifManager != null) {
            Integer id = contactMsgIdMap.get(thread_id);
            if (id != null) {
                notifManager.cancel(id);
                contactMsgIdMap.remove(thread_id);
            }
        }
    }

    public void cancelAllMessageNotif() {
        if (notifManager != null) {
            Iterator<String> iterator = contactMsgIdMap.keySet().iterator();
            while (iterator.hasNext()) {
                String contact = iterator.next();
                int id = contactMsgIdMap.get(contact);
                notifManager.cancel(id);
            }
            contactMsgIdMap = new HashMap<String, Integer>();
        }
    }

    public void cancelNewMessageNotif() {
        if (notifManager != null) {
            notifManager.cancel(NOTIF_NEW_MESSSGE_ID);
        }
    }
}
