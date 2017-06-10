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

package com.android.backup.vmsg;

import java.util.Date;
import android.text.TextUtils;
import android.util.Log;

public class ShortMessage implements Comparable<ShortMessage> {
    private static final String TAG = "ShortMessage";
    private Date date;
    private String from;
    private String to;
    private String body;
    private String read = "READ";
    private String boxType = "INBOX";
    private int locked = 0;
    private int subId;
    private static final String READ_STATUS = "READ";
    private static final String UNREAD_STATUS = "UNREAD";

    public ShortMessage(Date date, String from, String to,
            String body, String read, String boxType, int locked, int mSubId)
    {
        this.date = date;
        this.from = from;
        this.to = to;
        this.body = body;
        this.read = read;
        this.boxType = boxType;
        this.locked = locked;
        this.subId = mSubId;
    }

    public String getBody() {
        return body;
    }

    public Date getDate() {
        return date;
    }

    public String getFrom() {
        return from;
    }

    public String getTo() {
        return to;
    }

    public String getBoxType()
    {
        return boxType;
    }

    public int getLocked() {
        //
        Log.d(TAG, "Locked = " + locked);
        return locked;
    }

    public boolean isRead()
    {
        if (TextUtils.isEmpty(read))
        {
            return true;
        }
        else if (read.equalsIgnoreCase(UNREAD_STATUS))
        {
            return false;
        }
        return true;
    }

    public int compareTo(ShortMessage anotherMessage) {
        return date.compareTo(anotherMessage.date);
    }

    public int getSubId() {
        return subId;
    }
}
