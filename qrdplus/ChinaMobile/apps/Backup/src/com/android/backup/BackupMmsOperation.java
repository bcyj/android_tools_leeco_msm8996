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

import static com.google.android.mms.pdu.PduHeaders.MESSAGE_TYPE_SEND_REQ;
import static com.google.android.mms.pdu.PduHeaders.MESSAGE_TYPE_RETRIEVE_CONF;

import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.provider.Telephony.Sms.Inbox;
import android.provider.Telephony.Sms;
import android.database.Cursor;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ByteArrayOutputStream;
import java.text.SimpleDateFormat;
import com.android.backup.vmsg.ShortMessage;
import com.android.backup.vmsg.MessageParsers;
import com.android.backup.vcalendar.VCalendarManager;
import com.android.backup.vcalendar.VcalendarImporter;
import android.net.Uri;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Set;

import android.os.Handler;

import android.provider.Telephony.Mms;
import android.provider.BaseColumns;
import android.provider.Telephony.Mms.Outbox;
import android.provider.Telephony.Mms.Sent;
import android.provider.Telephony.Mms.Draft;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.GenericPdu;
import com.google.android.mms.pdu.PduPersister;
import com.google.android.mms.pdu.PduComposer;
import com.google.android.mms.pdu.SendReq;
import com.google.android.mms.pdu.PduParser;
import com.google.android.mms.pdu.RetrieveConf;
import com.google.android.mms.pdu.PduHeaders;

import android.telephony.SmsMessage;
import java.util.TimeZone;
import android.telephony.SmsManager;
import android.text.TextUtils;
import android.text.format.Time;
import com.android.internal.telephony.IccUtils;
import android.util.Log;
import android.widget.Toast;
//xml
import android.util.Xml;
import org.xmlpull.v1.XmlSerializer;
import java.io.StringWriter;
import java.io.OutputStreamWriter;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import org.xmlpull.v1.XmlPullParser;
import java.io.InputStream;
import java.io.FileReader;
//xml
import static com.android.backup.BackupUtils.*;
import static com.android.backup.BackupUtils.BACKUP_TYPE_MMS;
import static com.android.backup.BackupUtils.BACKUP_TYPE_SMS;
import static com.android.backup.BackupUtils.EVENT_INIT_PROGRESS_TITLE;
import static com.android.backup.BackupUtils.EVENT_SET_PROGRESS_VALUE;
import static com.android.backup.BackupUtils.EVENT_SDCARD_NO_SPACE;
import static com.android.backup.BackupUtils.EVENT_SDCARD_FULL;

public class BackupMmsOperation {
    private static final String TAG = "BackupMmsOperation";
    private static final boolean DBG = true;
    private static final Uri BACKUPSMS_URI = Uri.parse("content://sms");
    private static final Uri BACKUPMMSMSG_URI = Uri.parse("content://mms");
    private static final Uri GET_MMS_PDU_URI = Uri.parse("content://mms/get-pdu");
    private static final Uri MAILBOX_MESSAGES_COUNT = Uri.parse("content://mms-sms/messagescount");
    private static final Uri SMS_MESSAGES_COUNT = Uri.parse("content://mms-sms/smscount");
    private static final int MAX_MMS_SMS_COUNT = 2000;

    private static final String TAG_RECORD_COUNT = "mmsCount";
    private static final String TAG_RECORD = "record";
    private static final String ITEM_COUNT = "count";
    private static final String ITEM_ID = "_id";
    private static final String ITEM_PATH = "pdu_path";
    private static final String ITEM_ISREAD = "isread";
    private static final String ITEM_MSG_BOX = "msg_box";
    private static final String ITEM_DATE = "date";
    private static final String ITEM_M_SIZE = "m_size";
    private static final String ITEM_LOCKED = "islocked";
    private static final String ITEM_SIMID = "sim_id";
    private static final String MMS_XML_NAME = "mms_backup.xml";

    private static final String PDU_ID = "_id";
    private static final String PDU_PATH = "pdu_path";
    private static final String PDU_MSG_BOX = "msg_box";
    private static final String PDU_READ = "read";
    private static final String PDU_DATE = "date";
    private static final String PDU_M_SIZE = "m_size";
    private static final String PDU_LOCKED = "locked";
    private static final String PDU_PHONE_ID = "phone_id";

    static final String[] MAILBOX_PROJECTION = new String[] {
            BaseColumns._ID,
            Mms.MESSAGE_BOX,
            Mms.READ,
            Mms.DATE,
            Mms.LOCKED,
            Mms.MESSAGE_SIZE,
            "phone_id",
    };

    private static int MAX_INSERT_COUNT = 10;

    private ShortMessage[] mMessages;
    boolean mCanceled = false;
    private final Object mCursorLock = new Object();
    private static final int QUERY_BACKUP_SMS_TOKEN = 1;
    private static final int QUERY_BACKUP_MMS_TOKEN = 2;
    private ContentResolver mContentResolver;
    private Context mContext;
    private Handler mCallBackHandler;
    private String mMmsBackupDir;
    private String mPath;
    private int smsCount = 0;
    private int mmsCount = 0;
    private int mSlotId = BackupUtils.SLOT_ALL;

    public BackupMmsOperation(Context context, Handler callbackHandler, String dir) {
        mContext = context;
        mCallBackHandler = callbackHandler;
        mMmsBackupDir = dir;
    }

    public BackupMmsOperation(Context context, Handler callbackHandler, String dir, int slot, String path) {
        this(context, callbackHandler, dir);
        mSlotId = slot;
        mPath = path;
    }

