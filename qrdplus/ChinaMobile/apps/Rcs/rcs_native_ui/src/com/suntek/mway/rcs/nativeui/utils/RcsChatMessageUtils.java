/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.utils;

import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.client.aidl.provider.model.ChatMessage;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.client.api.util.log.LogHelper;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.RcsNativeUIApp;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

import java.io.File;

public class RcsChatMessageUtils {
    public static ChatMessage getChatMessageOnSMSDB(Context context, String id) {
        Uri uri = Uri.parse("content://sms/" + id);
        Cursor cursor = context.getContentResolver()
                .query(uri, null, null, null, null);
        ChatMessage msg = null;
        if (cursor != null && cursor.moveToFirst()) {
            if (!cursor.isAfterLast()) {
                msg = new ChatMessage();
                msg.setMessageId(cursor.getString(cursor.getColumnIndex("rcs_message_id")));
                msg.setContact(cursor.getString(cursor
                        .getColumnIndex("address")));
                msg.setConversationId(cursor.getString(cursor
                        .getColumnIndex("rcs_conversation_id")));
                msg.setContributionId(cursor.getString(cursor.getColumnIndex("rcs_contribution_id")));
                msg.setFilename(cursor.getString(cursor
                        .getColumnIndex("rcs_filename")));
                msg.setMsgType(cursor.getInt(cursor.getColumnIndex("rcs_msg_type")));
                msg.setFilesize(cursor.getInt(cursor.getColumnIndex("rcs_filesize")));
                msg.setFileSelector(cursor.getString(cursor
                        .getColumnIndex("rcs_file_selector")));
                msg.setFileTransferId(cursor.getString(cursor
                        .getColumnIndexOrThrow("rcs_file_transfer_id")));
                msg.setFileIcon(cursor.getString(cursor.getColumnIndexOrThrow("rcs_file_icon")));
                msg.setData(cursor.getString(cursor.getColumnIndexOrThrow("rcs_data")));
                msg.setSendReceive(cursor.getInt(cursor
                        .getColumnIndexOrThrow("rcs_send_receive")));
            }
        }
        if (cursor != null) {
            cursor.close();
        }
        return msg;

    }

    public static ChatMessage getTestChatMessage() {
        ChatMessage msg = new ChatMessage();
        msg.setMsgType(SuntekMessageData.MSG_TYPE_AUDIO);
        msg.setData("BurnMessage");
        // msg.setMsgType(SuntekMessageData.MSG_TYPE_IMAGE);
        // msg.setFilename("a.jpg");
        return msg;
    }

    public static String getFilePath(ChatMessage cMsg)
            throws ServiceDisconnectedException {
        String path = RcsApiManager.getMessageApi()
                .getFilepath(cMsg);
        if (path != null && new File(path).exists()) {
            return path;
        } else {
            if (path != null && path.lastIndexOf("/") != -1) {
                path = path.substring(0, path.lastIndexOf("/") + 1);
                return path + cMsg.getFilename();
            } else {
                return null;
            }
        }

    }

    public static boolean isFileDownload(String filePath, long fileSize) {

        if (TextUtils.isEmpty(filePath)) {
            return false;
        }
        boolean isDownload = false;
        File file = new File(filePath);
        if (file != null) {
            LogHelper.trace("filePath = " + filePath + " ; thisFileSize = "
                    + file.length() + " ; fileSize = " + fileSize);
            if (file.exists() && file.length() >= fileSize) {
                isDownload = true;
            }
        }
        return isDownload;

    }
}
