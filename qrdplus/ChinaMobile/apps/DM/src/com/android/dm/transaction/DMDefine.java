/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.dm.transaction;

/*
 * PIM
 */
public class DMDefine {
    public DMDefine() {
    }

    public static final int PIM_DEVICE_INFO_SYNCTYPE_TWOWAY = 1; // support of
                                                                 // two-way sync

    public static final int PIM_DEVICE_INFO_SYNCTYPE_STWOWAY = 2; // support of
                                                                  // slow
                                                                  // two-way
                                                                  // sync

    public static final int PIM_DEVICE_INFO_SYNCTYPE_RFCLIENT = 4; // refresh
                                                                   // sync from
                                                                   // client
                                                                   // only

    public static final int PIM_DEVICE_INFO_SYNCTYPE_RFSERVER = 6; // refresh
                                                                   // sync from
                                                                   // server
                                                                   // only

    public static final int PIM_MAX_APN_LEN = 50;
    public static final int PIM_MAX_USER_LEN = 25;
    public static final int PIM_MAX_PASSWORD_LEN = 10;
    public static final int PIM_MAX_IP_LEN = 50;
    public static final int PIM_MAX_PORT_LEN = 5;
    public static final int PIM_MAX_SERVER_ADDR_LEN = 100;
    public static final int PIM_MAX_PB_LEN = 20;
    public static final int PIM_MAX_URL_LEN = 100;
    public static final int PIM_MAX_IMEI_LEN = 10;

    public static final int PIM_ANCHOR_MAX = 20;

    public static final int PIM_DATETIME_LEN = 15;

    public static class PIM_EVENT {

        public static final int PIM_EVENT_CONN_EST = 0; // connect established
        public static final int PIM_EVENT_CONN_ERROR = 1; // connect error
        public static final int PIM_EVENT_DISCONNECT = 2; // disconnect the
                                                          // connection
        public static final int PIM_EVENT_SENDING = 3; // sending data
        public static final int PIM_EVENT_SENT = 4; // data sent over
        public static final int PIM_EVENT_SEND_CLIENT_DATA = 5; // send client
                                                                // data
        public static final int PIM_EVENT_RECEIVEING = 6; // receiving data
        public static final int PIM_EVENT_RECEIVE_DONE = 7; // receive data over
        public static final int PIM_EVENT_RECEIVE_SERVER_DATA = 8; // receive
                                                                   // server
                                                                   // data
        public static final int PIM_EVENT_NEXT_STEP = 9; // continue to the next
                                                         // step
        public static final int PIM_EVENT_SHOW_SENDING = 10; // show sending
                                                             // status
        public static final int PIM_EVENT_SHOW_RECEIVING = 11; // show receiving
                                                               // status
        public static final int PIM_EVENT_ADDR_FMT_ERROR = 12; // net address
                                                               // fomat error
        public static final int PIM_EVENT_AUTH_ERROR = 13; // invalid user not
                                                           // registered
        public static final int PIM_EVENT_INIT_ERROR = 14; // something wrong
                                                           // with
                                                           // initialization

        public static final int PIM_EVENT_COMM_ERROR = 15; // communication
                                                           // error
        public static final int PIM_EVENT_SYNC_ERROR = 16; // synchronous error
        public static final int PIM_EVENT_SYNC_SUCCESS = 17; // synchronous
                                                             // success

        public static final int PIM_EVENT_SYNC_INIT = 18;
        public static final int PIM_EVENT_SYNC_NOT_ENOUGH = 19; // synchronous
                                                                // mem not
                                                                // enough
        public static final int PIM_EVENT_SERVER_TIMEOUT = 20; // server is
                                                               // timeout
        public static final int PIM_EVENT_SERVER_HTTP500 = 21; // server return
                                                               // 500
        public static final int EVENT_MESSAGE_SHOW_SYNCING = 22;
        public static final int PIM_EVENT_STOP_SYNC = 23;
        public static final int PIM_EVENT_PHONEBOOK_NOT_ENOUGH_SPACE = 24; // no
                                                                           // enough
                                                                           // memory
                                                                           // for
                                                                           // phonebook

        public static final int PIM_EVENT_AUTH_NEED_USRNAME_PWD = 25; // zhl add
                                                                      // for
                                                                      // input
                                                                      // usrname
                                                                      // and pwd
        public static final int PIM_EVENT_SERVER_SPACE_NOT_ENOUGH = 26; // Server
                                                                        // space
                                                                        // not
                                                                        // enough
        public static final int PIM_EVENT_SERVER_SYNC_ERROR = 27; // Server
                                                                  // space not
                                                                  // enough
        public static final int PIM_EVENT_SYNC_STARTED = 28;
        public static final int PIM_EVENT_INCOMPLETED_CMD = 29;
    };

    public class pim_item_status {
        public static final int PIMDEF_ADDED = 0;
        public static final int PIMDEF_DELETED = 1;
        public static final int PIMDEF_MODIFIED = 2;

    };

    public class PIMDEF_PB_STATUS {
        public static final int PIMPB_NO_USE = 0;
        public static final int PIMPB_STATE_ALL = 1;
        public static final int PIMPB_STATE_SYNCED = 2;
        public static final int PIMPB_STATE_ADDED = 3;
        public static final int PIMPB_STATE_REPLACED = 4;
        public static final int PIMPB_STATE_DELETED = 5;
        public static final int PIMPB_STATE_INVALID = 6;

    };

    public class operationStatus {
        public static final int Idless = 0;
        public static final int Synced = 1;
        public static final int Backup = 2;
        public static final int Resume = 3;

    };

    public class SmlEncoding_t {
        public static final int SML_UNDEF = 0;
        public static final int SML_WBXML = 1;
        public static final int SML_XML = 2;

    };

    public class auth_type {
        public static final int AUTH_DUMMY = 0;
        public static final int AUTH_B64 = 1;
        public static final int AUTH_MD5 = 2;

    };
    /*
     * public class PIM_T { boolean is_init; boolean is_proxy; boolean
     * is_alreadyReceive; boolean is_serverTimeout; boolean is_requestTimeout;
     * boolean lastSyncFlag; int Test_xmlNo; char Test_xmlTime[] = new char[15];
     * int m_step_bak; int m_step_priv_bak; int m_receive_pakage_counter;
     * PIM_EVENT g_event; pim_sync_result g_sync_result; auth_type m_authtype;
     * SmlEncoding_t m_sml_encoding_type; int m_sync_max_msg_size; int
     * m_sync_single_VCard_size; int m_sync_single_msg_reserve;
     * net_setting_info_type m_net_setting; proxy_setting_info_type
     * m_proxy_setting; server_setting_info_type m_server_setting;
     * wap_url_info_type m_url_setting; imei_info_type m_imei_info; pim_anchor
     * m_anchor; int m_synctype; int m_operationType; pim_syncTime m_syncTime;
     * };
     */
}
