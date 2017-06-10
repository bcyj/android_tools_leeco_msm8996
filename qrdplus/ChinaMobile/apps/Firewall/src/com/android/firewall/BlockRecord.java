/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
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

import android.net.Uri;
import android.provider.BaseColumns;

// BlockRecord (PCI ADD)
    public class BlockRecord implements BaseColumns {

        public static final Uri CONTENT_URI = Uri
                .parse("content://com.android.firewall/blockrecorditems");

        private static final String BLOCKRECORD_TABLE = "blockrecorditems";
        
        public static final String DEFAULT_SORT_ORDER = "_id ASC";

        public static final int BLOCK_TYPE_CALL = 0;

        public static final int BLOCK_TYPE_RCS_MESSAGE = 1;
        public static final int BLOCK_TYPE_SMS = 2;
        public static final int BLOCK_TYPE_MMS = 3;
        // blockRecord type(0 is call block record , 1 is rcs message block
        // record)
        // 2 is sms block record ,3 is mms block record;
        public static final String BLOCK_TYPE = "block_type";

        public static final String DATE = "date";

        public static final String CONTACT = "contact";

        public static final String DATA = "data";

        public static final String FILENAME = "filename";

        public static final String FILESIZE = "filesize";

        public static final String MIME_TYPE = "mime_type";

        public static final String MSG_TYPE = "msg_type";

        public static final String SEND_RECEIVE = "send_receive";

        public static final String IS_READ = "is_read";

        public static final String MSG_STATE = "msg_state";

        public static final String CHAT_TYPE = "chat_type";

        public static final String CONVERSATION_ID = "conversation_id";

        public static final String CONTRIBUTION_ID = "contribution_id";

        public static final String FILE_SELECTOR = "file_selector";

        public static final String FILE_TRANSFER_EXT = "file_transfer_ext";

        public static final String FILE_TRANSFER_ID = "file_transfer_id";

        public static final String FILE_ICON = "file_icon";

        public static final String BURN_FLAG = "burn_flag";

        public static final String BARCYCLE = "barcycle";

        public static final String HEADER = "header";

        public static final String BODY = "body";

        public static final String TRANSACTIONID = "transactionId";

        public static final String PDUTYPE = "pduType";

        public static final String CONTENTTYPEPARAMETERS = "contentTypeParameters";

        public static final String ADDRESS = "address";

        public static final String SUBID = "subid";

        public static final String PHONEID = "phoneId";

        public static final String SLOTID = "slotid";

        public static final String PRESENTATION= "presentation";

        public static final String DATA_USAGE="data_usage";

        public static final String CALL_TYPE ="call_type";

        public static final String FEATURES ="features";

        public static final String SUBSCRIPTION_COMPONENT_NAME=  "subscription_component_name";

        public static final String SUBSCRIPTION_ID="subscription_id";

        public static final String SUB_ID ="sub_id";

        public static final String NEW ="new";

        public static final String NUMBERTYPE = "numbertype";

        public static final String NUMBERLABEL = "numberlabel";

        public static final String COUNTRYISO = "countryiso";

        public static final String VOICEMAIL_URI ="voicemail_uri";

        public static final String GEOCODED_LOCATION = "geocoded_location";

        public static final String LOOKUP_URI = "lookup_uri";

        public static final String MATCHED_NUMBER = "matched_number";

        public static final String NORMALIZED_NUMBER = "normalized_number";

        public static final String PHOTO_ID = "photo_id";

        public static final String FORMATTED_NUMBER = "formatted_number";

        public static final String _DATA = "_data";

        public static final String HAS_CONTENT = "has_content";

        public static final String SOURCE_DATA = "source_data";

        public static final String SOURCE_PACKAGE = "source_package";

        public static final String TRANSCRIPTION = "transcription";

        public static final String STATE = "state";

        public static final String CREATE_TABLE_BLOCK_RECORD = "CREATE TABLE "+ BLOCKRECORD_TABLE
                    +
                    " (_id INTEGER PRIMARY KEY,"
                    +
                    "contact TEXT,"
                    +
                    "date INTEGER, "
                    +
                    "block_type INTEGER ,"
                    +
                    "data TEXT ,"
                    +
                    "filename TEXT ,"
                    +
                    "filesize INTEGER ,"
                    +
                    "msg_type INTEGER ,"
                    +
                    "send_receive INTEGER ,"
                    +
                    "is_read INTEGER ,"
                    +
                    "msg_state INTEGER ,"
                    +
                    "chat_type INTEGER ,"
                    +
                    "conversation_id TEXT ,"
                    +
                    "contribution_id TEXT ,"
                    +
                    "file_selector TEXT ,"
                    +
                    "file_transfer_ext TEXT ,"
                    +
                    "file_transfer_id TEXT ,"
                    +
                    "burn_flag INTEGER ,"
                    +
                    "barcycle INTEGER ,"
                    +
                    "header TEXT ,"
                    +
                    "transactionId TEXT ,"
                    +
                    "pduType TEXT ,"
                    +
                    "contentTypeParameters TEXT ,"
                    +
                    "address TEXT ,"
                    +
                    "subid TEXT ,"
                    +
                    "phoneId TEXT ,"
                    +
                    "slotid TEXT ,"
                    +
                    " presentation INTEGER NOT NULL DEFAULT 1,"
                    +
                    "data_usage INTEGER,"
                    +
                    "call_type INTEGER,"
                    +
                    "features INTEGER NOT NULL DEFAULT 0,"
                    +
                    "subscription_component_name TEXT,"
                    +
                    "subscription_id TEXT,"
                    +
                    " sub_id INTEGER DEFAULT -1,"
                    +
                    "new INTEGER,name TEXT,"
                    +
                    "numbertype INTEGER,"
                    +
                    "numberlabel TEXT,"
                    +
                    "countryiso TEXT,"
                    +
                    "voicemail_uri TEXT,"
                    +
                    "geocoded_location TEXT,"
                    +
                    "lookup_uri TEXT,"
                    +
                    "matched_number TEXT,"
                    +
                    "normalized_number TEXT,"
                    +
                    "photo_id INTEGER NOT NULL DEFAULT 0,"
                    +
                    "formatted_number TEXT,"
                    +
                    "_data TEXT,"
                    +
                    "has_content INTEGER,"
                    +
                    "mime_type TEXT,"
                    +
                    "source_data TEXT,"
                    +
                    "source_package TEXT,"
                    +
                    "transcription TEXT,"
                    +
                    "state INTEGER  ,"
                    +
                    "body TEXT);";
    }