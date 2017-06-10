/*
 * Copyright (c) 2011-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qrd.wappush;

import java.io.IOException;
import java.io.InputStream;
import org.xml.sax.SAXException;
import android.content.Context;
import android.util.Log;
import android.net.Uri;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Threads;
import android.database.sqlite.SqliteWrapper;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;

public class WapPushHandler implements IWapPushHandler {
    private String mAction = null;
    private String mContent = null;
    private String mLink = null;
    private long mThreadID = 0;
    private final static String SPACE = " ";

    public String TAG = "WapPushHandler";

    public Uri handleWapPush(InputStream inputstream, String mime, Context context,
            int slotID, String address) throws SAXException, IOException {
        boolean bIsSI = mime.equals("application/vnd.wap.sic");
        boolean bIsSL = mime.equals("application/vnd.wap.slc");
        if (!bIsSI && !bIsSL)
            throw new SAXException("Error: can not handler unsupported type");

        int pushType = bIsSI ? WapPushParser.SI_TYPE : WapPushParser.SL_TYPE;
        WapPushParser parser = new WapPushParser();
        parser.parse(inputstream, pushType);
        mAction = parser.getAction();
        mContent = (parser.getContent() == null) ? "" : parser.getContent();
        mLink = (parser.getHyperLink() == null) ? "" : parser.getHyperLink();
        Uri pushUri = storeWapPushMessage(context, mContent + SPACE + mLink, slotID, address);
        if(bIsSL && (!mLink.isEmpty())) {
            Intent target = new Intent(Intent.ACTION_VIEW, Uri.parse(mLink));
            context.startActivity(target.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK));
        }
        return pushUri;
    }

    public long getThreadID() {
        return mThreadID;
    }

    private Uri storeWapPushMessage(Context context, String pushContent,
            int subscription, String address) {

        // Store the message in the content provider.
        ContentValues values = new ContentValues();

        values.put(Sms.Inbox.ADDRESS, address);
        Log.d(TAG, "storeWapPushMessage : ADDRESS " + address + ", subscription "
            + subscription + ", Content " + pushContent);

        // Use now for the timestamp to avoid confusion with clock
        // drift between the handset and the SMSC.
        values.put(Sms.Inbox.DATE, new Long(System.currentTimeMillis()));
        values.put(Sms.Inbox.READ, 0);
        values.put(Sms.Inbox.SEEN, 0);
        values.put(Sms.ERROR_CODE, 0);
        values.put(Sms.PHONE_ID, subscription);
        values.put(Sms.Inbox.BODY, pushContent);
        values.put(Sms.ADDRESS, address);
        mThreadID = Threads.getOrCreateThreadId(context, address);
        values.put(Sms.THREAD_ID, mThreadID);

        ContentResolver resolver = context.getContentResolver();

        Uri insertedUri = SqliteWrapper.insert(context, resolver, Sms.Inbox.CONTENT_URI, values);

        return insertedUri;
    }
}