    private void BackupMsg(Cursor cursor, int token)
    {
        switch (token) {
            case QUERY_BACKUP_SMS_TOKEN: {
                Log.d(TAG, "onQueryComplete QUERY_BACKUP_SMS_TOKEN");
                if (cursor != null) {
                    Log.d(TAG, "cursor.getCount()=" + cursor.getCount());
                    VMessageComposer composer = new VMessageComposer();
                    if (cursor.moveToFirst()) {
                        if (mMmsBackupDir != null) {
                            composer.onInit(mMmsBackupDir, mContext);
                        }
                    } else {
                        Log.d(TAG, "backup message query null!");
                        return;
                    }

                    int size = cursor.getCount();
                    smsCount = size;
                    ((BackupService) mContext).sendBackupMessage(
                            EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_SMS, size);
                    int i = 1;
                    int myType = 1;
                    while (!cursor.isAfterLast()) {
                        if (DBG) {
                            Log.d(TAG, " " + i);
                        }
                        if (((BackupService) mContext).getCancelBackupFlag()) {
                            return;
                        }
                        try {
                            // Sdcard full
                            if (mPath.equals(BackupUtils.strExternalPath)) {
                                if (BackupUtils.isTfCardFull()) {
                                    ((BackupService) mContext).sendBackupMessage(
                                            EVENT_SDCARD_NO_SPACE, 0, 0);
                                    ((BackupService) mContext).setCancelBackupFlag(true);
                                    return;
                                }
                            } else {
                                if (BackupUtils.isInternalFull()) {
                                    ((BackupService) mContext).sendBackupMessage(
                                            EVENT_SDCARD_NO_SPACE, 0, 0);
                                    ((BackupService) mContext).setCancelBackupFlag(true);
                                    return;
                                }
                            }
                        } catch (IllegalArgumentException e) {
                            Log.v(TAG, "Backup Sms: " + e.getMessage());
                            return;
                        }
                        // address,X-BOX,X-READ,X-SIMID,X-LOCKED,X-TYPE,DATE,SUBJECT,
                        // we lost simID,locked,type,subject
                        String address = cursor.getString(0);
                        String body = cursor.getString(1);
                        long date = cursor.getLong(2);
                        int type = cursor.getInt(3);
                        int read = cursor.getInt(4);
                        int locked = cursor.getInt(5);
                        int subId = cursor.getInt(6);
                        Log.d(TAG, "Date=" + date + " read=" + read);
                        Date strdate = new Date(date);
                        String timeFormat = "yyyy/MM/dd HH:mm:ss";
                        SimpleDateFormat sdf = new SimpleDateFormat(timeFormat);
                        String formatDate = "";
                        formatDate = sdf.format(strdate).toString();
                        Log.d(TAG, "FormatDate=" + formatDate);

                        composer.addOneMessageNew(body, address, formatDate, type, "", locked,
                                read != 0, subId);
                        ((BackupService) mContext).sendBackupMessage(
                                EVENT_SET_PROGRESS_VALUE, i, 0);
                        cursor.moveToNext();
                        i++;
                    }
                    composer.flushToSDCard();
                    if (cursor != null) {
                        cursor.close();
                    }

                }
                else
                {
                    Log.d(TAG, "Cannot init the cursor for the backup messages.");
                    if (cursor != null) {
                        cursor.close();
                    }
                }
                break;
            } // End QUERY_BACKUP_SMS_TOKEN
        }
    }

    /**
     * {@link ThreadFactory} which sets a meaningful name for the thread.
     */
    private static class BackgroundThreadFactory implements ThreadFactory {
        private final AtomicInteger mCount = new AtomicInteger(1);
        private final String mTag;

        public BackgroundThreadFactory(String tag) {
            mTag = tag;
        }

        public Thread newThread(final Runnable r) {
            Thread t =  new Thread(r, mTag + "-" + mCount.getAndIncrement());

            return t;
        }
    }

    private static final int MAX_QUERY_COUNT = 10;
    private static final int MAX_NUM_PER_TASK = MAX_QUERY_COUNT;
    private static final int POOL_SIZE = 10;
    private static final int RECORD_START_INDEX = 0;
    private final AtomicInteger mTaskCount = new AtomicInteger(0);

    ThreadPoolExecutor mExecutor = new ThreadPoolExecutor(
            POOL_SIZE, POOL_SIZE, 5, TimeUnit.SECONDS,
            new LinkedBlockingQueue<Runnable>(),
            new BackgroundThreadFactory(TAG));

    private void pushTask(Runnable r) {
        if (mExecutor != null) {
            mExecutor.execute(r);
        } else {
            new Thread(r).start();
        }
    }

    public class PduTask implements Runnable {
        private ContentValues mValues;

        public PduTask(ContentValues values) {
            mTaskCount.incrementAndGet();
            mValues = values;
        }

        /** {@inheritDoc} */
        public void run() {
            try {
                Set<String> keys = mValues.keySet();
                for (String key : keys) {
                    if (!isContinue()) {
                        return;
                    }
                    writeData(key, mValues.getAsByteArray(key));
                }
                ((BackupService) mContext).sendBackupMessage(
                        EVENT_SET_PROGRESS_VALUE,
                        BackupActivity.INVALID_ARGUMENT, keys.size());
            } finally {
                mTaskCount.decrementAndGet();
            }
        }

        private void writeData(final String path, final byte[] pduData) {
            FileOutputStream fout = null;
            String fileName = null;
            try {
                File file = new File(path);
                fout = new FileOutputStream(file);
                fout.write(pduData);
                fileName = file.getName();
            } catch (IOException e) {
                Log.d(TAG, "writeData IOException" + e);
            } finally {
                if (null != fout) {
                    try {
                        fout.close();
                    } catch (IOException e) {
                    }
                }
            }
        }
    }

