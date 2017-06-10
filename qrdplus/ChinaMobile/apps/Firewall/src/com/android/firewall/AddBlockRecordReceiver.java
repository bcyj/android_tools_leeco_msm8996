/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.firewall;

import com.android.firewall.FirewallProvider;
import com.suntek.mway.rcs.client.aidl.provider.model.ChatMessage;
import com.android.firewall.BlockRecord;
import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class AddBlockRecordReceiver extends BroadcastReceiver {

    public static final String ADD_CALL_BLOCK_RECORD = "com.android.firewall.ADD_CALL_BLOCK_RECORD";
    public static final String ADD_RCS_MESSAGE_BLOCK_RECORD = "com.android.firewall.ADD_RCS_MESSAGE_BLOCK_RECORD";
    public static final String ADD_SMS_BLOCK_RECORD = "com.android.firewall.ADD_SMS_BLOCK_RECORD";
    public static final String ADD_MMS_BLOCK_RECORD = "com.android.firewall.ADD_MMS_BLOCK_RECORD";
    public static final String BLOCK_NUMBER = "number";
    public static final String MMS_BLOCK_CONTENT = "content";
    private static final int MSG_NUMBER_LEN= 11;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d("firewall", "Action = " + intent.getAction());
        if (ADD_CALL_BLOCK_RECORD.equals(intent.getAction())) {
            String number = intent.getStringExtra(BLOCK_NUMBER);
            int presentation = intent.getIntExtra(BlockRecord.PRESENTATION, 1);
            long date = intent.getLongExtra(BlockRecord.DATE, System.currentTimeMillis());
            int data_usage = intent.getIntExtra(BlockRecord.DATA_USAGE, 0);
            int features = intent.getIntExtra(BlockRecord.FEATURES, 0);
            String subscription_component_name = intent
                    .getStringExtra(BlockRecord.SUBSCRIPTION_COMPONENT_NAME);
            String subscription_id =intent.getStringExtra(BlockRecord.SUBSCRIPTION_ID);
            int sub_id =intent.getIntExtra(BlockRecord.SUB_ID, -1);
            int numbertype= intent.getIntExtra(BlockRecord.NUMBERTYPE,0);
            String numberlabel = intent.getStringExtra(BlockRecord.NUMBERLABEL);
            String countryiso = intent.getStringExtra(BlockRecord.COUNTRYISO);
            String voicemail_uri = intent.getStringExtra(BlockRecord.VOICEMAIL_URI);
            String geocoded_location = intent.getStringExtra(BlockRecord.GEOCODED_LOCATION);
            String lookup_uri = intent.getStringExtra(BlockRecord.LOOKUP_URI);
            String matched_number = intent.getStringExtra(BlockRecord.MATCHED_NUMBER);
            String normalized_number = intent.getStringExtra(BlockRecord.NORMALIZED_NUMBER);
            int photo_id = intent.getIntExtra(BlockRecord.PHOTO_ID, 0);
            String formatted_number = intent.getStringExtra(BlockRecord.FORMATTED_NUMBER);
            String _data = intent.getStringExtra(BlockRecord._DATA);
            int has_content = intent.getIntExtra(BlockRecord.HAS_CONTENT, 0);
            String mime_type = intent.getStringExtra(BlockRecord.MIME_TYPE);
            String source_data = intent.getStringExtra(BlockRecord.SOURCE_DATA);
            String source_package = intent.getStringExtra(BlockRecord.SOURCE_PACKAGE);
            String transcription = intent.getStringExtra(BlockRecord.TRANSCRIPTION);
            ContentValues values =new ContentValues();
            values.put(BlockRecord.CONTACT, number);
            values.put(BlockRecord.BLOCK_TYPE, BlockRecord.BLOCK_TYPE_CALL);
            values.put(BlockRecord.PRESENTATION, presentation);
            values.put(BlockRecord.DATE, date);
            values.put(BlockRecord.DATA_USAGE, data_usage);
            values.put(BlockRecord.FEATURES, features);
            values.put(BlockRecord.SUBSCRIPTION_COMPONENT_NAME, subscription_component_name);
            values.put(BlockRecord.SUBSCRIPTION_ID, subscription_id);
            values.put(BlockRecord.SUB_ID, sub_id);
            values.put(BlockRecord.NUMBERTYPE, numbertype);
            values.put(BlockRecord.NUMBERLABEL, numberlabel);
            values.put(BlockRecord.COUNTRYISO, countryiso);
            values.put(BlockRecord.VOICEMAIL_URI, voicemail_uri);
            values.put(BlockRecord.GEOCODED_LOCATION, geocoded_location);
            values.put(BlockRecord.LOOKUP_URI, lookup_uri);
            values.put(BlockRecord.MATCHED_NUMBER, matched_number);
            values.put(BlockRecord.NORMALIZED_NUMBER, normalized_number);
            values.put(BlockRecord.PHOTO_ID, photo_id);
            values.put(BlockRecord.FORMATTED_NUMBER, formatted_number);
            values.put(BlockRecord._DATA, _data);
            values.put(BlockRecord.HAS_CONTENT, has_content);
            values.put(BlockRecord.MIME_TYPE, mime_type);
            values.put(BlockRecord.SOURCE_DATA, source_data);
            values.put(BlockRecord.SOURCE_PACKAGE,source_package);
            values.put(BlockRecord.TRANSCRIPTION, transcription);
            context.getContentResolver().insert(BlockRecord.CONTENT_URI, values);
        } else if (ADD_RCS_MESSAGE_BLOCK_RECORD.equals(intent.getAction())) {
            ChatMessage cMsg = intent.getParcelableExtra("chatmessage");
            if (cMsg != null) {
                ContentValues values = new ContentValues();
                values.put(BlockRecord.CONTACT, cMsg.getContact());
                values.put(BlockRecord.DATE, cMsg.getTime());
                values.put(BlockRecord.BLOCK_TYPE, BlockRecord.BLOCK_TYPE_RCS_MESSAGE);
                values.put(BlockRecord.DATA, cMsg.getData());
                values.put(BlockRecord.FILENAME, cMsg.getFilename());
                values.put(BlockRecord.FILESIZE, cMsg.getFilesize());
                values.put(BlockRecord.MSG_TYPE, cMsg.getMsgType());
                values.put(BlockRecord.SEND_RECEIVE, cMsg.getSendReceive());
                values.put(BlockRecord.IS_READ, cMsg.getIsRead());
                values.put(BlockRecord.MSG_STATE, cMsg.getMsgState());
                values.put(BlockRecord.CHAT_TYPE, cMsg.getChatType());
                values.put(BlockRecord.CONVERSATION_ID, cMsg.getConversationId());
                values.put(BlockRecord.CONTRIBUTION_ID, cMsg.getContributionId());
                values.put(BlockRecord.FILE_SELECTOR, cMsg.getFileSelector());
                values.put(BlockRecord.FILE_TRANSFER_EXT, cMsg.getFileTransferExt());
                values.put(BlockRecord.FILE_TRANSFER_ID, cMsg.getFileTransferId());
                values.put(BlockRecord.BURN_FLAG, cMsg.getMsgBurnAfterReadFlag());
                values.put(BlockRecord.BARCYCLE, cMsg.getBarCycle());
                context.getContentResolver().insert(BlockRecord.CONTENT_URI, values);
            }

        } else if (ADD_SMS_BLOCK_RECORD.equals(intent.getAction())) {
            String number = intent.getStringExtra(BLOCK_NUMBER);
            long date = intent.getLongExtra(BlockRecord.DATE, System.currentTimeMillis());
            String content = intent.getStringExtra(MMS_BLOCK_CONTENT);
            ContentValues values = new ContentValues();
            values.put(BlockRecord.CONTACT, number);
            values.put(BlockRecord.DATE, date);
            values.put(BlockRecord.BLOCK_TYPE, BlockRecord.BLOCK_TYPE_SMS);
            values.put(BlockRecord.DATA, content);
            context.getContentResolver().insert(BlockRecord.CONTENT_URI, values);

        } else if (ADD_MMS_BLOCK_RECORD.equals(intent.getAction())) {
            String number = intent.getStringExtra(BLOCK_NUMBER);
            long date = intent.getLongExtra(BlockRecord.DATE, System.currentTimeMillis());
            int transactionId = intent.getIntExtra(BlockRecord.TRANSACTIONID, 0);
            int pduType = intent.getIntExtra(BlockRecord.PDUTYPE, 0);
            byte[] header = intent.getByteArrayExtra(BlockRecord.HEADER);
            byte[] data = intent.getByteArrayExtra(BlockRecord.DATA);
            String address = intent.getStringExtra(BlockRecord.ADDRESS);
            long subId = intent.getLongExtra(BlockRecord.SUBID, -1);
            int phoneId = intent.getIntExtra(BlockRecord.PHONEID, 0);
            int slotId = intent.getIntExtra(BlockRecord.SLOTID, -1);
            ContentValues values = new ContentValues();
            values.put(BlockRecord.BLOCK_TYPE, BlockRecord.BLOCK_TYPE_MMS);
            values.put(BlockRecord.CONTACT, number);
            values.put(BlockRecord.DATE, date);
            values.put(BlockRecord.TRANSACTIONID, transactionId);
            values.put(BlockRecord.PDUTYPE, pduType);
            values.put(BlockRecord.HEADER, header);
            values.put(BlockRecord.DATA, data);
            values.put(BlockRecord.ADDRESS, address);
            values.put(BlockRecord.SUBID, subId);
            values.put(BlockRecord.PHONEID, phoneId);
            values.put(BlockRecord.SLOTID, slotId);
            context.getContentResolver().insert(BlockRecord.CONTENT_URI, values);
        }

    }
}
