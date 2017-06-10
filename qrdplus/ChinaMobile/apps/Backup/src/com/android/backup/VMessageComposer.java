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

package com.android.backup;

import java.nio.charset.Charset;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.NoSuchElementException;
import android.util.Log;

import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.io.FileNotFoundException;
import java.io.File;
import com.android.backup.vmsg.*;

import android.text.format.Time;
import android.content.Context;
import org.apache.commons.codec.net.QuotedPrintableCodec;//sunxiaoming add for function convertToQpEncoding()
import android.text.format.DateUtils;
import android.text.TextUtils;//sxm add 20121021

public class VMessageComposer
{
    private static final String TAG = "VMessageComposer";
    private static final String BEGIN_MARKER = "BEGIN:";
    private static final String END_MARKER = "END:";
    private static final String VMSG_TAG = "VMSG";
    private static final String VENV_TAG = "VENV";
    private static final String VCARD_TAG = "VCARD";
    private static final String VBODY_TAG = "VBODY";
    private static final String TEL_TAG = "TEL:";
    private static final String NAME_TAG = "N:";
    private static final String DATE_TAG = "Date:";
    private static final String TEXT_TAG = "Text:";
    private static final String SUBJECT_TAG = "Subject";
    private static final String STATE_TAG = "X-IRMC-STATUS:";
    private static final String STATE_TAG2 = "X-READ:";
    private static final String MAILBOX_TAG = "X-IRMC-BOX:";
    private static final String MAILBOX_TAG2 = "X-BOX:";
    private static final String SIMID_TAG2 = "X-SIMID:";
    private static final String LOCKED_TAG2 = "X-LOCKED:";
    private static final String TYPE_TAG2 = "X-TYPE:";
    private static final String VERSION = "VERSION:";
    private static final String VMESSAGEVERSION = "1.1";
    private static final String VCARDVERSION = "2.1";
    private static final String INBOX_TAG = "INBOX";
    private static final String SENT_TAG = "SENDBOX";
    private static final String DRAFT_TAG = "DRAFT";
    private static final String READ_TAG = "READ";
    private static final String UNREAD_TAG = "UNREAD";

    private static final int INBOX = 1;
    private static final int SENT = 2;
    private static final int DRAFT = 3;

    private static final String BACKUP_FILE = "/sms.vmsg";
    private static final String BACKUP_DIR = BackupUtils.strSDCardPath + "/others";

    private OutputStream mOutputStream = null;
    private BufferedOutputStream mWriter = null;
    private static Context mContext;

    private boolean DBG = true;

    public boolean onInit(String dir, Context context)
    {
        mContext = context;
        if (dir == null) {
            return false;
        }
        String filePath = dir + BACKUP_FILE;
        Log.d(TAG, "filepath " + filePath);

        try
        {
            File outfile = new File(filePath);
            if (outfile == null)
            {
                Log.d(TAG, "new file error ");
                return false;
            }
            outfile.createNewFile();
            mOutputStream = new FileOutputStream(outfile);
            mWriter = new BufferedOutputStream(mOutputStream);
            if (mWriter == null)
            {
                Log.d(TAG, "mWriter create error ");
                return false;
            }

        } catch (java.io.FileNotFoundException e)
        {
            Log.d(TAG, "FileNotFoundException " + e);
            return false;
        } catch (java.io.IOException e)
        {
            Log.d(TAG, "IOException " + e);
            return false;
        }
        return true;
    }