    private static String getDBRow(String recordRow) {
        if (ITEM_ID.equalsIgnoreCase(recordRow)) {
            return PDU_PATH;
        } else if (ITEM_ISREAD.equalsIgnoreCase(recordRow)) {
            return PDU_READ;
        } else if (ITEM_LOCKED.equalsIgnoreCase(recordRow)) {
            return PDU_LOCKED;
        } else if (ITEM_M_SIZE.equalsIgnoreCase(recordRow)) {
            return PDU_M_SIZE;
        } else if (ITEM_SIMID.equalsIgnoreCase(recordRow)) {
            return PDU_PHONE_ID;
        } else if (ITEM_MSG_BOX.equalsIgnoreCase(recordRow)) {
            return PDU_MSG_BOX;
        } else if (ITEM_DATE.equalsIgnoreCase(recordRow)) {
            return PDU_DATE;
        } else {
            Log.d(TAG, "Don't support record row :" + recordRow);
            return null;
        }
    }

    private static String getRecordRow(String dbRow) {
        if (PDU_ID.equalsIgnoreCase(dbRow)) {
            return ITEM_ID;
        } else if (PDU_READ.equalsIgnoreCase(dbRow)) {
            return ITEM_ISREAD;
        } else if (PDU_LOCKED.equalsIgnoreCase(dbRow)) {
            return ITEM_LOCKED;
        } else if (PDU_M_SIZE.equalsIgnoreCase(dbRow)) {
            return ITEM_M_SIZE;
        } else if (PDU_PHONE_ID.equalsIgnoreCase(dbRow)) {
            return ITEM_SIMID;
        } else if (PDU_MSG_BOX.equalsIgnoreCase(dbRow)) {
            return ITEM_MSG_BOX;
        } else if (PDU_DATE.equalsIgnoreCase(dbRow)) {
            return ITEM_DATE;
        } else {
            Log.d(TAG, "Don't support db row :" + dbRow);
            return null;
        }
    }

    /**
     * Save pdu data to path within providers
     *
     * @param where set of message id, message type and pdu path with format
     *            like msgId1:msgType1:pduPath1:msgId2:msgType2:..
     * @param itemCount the row count of where.
     * @param dataCount the columnt count fo where.
     */
    private boolean savePduData(String where, int itemCount, int dataCount) {
        Cursor cursor = null;
        Uri url = GET_MMS_PDU_URI.buildUpon()
                .appendQueryParameter("item_count", "" + itemCount)
                .appendQueryParameter("data_count", "" + dataCount)
                .appendQueryParameter("data_split", DATA_SPILT)
                .appendQueryParameter("data", where).build();
        try {
            cursor = mContext.getContentResolver().query(
                    url, null, null, null, null);
            if (cursor != null && cursor.getCount() != 0) {
                // There are three row of cursor: msgId, pduPath, pduData
                // We save the pduPath and pduData into ContentValues and put
                // the writing data task to task queue
                ContentValues values = new ContentValues(MAX_NUM_PER_TASK);
                int i = 0;
                while (cursor.moveToNext() && isContinue()) {
                    values.put(cursor.getString(1), cursor.getBlob(2));
                    if (++i % MAX_NUM_PER_TASK == 0) {
                        pushTask(new PduTask(values));
                        values = new ContentValues(MAX_NUM_PER_TASK);
                    }
                }

                if (values.size() > 0) {
                    pushTask(new PduTask(values));
                }
            } else {
                Log.e(TAG, "Can't get Pdu data with:" + where);
            }
        } catch (Exception e) {
            Log.e(TAG, "getPdu data exception:", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return false;
    }

    private String getPduPath(int index) {
        return mMmsBackupDir + "/" + index + ".pdu";
    }

    private final static String DATA_SPILT = ":";

    private MmsRecordList getMmsRecordList(Cursor cursor, int slotId) {
        if (cursor == null || cursor.getCount() == 0) {
            return null;
        }

        MmsRecordList recordList = new MmsRecordList(slotId);
        while (cursor.moveToNext() && isContinue()) {
            ContentValues mmsRecord = new ContentValues();
            for (int i = 0; i < MAILBOX_PROJECTION.length; i++) {
                mmsRecord.put(MAILBOX_PROJECTION[i], cursor.getString(i));
            }
            recordList.addMmsRecord(mmsRecord);
        }

        if (!isContinue() || recordList.size() == 0) {
            Log.d(TAG, "empty record.");
            recordList.clear();
            return null;
        }
        return recordList;
    }

    private long getSpentTime(long start) {
        return System.currentTimeMillis() - start;
    }

    private boolean writeMmsRecord(XmlSerializer serializer, ContentValues values) {
        try {
            Set<String> dbColumns = values.keySet();
            for (String dbColumn : dbColumns) {
                serializer.attribute("", getRecordRow(dbColumn),
                        values.getAsString(dbColumn));
            }
            return true;
        } catch (Exception e) {
            Log.d(TAG, "writeMmsRecord Exception" + e);
        }
        return false;
    }

    private int writeMmsRecords(XmlSerializer serializer, List<ContentValues> records, int start) {
        int i = start;
        StringBuilder where = new StringBuilder();
        try {
            for (ContentValues values: records){
                if (!isContinue()) {
                    return i;
                }
                serializer.startTag("", "record");
                String msgId = values.getAsString(PDU_ID);
                String msgBox = values.getAsString(PDU_MSG_BOX);
                String pduPath = getPduPath(i);

                where.append(msgId);
                where.append(DATA_SPILT + msgBox);
                where.append(DATA_SPILT + pduPath);
                where.append(DATA_SPILT);
                serializer.attribute("", getRecordRow(PDU_ID), "" + i + ".pdu");
                values.remove(PDU_ID);
                writeMmsRecord(serializer, values);
                serializer.endTag("", "record");
                i++;
                if (i % MAX_QUERY_COUNT == 0) {
                    savePduData(where.toString(), 3, MAX_QUERY_COUNT);
                    where.delete(0, where.length());
                }
            }

            if (i % MAX_QUERY_COUNT != 0) {
                // save the left message pdu
                savePduData(where.toString(), 3, i % MAX_QUERY_COUNT);
                where.delete(0, where.length());
            }
        } catch (Exception e) {
            Log.e(TAG, "write records exception :", e);
        }
        return i;
    }

    private boolean writeMmsRecords(XmlSerializer serializer, MmsRecordList mmsRecordList) {
        long start = System.currentTimeMillis();
        if (serializer == null || mmsRecordList == null) {
            return false;
        }

        int size = writeMmsRecords(serializer, mmsRecordList.mInBoxRecordList,
                RECORD_START_INDEX);
        size = writeMmsRecords(serializer, mmsRecordList.mSendBoxRecordList,
                size);
        Log.d(TAG, "finish backup  :" + mmsRecordList.size()
                + " mms using:" + getSpentTime(start));
        return (size == mmsRecordList.size());
    }

    public boolean backupMmsMessages() {
        long start_time = System.currentTimeMillis();
        boolean success = false;
        String selection = generateMMSSelection();
        Cursor cursor = mContext.getContentResolver().query(BACKUPMMSMSG_URI, MAILBOX_PROJECTION,
                selection, null, null);
        XmlSerializer serializer = Xml.newSerializer();
        StringWriter writer = new StringWriter();
        if (cursor != null) {
            ((BackupService) mContext).sendBackupMessage(
                    EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_MMS, cursor.getCount());
            try {
                serializer.setOutput(writer);
                serializer.startDocument("UTF-8", true);
                serializer.startTag("", "mms");
                serializer.startTag("", "mmsCount");
                serializer.attribute("", "count", String.valueOf(cursor.getCount()));
                serializer.endTag("", "mmsCount");
                success = writeMmsRecords(serializer, getMmsRecordList(cursor, mSlotId));
                serializer.endTag("", "mms");
                serializer.endDocument();
            } catch (Exception e) {
                Log.e(TAG, "backup exception :", e);
            } finally {
                cursor.close();
            }
        }

        if (isContinue() && success) {
            try {
                File msgXmlS = new File(mMmsBackupDir + "/" + MMS_XML_NAME);
                if (!msgXmlS.exists()) {
                    msgXmlS.createNewFile();
                }
                final FileOutputStream out = new FileOutputStream(msgXmlS);
                out.write(writer.toString().getBytes());
                out.flush();
                out.close();
            } catch (Exception e) {
            }
        }

        int unFinishCount = mTaskCount.get();
        while (unFinishCount != 0) {
            try {
                Thread.sleep(500);
            } catch (Exception e) {
            }
            unFinishCount = mTaskCount.get();
            Log.d(TAG, "running task count :" + unFinishCount);
        }
        Log.d(TAG, "bakcup finish write xml :" + getSpentTime(start_time));
        return success;
    }

    public void backupSmsMessages()
    {
        final String[] smsProjection = new String[] {
                Sms.ADDRESS,
                Sms.BODY,
                Sms.DATE,
                Sms.TYPE,
                Sms.READ,
                Sms.LOCKED,
                "phone_id",
        };
        final String smsSelection = generateSMSSelection();
        Cursor cr = mContext.getContentResolver().query(BACKUPSMS_URI, smsProjection, smsSelection,
                null, null);
        if (DBG) {
            Log.d(TAG, "cr = " + cr.toString() + " cr.count = " + cr.getCount());
        }
        BackupMsg(cr, QUERY_BACKUP_SMS_TOKEN);
    }

    private String generateSMSSelection() {
        String selection = " type=1 OR type=2";
        if (mSlotId != BackupUtils.SLOT_ALL) {
            selection = " (type=1 OR type=2) AND (phone_id=" + mSlotId + ") ";
        }
        return selection;
    }

    public void recoverSmsMessages(String directory) {
        int msgCount = 0;
        if (isCMCCTest()) {
            Log.d(TAG, "isCMCCTest");
            final Cursor c = mContext.getContentResolver().query(
                    MAILBOX_MESSAGES_COUNT, null, null, null, null);
            if (c != null) {
                try {
                    if (c.moveToFirst()) {
                        msgCount = c.getInt(0);
                    }
                } finally {
                    c.close();
                }
            }
        }
        Log.d(TAG, "addMsg msgCount: " + msgCount);
        if (directory == null) {
            return;
        }
        if (DBG)
            Log.d(TAG, "recoverSmsMessages directory: " + directory);

        File dir = new File(directory);
        if (!dir.exists()) {
            return;
        }
        File[] smsFileList = dir.listFiles();
        int num = smsFileList.length;
        for (int i = 0; i < num; i++) {
            if (((RecoverOperateActivity) mContext).getCancelRestoreFlag()) {
                Log.d(TAG, "addMsg cancelled");
                return;
            }
            try {
                File smsFile = smsFileList[i];
                if (smsFile.isDirectory()) {
                    continue;
                }
                if (smsFile.toString().endsWith(".xml")) {
                    continue;
                }
                String myType = smsFile.toString();
                String mailbox = "inbox";
                int len = (int) smsFile.length();
                byte[] data = new byte[len];
                FileInputStream fis = new FileInputStream(smsFile);
                fis.read(data);
                SmsMessage mSmsMsg = null;
                int mti = 0;
                int read = 1;
                int mFlag = data[0] & 0xff;
                if (mFlag != 5 && mFlag != 1 && mFlag != 3 && mFlag != 7) {
                    int mLen = data[0] & 0xff;
                    int myLen = mLen + 1;
                    mti = data[myLen] & 0x3;
                    mSmsMsg = SmsMessage.createFromPdu(data);
                } else {
                    if (1 == mFlag) {
                        mti = 0;
                        read = 1;
                    } else if (3 == mFlag) {
                        mti = 0;
                        read = 0;
                    } else if (5 == mFlag) {
                        mti = 1;
                    } else if (7 == mFlag) {
                        mti = 2;
                    }
                    byte[] mData = new byte[len - 1];
                    for (int j = 1; j < len; j++) {
                        mData[j - 1] = data[j];
                    }
                    mSmsMsg = SmsMessage.createFromPdu(mData);
                    if (DBG)
                        Log.d(TAG, "pdu = " + IccUtils.bytesToHexString(mData));
                }
                if (DBG) {
                    Log.d(TAG, "pdu = " + IccUtils.bytesToHexString(data));
                    Log.d(TAG, "mti = " + mti);
                }
                String srcAddress = mSmsMsg.getOriginatingAddress();
                String desAdress = "";// wrong
                String body = mSmsMsg.getMessageBody();
                boolean isRead = true;
                ContentValues values = new ContentValues();
                if (DBG)
                    Log.d(TAG, "mti = " + mti);
                if (0 == mti) {
                    values.put("address", srcAddress);
                    mailbox = "inbox";
                }
                if (1 == mti) {
                    values.put("address", desAdress);
                    mailbox = "sent";
                }
                if (2 == mti) {
                    values.put("address", desAdress);
                    mailbox = "draft";
                }

                values.put("modem", 1);
                values.put("body", body);
                values.put("date", mSmsMsg.getTimestampMillis());
                values.put(Inbox.READ, read);

                Log.d(TAG, " srcAddress = " + srcAddress + " body = " + body + " desAdress = "
                        + desAdress + " mailbox =" + mailbox);
                Log.d(TAG, " i = " + i);

                String uriStr = "content://sms/" + mailbox;
                Log.d(TAG, " uriStr = " + uriStr);
                Uri msgUri = Uri.parse(uriStr);
                if (msgCount > MAX_MMS_SMS_COUNT && isCMCCTest()) {
                    Log.e(TAG, "msgCount = " + msgCount);
                    return;
                }

                try {
                    Uri resultUri = mContext.getContentResolver().insert(msgUri, values);
                    msgCount++;
                    Log.d(TAG, " resultUri = " + resultUri.toString());
                } catch (Exception e) {
                    Log.d(TAG, "recoverSmsMessages :insert error");
                }

            } catch (Exception e) {
                Log.d(TAG, "recoverSmsMessages :revocer error");
            }

        }
    }

    public void recoverSmsMessagesWithVmsg(String directory)
    {
        if (directory == null) {
            return;
        }
        Log.d(TAG, "recoverSmsMessages directory: " + directory);

        File dir = new File(directory);
        if (!dir.isDirectory()) {
            Log.d(TAG, "can not find dir /sdcard/others/");
            return;
        }

        try {
            System.out.println("Using directory: " + dir.getCanonicalPath());
        } catch (IOException e) {
        }

        File[] files = dir.listFiles(MessageParsers.getFilenameFilter());
        if (files.length == 0) {
            Log.e(TAG, "Not find vmsg files");
            return;
        }
        mMessages = parseMessages(files);
        if (mMessages != null)
        {
            // MessageUtils.printMmsLog("z1061 mMessages.length = " +
            // mMessages.length);
            resumeMessageToDatabase(mMessages.length);
        }
    }

    public void recoverSmsMessagesWithXML(String directory) {
        Log.d(TAG, "recoverMmsMessage directory: " + directory);

        File dir = new File(directory);
        if (!dir.isDirectory()) {
            Log.d(TAG, "can not find dir /sdcard/others/");
            return;
        }

        int msgCount = 0;
        final Cursor c = mContext.getContentResolver().query(
                MAILBOX_MESSAGES_COUNT, null, null, null, null);

        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    msgCount = c.getInt(0);
                }
            } finally {
                c.close();
            }
        }
        Log.d(TAG, "recoverMmsMessages msgCount: " + msgCount);