    public void addOneMessageNew(String body, String address, String date, int type,
            String subject, int locked, boolean read, int subId)
    {
        byte[] tmpBuf = null;
        if (mWriter == null)
        {
            Log.d(TAG, "Init failed");
            return;
        }
        try
        {
            tmpBuf = (BEGIN_MARKER + VMSG_TAG).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// BEGIN:VMSG
            mWriter.write('\n');

            tmpBuf = (VERSION + VMESSAGEVERSION).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// VERSION:1.1
            mWriter.write('\n');

            tmpBuf = (BEGIN_MARKER + VCARD_TAG).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// BEGIN:VCARD
            mWriter.write('\n');

            if (type == SENT || type == DRAFT)
            {
                tmpBuf = (TEL_TAG + address).getBytes();
                mWriter.write(tmpBuf, 0, tmpBuf.length);
                mWriter.write('\n');
            }
            else
            {
                tmpBuf = (TEL_TAG + address).getBytes();
                mWriter.write(tmpBuf, 0, tmpBuf.length);// TEL:10086
                mWriter.write('\n');
            }

            tmpBuf = (END_MARKER + VCARD_TAG).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// END:VCARD
            mWriter.write('\n');

            tmpBuf = (BEGIN_MARKER + VBODY_TAG).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// BEGIN:VBODY
            mWriter.write('\n');

            switch (type)
            {
                case INBOX:
                    tmpBuf = (MAILBOX_TAG2 + INBOX_TAG).getBytes();
                    mWriter.write(tmpBuf, 0, tmpBuf.length);// X-BOX:INBOX
                    mWriter.write('\n');
                    break;
                case SENT:
                    tmpBuf = (MAILBOX_TAG2 + SENT_TAG).getBytes();
                    mWriter.write(tmpBuf, 0, tmpBuf.length);// X-BOX:SENT
                    mWriter.write('\n');
                    break;
                case DRAFT:
                    tmpBuf = (MAILBOX_TAG2 + DRAFT_TAG).getBytes();
                    mWriter.write(tmpBuf, 0, tmpBuf.length);// X-BOX:DRAFT
                    mWriter.write('\n');
                    break;
                default:
                    Log.d(TAG, "z188  type=" + type);
                    break;
            }

            if (read) {
                tmpBuf = (STATE_TAG2 + READ_TAG).getBytes();
                mWriter.write(tmpBuf, 0, tmpBuf.length);// X-STATUS:READ
                mWriter.write('\n');
            } else {
                tmpBuf = (STATE_TAG2 + UNREAD_TAG).getBytes();
                mWriter.write(tmpBuf, 0, tmpBuf.length);// X-STATUS:UNREAD
                mWriter.write('\n');
            }
            int mSubId = 0;
            if (subId == 1) {
                mSubId = 1;
            }
            tmpBuf = (SIMID_TAG2 + mSubId).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// X-SIMID:0
            mWriter.write('\n');
            String mLocked = "UNLOCKED";
            if (1 == locked) {
                mLocked = "LOCKED";
            }
            tmpBuf = (LOCKED_TAG2 + mLocked).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// X-LOCKED:0
            mWriter.write('\n');

            tmpBuf = (TYPE_TAG2 + "SMS").getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// X-TYPE:SMS
            mWriter.write('\n');

            tmpBuf = (DATE_TAG + date).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// Date:25.9.2009 08:12:11
            mWriter.write('\n');

            tmpBuf = (SUBJECT_TAG + convertToQpEncoding(body)).getBytes();
            if (tmpBuf.length > 0) {
                mWriter.write(tmpBuf, 0, tmpBuf.length);// subject ,sunxiaoming
                                                        // add, in sms ,the
                                                        // subject always be
                                                        // null
            }
            mWriter.write('\n');

            tmpBuf = (END_MARKER + VBODY_TAG).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// END:VBODY
            mWriter.write('\n');

            tmpBuf = (END_MARKER + VMSG_TAG).getBytes();
            mWriter.write(tmpBuf, 0, tmpBuf.length);// END:VMSG
            mWriter.write('\n');
            // mWriter.write(0x00);
        } catch (IOException e)
        {
            Log.d(TAG, "IOException: " + e);
        }

    }

    public boolean flushToSDCard()
    {
        if (mWriter == null)
        {
            Log.d(TAG, "mWriter == null");
            return false;
        }
        try
        {
            mWriter.flush();
            mWriter.close();
            mOutputStream.close();
        } catch (IOException e)
        {
            Log.d(TAG, "IOException: " + e);
            return false;
        }
        return true;
    }

    private String convertToQpEncoding(String originalString)
    {
        QuotedPrintableCodec qpc = new QuotedPrintableCodec();
        String rStr = null;
        String mEncodeStr = null;
        mEncodeStr = encodeQuotedPrintable(originalString);
        try {
            // rStr =
            // ";ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:"+qpc.encode(originalString);
            rStr = ";ENCODING=QUOTED-PRINTABLE;CHARSET=UTF-8:" + mEncodeStr;
        } catch (Exception e) {
            Log.e("VCalComposer", "Failed to decode quoted-printable: " + e);
        }
        return rStr;
    }

    private String encodeQuotedPrintable(final String str) {
        if (TextUtils.isEmpty(str)) {
            return "";
        }

        final StringBuilder builder = new StringBuilder();
        int index = 0;
        int lineCount = 0;
        byte[] strArray = null;
        strArray = str.getBytes();
        while (index < strArray.length) {
            builder.append(String.format("=%02X", strArray[index]));
            index += 1;
            lineCount += 3;

            if (lineCount >= 67) {
                // Specification requires CRLF must be inserted before the
                // length of the line
                // becomes more than 76.
                // Assuming that the next character is a multi-byte character,
                // it will become
                // 6 bytes.
                // 76 - 6 - 3 = 67
                builder.append("=\r\n");
                lineCount = 0;
            }
        }

        return builder.toString();
    }

    // sunxiaoming add ,we can format the timestamp into the chinamobile
    // -standard
    public static String formatTimeStampString(long when, boolean fullFormat) {
        Time then = new Time();
        then.set(when);
        Time now = new Time();
        now.setToNow();

        // Basic settings for formatDateTime() we want for all cases.
        int format_flags = DateUtils.FORMAT_NO_NOON_MIDNIGHT |
                DateUtils.FORMAT_ABBREV_ALL |
                DateUtils.FORMAT_CAP_AMPM;

        // If the message is from a different year, show the date and year.
        if (then.year != now.year) {
            format_flags |= DateUtils.FORMAT_SHOW_YEAR | DateUtils.FORMAT_SHOW_DATE;
        } else if (then.yearDay != now.yearDay) {
            // If it is from a different day than today, show only the date.
            format_flags |= DateUtils.FORMAT_SHOW_DATE;
        } else {
            // Otherwise, if the message is from today, show the time.
            format_flags |= DateUtils.FORMAT_SHOW_TIME;
        }

        // If the caller has asked for full details, make sure to show the date
        // and time no matter what we've determined above (but still make
        // showing
        // the year only happen if it is a different year from today).
        if (fullFormat) {
            format_flags |= (DateUtils.FORMAT_SHOW_DATE | DateUtils.FORMAT_SHOW_TIME);
        }

        return DateUtils.formatDateTime(mContext, when, format_flags);
    }

}