        InputStream inputStream = null;
        XmlPullParser xmlParser = Xml.newPullParser();
        try {
            Log.d(TAG, "start = " + directory
                    + "msg_box.xml");
            File fff = new File(directory + "msg_box.xml");
            Log.d(TAG, "fff.exists = " + fff.exists());
            if (!fff.exists())
            {
                recoverSmsMessages(directory);
                return;
            }
            xmlParser.setInput(new FileReader(fff));
            int evtType = xmlParser.getEventType();
            if (DBG)
                Log.d(TAG, "evtType = " + evtType);
            while (evtType != XmlPullParser.END_DOCUMENT) {

                switch (evtType) {
                    case XmlPullParser.START_TAG: {
                        String tag = xmlParser.getName();
                        if (tag.equalsIgnoreCase(TAG_RECORD)) {
                            String isReadS = xmlParser.getAttributeValue(null,
                                    ITEM_ISREAD);
                            int boxId = Integer.parseInt(xmlParser
                                    .getAttributeValue(null, ITEM_MSG_BOX));
                            long dateL = Long.parseLong(xmlParser
                                    .getAttributeValue(null, ITEM_DATE));
                            File pduFile = new File(directory + "/"
                                    + xmlParser.getAttributeValue(null, ITEM_ID));
                            byte[] data = new byte[(int) pduFile.length() - 1];
                            FileInputStream fin = null;
                            String mailbox = "inbox";
                            int mti = 0x01;
                            try {
                                fin = new FileInputStream(pduFile);
                                mti = fin.read();
                                fin.read(data);
                                SmsMessage mSmsMsg = null;
                                Log.d(TAG,
                                        "read pdu from file = " + IccUtils.bytesToHexString(data));
                                Log.d(TAG, "mti = " + mti);
                                mSmsMsg = SmsMessage.createFromPdu(data);
                                String body = mSmsMsg.getMessageBody();
                                boolean isRead = true;
                                ContentValues values = new ContentValues();
                                if (0x01 == mti) {
                                    values.put("address", mSmsMsg.getOriginatingAddress());
                                    mailbox = "inbox";
                                    values.put(Inbox.READ, 1);
                                } else if (0x03 == mti) {
                                    values.put("address", mSmsMsg.getOriginatingAddress());
                                    mailbox = "inbox";
                                    values.put(Inbox.READ, 0);
                                } else if (05 == mti) {
                                    values.put("address", mSmsMsg.getOriginatingAddress()); // need
                                                                                            // modify
                                    // mSmsMsg.getDisplayDestinationAddress());
                                    mailbox = "sent";
                                } else if (07 == mti) {
                                    values.put("address", mSmsMsg.getOriginatingAddress()); // need
                                                                                            // modify
                                    // mSmsMsg.getDisplayDestinationAddress());
                                    mailbox = "draft";
                                }
                                values.put("body", body);
                                values.put("date", dateL);
                                String uriStr = "content://sms/" + mailbox;
                                if (DBG) {
                                    Log.d(TAG, " uriStr = " + uriStr);
                                }
                                try {
                                    if (msgCount > MAX_MMS_SMS_COUNT && isCMCCTest()) {
                                        return;
                                    }
                                    Uri msgUri = Uri.parse(uriStr);
                                    Uri resultUri = mContext.getContentResolver().insert(msgUri,
                                            values);
                                    msgCount++;
                                    Log.d(TAG, " resultUri = " + resultUri.toString());
                                } catch (Exception e) {
                                    Log.d(TAG, "recoverSmsMessages :insert error");
                                }
                            } finally {
                                if (fin != null) {
                                    fin.close();
                                }
                            }
                        }
                        break;
                    }
                    case XmlPullParser.END_TAG:
                        break;
                    default:
                        break;
                }
                evtType = xmlParser.next();
            }
        } catch (Exception e) {
            Log.d(TAG, "error = " + e);
            System.out.print("Unable to parse file: " + e);
        }

    }

    private void resumeMessageToDatabase(int number)
    {
        addMsg(mMessages);
    }

    private void addMsg(final ShortMessage[] messages)
    {

        int msgCount = 0;
        if (isCMCCTest()) {
            Log.d(TAG, "isCMCCTest");
            final Cursor c = mContext.getContentResolver().query(
                    MAILBOX_MESSAGES_COUNT, null, null, null, null);
            if (c != null) {
                try {
                    if (c.moveToFirst()) {
                        msgCount = c.getInt(0);
                    }
                } finally {
                    c.close();
                }
            }
        }
        Log.d(TAG, "addMsg msgCount: " + msgCount + "messages.length: " + messages.length);
        if (msgCount < MAX_MMS_SMS_COUNT || !isCMCCTest()) {
            int recMsgCnt = messages.length;
            if (isCMCCTest() && recMsgCnt > (MAX_MMS_SMS_COUNT - msgCount))
                recMsgCnt = MAX_MMS_SMS_COUNT - msgCount;
                mCallBackHandler.sendMessage(mCallBackHandler.obtainMessage(
                    EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_SMS, recMsgCnt));
        }

        List<ContentValues> valuesList = new ArrayList<ContentValues>();
        for (int i = 0; i < messages.length && (msgCount < MAX_MMS_SMS_COUNT || !isCMCCTest()); i++)
        {
            if (!isBackUpSubMsg(messages[i].getSubId())) {
                mCallBackHandler.sendMessage(mCallBackHandler.obtainMessage(
                        EVENT_SET_PROGRESS_VALUE, i + 1, 0));
                continue;
            }
            if (((RecoverOperateActivity) mContext).getCancelRestoreFlag()) {
                Log.d(TAG, "addMsg cancelled");
                break;
            }
            String addressNumber = messages[i].getFrom();
            boolean isRead = messages[i].isRead();
            String box = messages[i].getBoxType();

            int locked = 0;
            locked = messages[i].getLocked();
            Log.d(TAG, "locked:" + locked);

            int mailboxType = Sms.MESSAGE_TYPE_INBOX;
            if (box.equalsIgnoreCase("SENDBOX")) {
                if (DBG)
                    Log.d(TAG, "SENDBOX");
                addressNumber = messages[i].getTo();
                mailboxType = Sms.MESSAGE_TYPE_SENT;
                isRead = true;
            } else if (box.equalsIgnoreCase("DRAFT")) {
                if (DBG)
                    Log.d(TAG, "DRAFT");
                addressNumber = messages[i].getTo();
                mailboxType = Sms.MESSAGE_TYPE_DRAFT;
                isRead = true;
            }
            String msgBody = messages[i].getBody();
            Date date = messages[i].getDate();
            int subId = messages[i].getSubId();
            ContentValues values = new ContentValues();
            values.put(Sms.TYPE, mailboxType);
            values.put("address", addressNumber);
            values.put("body", msgBody);
            values.put("date", date.getTime());
            values.put("phone_id", subId);
            values.put(Inbox.READ, Integer.valueOf(isRead ? 1 : 0));
            values.put("locked", locked);
            String uriStr = "content://sms/";
            Uri msgUri = Uri.parse(uriStr);

            valuesList.add(values);
            if (i > 0 && i % 50 == 0) {
                try {
                    ContentValues[] contents = valuesList.toArray(new ContentValues[valuesList.size()]);
                    Log.d(TAG, "bulkInsert uri is " + msgUri + " values size is " + contents.length);
                    mContext.getContentResolver().bulkInsert(msgUri, contents);
                    valuesList.clear();
                } catch (Exception e) {
                    Log.e(TAG, "error: " + e);
                    return;
                }
            }
            msgCount++;
            mCallBackHandler.sendMessage(mCallBackHandler.obtainMessage(
                    EVENT_SET_PROGRESS_VALUE, i + 1, 0));
        }
        if (valuesList != null && !valuesList.isEmpty()) {
            try {
                ContentValues[] contents = valuesList.toArray(new ContentValues[valuesList.size()]);
                Log.d(TAG, "bulkInsert values size is " + contents.length);
                mContext.getContentResolver().bulkInsert(Uri.parse("content://sms/"), contents);
                valuesList.clear();
            } catch (Exception e) {
                Log.e(TAG, "error: " + e);
                return;
            }
        }

        if (isCMCCTest() && msgCount >= MAX_MMS_SMS_COUNT) {
            mCallBackHandler.sendEmptyMessage(EVENT_SDCARD_FULL);
        }
    }

    private static ShortMessage[] parseMessages(File[] files) {
        ArrayList<ShortMessage> messages = new ArrayList<ShortMessage>(files.length);
        for (File file : files)
        {
            try {
                FileInputStream mMessagesStream = new FileInputStream(file);
                ShortMessage[] msgs = MessageParsers.getParser(file).readMessage(mMessagesStream);
                for (int index = 0; index < msgs.length; index++) {
                    messages.add(msgs[index]);
                }
                if (mMessagesStream != null) {
                    mMessagesStream.close();
                }
            } catch (IOException e) {
                Log.e(TAG, "Error when closing stream" + e.toString());
            } catch (Exception e) {
                Log.e(TAG, "(Unable to parse file: " + file.getName() + ") " + e);
            }
        }
        return messages.toArray(new ShortMessage[0]);
    }

    private File getUniqueDestination(String base, String extension) {
        File file;

        if (null != extension) {
            file = new File(base + "." + extension);
        } else {
            file = new File(base);
            for (int i = 2; file.exists(); i++) {
                file = new File(base + "_" + i);
            }
        }

        return file;
    }

    private static final String BACKUP_MMS_CONSTRAINT =
        "(" + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_INBOX
                + " or " + Mms.MESSAGE_BOX + "=" + Mms.MESSAGE_BOX_SENT + ")"
        + " and (" + Mms.MESSAGE_TYPE + "=" + MESSAGE_TYPE_SEND_REQ
                + " or " + Mms.MESSAGE_TYPE + "=" + MESSAGE_TYPE_RETRIEVE_CONF + ")";

    private String generateMMSSelection() {
        // just need to backup inbox ,sent
        String selection = BACKUP_MMS_CONSTRAINT;
        if (mSlotId != BackupUtils.SLOT_ALL) {
            selection = BACKUP_MMS_CONSTRAINT + " and (phone_id="+ mSlotId + ")";
        }
        return selection;
    }

    private File openMmsXmlFile (String directory, String mmsXml) {
        File dir = new File(directory);
        if (!dir.isDirectory()) {
            return null;
        }

        File[] files = dir.listFiles();
        if (files.length == 0) {
            Log.e(TAG, "Not find vmsg files");
            return null;
        }
        return new File(directory + mmsXml);
    }

    private static class MmsRecordList {
        public List<ContentValues> mInBoxRecordList ;
        public List<ContentValues> mSendBoxRecordList ;
        private int mSlotId;

        public MmsRecordList(int slotId) {
            mInBoxRecordList = new ArrayList<ContentValues>();
            mSendBoxRecordList = new ArrayList<ContentValues>();
            mSlotId = slotId;
        }

        private void clear() {
            mInBoxRecordList.clear();
            mSendBoxRecordList.clear();
        }

        private boolean isRequestMsg(int subId) {
            return mSlotId == BackupUtils.SLOT_ALL || mSlotId == subId;
        }

        private int size() {
            return mInBoxRecordList.size() + mSendBoxRecordList.size();
        }

        private boolean isValidRecord(ContentValues mmsRecord) {
            return mmsRecord != null
                    && isRequestMsg(mmsRecord.getAsInteger(PDU_PHONE_ID));
        }

        public void addMmsRecord(ContentValues mmsRecord) {
            if (!isValidRecord(mmsRecord)) {
                Log.e(TAG, "un-request mms record.");
                return;
            }

            switch (mmsRecord.getAsInteger(PDU_MSG_BOX)) {
                case Mms.MESSAGE_BOX_INBOX:
                    mInBoxRecordList.add(mmsRecord);
                    break;
                case Mms.MESSAGE_BOX_SENT:
                    mSendBoxRecordList.add(mmsRecord);
                    break;
                default:
                    Log.d(TAG, "unSupport mms box.");
                    break;
            }
        }
    }

    private ContentValues parseMmsRecord(XmlPullParser xmlParser) {
        if (xmlParser == null) {
            return null;
        }

        int itemCount = xmlParser.getAttributeCount();
        if (itemCount == 0) {
            Log.e(TAG, "no attribute.");
            return null;
        }

        ContentValues mmsRecord = new ContentValues(itemCount);
        for (int i = 0; i < itemCount; i++) {
            String attribute = getDBRow(xmlParser.getAttributeName(i));
            if (!TextUtils.isEmpty(attribute)) {
                mmsRecord.put(attribute, xmlParser.getAttributeValue(i));
            } else {
                Log.d(TAG, "unknow attribute :" + attribute);
            }
        }

        if (mmsRecord.size() != 0) {
            return mmsRecord;
        }
        return null;
    }

    public MmsRecordList parseMmsXml(String directory, String mmsXml) {
        File mmsXmlFile = openMmsXmlFile(directory, mmsXml);
        int recordCount = 0;
        if (mmsXmlFile == null || !mmsXmlFile.exists()) {
            return null;
        }
        MmsRecordList recordList = new MmsRecordList(mSlotId);
        try {
            XmlPullParser xmlParser = Xml.newPullParser();
            xmlParser.setInput(new FileReader(mmsXmlFile));
            int evtType = xmlParser.getEventType();
            while (evtType != XmlPullParser.END_DOCUMENT && isContinue()) {
                switch (evtType) {
                    case XmlPullParser.START_TAG:
                        String tag = xmlParser.getName();
                        if (tag.equalsIgnoreCase(TAG_RECORD)) {
                            recordList.addMmsRecord(parseMmsRecord(xmlParser));
                        } else if (tag.equalsIgnoreCase(TAG_RECORD_COUNT)) {
                            recordCount = Integer.parseInt(xmlParser.getAttributeValue(null,
                                    ITEM_COUNT));
                            mCallBackHandler.sendMessage(mCallBackHandler.obtainMessage(
                                    EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_MMS, recordCount));
                            Log.d(TAG, "mms record count :" + recordCount);
                        }
                        break;
                    case XmlPullParser.END_TAG:
                        break;
                    default:
                        break;
                }
                evtType = xmlParser.next();
            }

            if (recordCount == 0 && recordList.size() != 0) {
                Log.d(TAG, "There is no mmsCount tag, so use the record count.");
                recordCount = recordList.size();
                mCallBackHandler.sendMessage(mCallBackHandler.obtainMessage(
                        EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_MMS, recordCount));
            }

            if (!isContinue() || recordCount != recordList.size()) {
                Log.d(TAG, "error mms xml :" + isContinue());
                recordList.clear();
                recordList = null;
            }
        } catch (Exception e) {
            Log.e(TAG, "restore exception: ", e);
        }
        return recordList;
    }

    private int bulkInsert(Uri url, List<ContentValues> insertList) {
        try {
            ContentValues[] contents = insertList
                    .toArray(new ContentValues[insertList.size()]);
            mCallBackHandler.sendMessage(mCallBackHandler.obtainMessage(
                    EVENT_SET_PROGRESS_VALUE, BackupActivity.INVALID_ARGUMENT,
                    insertList.size()));
            Log.d(TAG, "bulkInsert values size is " + contents.length);
            return mContext.getContentResolver().bulkInsert(url, contents);
        } catch (Exception e) {
            Log.e(TAG, "error: " + e);
        }
        return 0;
    }

    private int insert(Uri url, List<ContentValues> recordList) {
        List<ContentValues> insertList = new ArrayList<ContentValues>();
        int count = 0;
        int recordSize = recordList.size();
        int i = 0;
        if (recordList == null || recordSize == 0) {
            Log.d(TAG, "bulkInsert size is 0.");
            return 0;
        }
        Uri insertUrl = url.buildUpon()
            .appendQueryParameter("restore_dir", mMmsBackupDir).build();
        for (ContentValues record : recordList) {
            if (!isContinue()) {
                insertList.clear();
                break;
            }
            insertList.add(record);
            if (++i % MAX_INSERT_COUNT == 0) {
                count += bulkInsert(insertUrl, insertList);
                insertList.clear();
            }
        }

        if (insertList.size() > 0) {
            count += bulkInsert(insertUrl, insertList);
            insertList.clear();
        }
        return count;
    }

    public int recoverMms(String dir) {
        long start = System.currentTimeMillis();
        MmsRecordList recordList = parseMmsXml(dir, MMS_XML_NAME);
        if (recordList == null || recordList.size() < 0) {
            Log.e(TAG, "empty mms data.");
            return 0;
        }

        int success = insert(Mms.Inbox.CONTENT_URI, recordList.mInBoxRecordList)
                + insert(Mms.Sent.CONTENT_URI, recordList.mSendBoxRecordList);

        Log.d(TAG, "finish: restore count: " + recordList.size()
                + " successfully count : " + success
                + " using : " + (System.currentTimeMillis() - start)
                + " is finihing :" + isContinue());
        return success;
    }

    private boolean isBackUpSubMsg(int subId) {
        return mSlotId == BackupUtils.SLOT_ALL || mSlotId == subId;
    }

    /* whether is in cmcc test mode, 0 is false ,1 is true */
    public static boolean isCMCCTest() {
        return true;
    }

    public void cancel() {
        mCanceled = true;
        Log.d(TAG, "cancel operation");
    }

    public boolean isContinue() {
        boolean isFull;
        if (mPath.equals(BackupUtils.strExternalPath)) {
            isFull = BackupUtils.isTfCardFull();
        } else {
            isFull = BackupUtils.isInternalFull();
        }
        return !mCanceled && !isFull;
    }
}
