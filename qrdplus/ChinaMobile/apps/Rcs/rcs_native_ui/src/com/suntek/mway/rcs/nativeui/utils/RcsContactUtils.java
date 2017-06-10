/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.utils;

import com.suntek.mway.rcs.client.aidl.contacts.RCSContact;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.QRCardImg;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.QRCardInfo;
import com.suntek.mway.rcs.client.api.profile.callback.QRImgListener;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.Avatar;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.Avatar.IMAGE_TYPE;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.TelephoneModel;
import com.suntek.mway.rcs.client.api.profile.callback.ProfileListener;
import com.suntek.mway.rcs.client.api.profile.impl.ProfileApi;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.Profile;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.TelephoneModel;
import com.suntek.mway.rcs.client.aidl.provider.SuntekMessageData;
import com.suntek.mway.rcs.client.aidl.provider.model.GroupChatModel;
import com.suntek.mway.rcs.client.aidl.provider.model.GroupChatUser;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.client.aidl.contacts.RCSContact;
import com.suntek.mway.rcs.nativeui.RcsApiManager;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.ArrayList;

public class RcsContactUtils {
    public static final String NOTIFY_CONTACT_PHOTO_CHANGE = "com.suntek.mway.rcs.NOTIFY_CONTACT_PHOTO_CHANGE";
    public static final String LOCAL_PHOTO_SETTED = "local_photo_setted";
    public static final String TAG = "NativeUI_RcsContactUtils";
    private static volatile boolean rcsConnection = false;

    public static final String PREF_FOLLOW_STATE_CHANGED = "pref_follow_state";

    public static boolean isRcsConnection() {
        return rcsConnection;
    }

    public static void setRcsConnectionState(boolean flag) {
        rcsConnection = flag;
    }

    public static void sleep(long time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public static String replaceNumberSpace(String number) {
        if (TextUtils.isEmpty(number))
            return null;
        return number.replaceAll("[- ]", "");
    }

    public static Bitmap getPhotoByNumber(Context context, String number) {
        Bitmap photo = null;

        long rawContactId = getRawContactIdByNumber(context, number);
        Log.d("RCS_UI", "rawContactId=" + rawContactId);
        if (rawContactId > 0) {
            long contactId = getContactIdByRawContactId(context, rawContactId);
            Log.d("RCS_UI", "contactId=" + contactId);
            if (contactId > 0) {
                photo = getPhotoByContactId(context, contactId);
                Log.d("RCS_UI", "photo=" + photo);
            } else {
                photo = getPhotoByContactId(context, rawContactId);
            }
        }

        return photo;
    }

    public static long getContactIdByRawContactId(Context context, long rawContactId) {
        long contactId = -1;

        Cursor cursor = context.getContentResolver().query(RawContacts.CONTENT_URI, new String[] {
            RawContacts.CONTACT_ID
        }, RawContacts._ID + "=?", new String[] {
            String.valueOf(rawContactId)
        }, null);

        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    contactId = cursor.getLong(0);
                }
            } finally {
                cursor.close();
            }
        }

        return contactId;
    }

    public static long getRawContactIdByNumber(Context context, String number) {
        long rawContactId = -1;
        String contactName = number;
        String numberW86;
        if (!number.startsWith("+86")) {
            numberW86 = "+86" + number;
        } else {
            numberW86 = number;
            number = number.substring(3);
        }
        String formatNumber = getAndroidFormatNumber(number);

        ContentResolver cr = context.getContentResolver();
        Cursor pCur = cr.query(
                ContactsContract.CommonDataKinds.Phone.CONTENT_URI, new String[] {
                    ContactsContract.CommonDataKinds.Phone.CONTACT_ID
                },
                ContactsContract.CommonDataKinds.Phone.NUMBER + " = ? OR "
                        + ContactsContract.CommonDataKinds.Phone.NUMBER + " = ? OR "
                        + ContactsContract.CommonDataKinds.Phone.NUMBER + " = ? ",
                new String[] {
                        number, numberW86, formatNumber
                }, null);
        try {
            if (pCur != null && pCur.moveToFirst()) {
                rawContactId = pCur
                        .getLong(pCur
                                .getColumnIndex(ContactsContract.CommonDataKinds.Phone.CONTACT_ID));
            }
        } finally {
            if (pCur != null){
                pCur.close();
            }
        }

        return rawContactId;
    }

    public static Bitmap getPhotoByContactId(Context context, long contactId) {
        ContentResolver cr = context.getContentResolver();
        Uri uri = ContentUris.withAppendedId(Contacts.CONTENT_URI, contactId);
        Log.d("RCS_UI", "contact uri=" + uri);
        InputStream input = Contacts.openContactPhotoInputStream(cr, uri);
        Log.d("RCS_UI", "input=" + input);
        Bitmap contactPhoto = BitmapFactory.decodeStream(input);
        return contactPhoto;
    }

    public static String getGroupChatMemberDisplayName(Context context, String groupId,
            String number, String myPhoneNumber) {
        GroupChatModel model = null;
        try {
            model = RcsApiManager.getMessageApi().getGroupChatById(groupId);
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
        if (model == null)
            return number;
        List<GroupChatUser> list = model.getUserList();
        if (list == null || list.size() == 0)
            return number;
        for (GroupChatUser groupChatUser : list) {
            if (groupChatUser.getNumber().equals(number)) {
                if (!TextUtils.isEmpty(groupChatUser.getAlias())) {
                    return groupChatUser.getAlias();
                } else {
                    return getContactNameFromPhoneBook(context, number, myPhoneNumber);
                }
            }
        }
        return number;
    }

    public static String getContactNameFromPhoneBook(Context context, String phoneNum,
            String myPhoneNumber) {
        Uri qureyUri = null;
        if (myPhoneNumber != null && myPhoneNumber.endsWith(phoneNum)) {
            qureyUri = Uri.withAppendedPath(ContactsContract.Profile.CONTENT_URI,
                    "data");
        } else {
            qureyUri = ContactsContract.CommonDataKinds.Phone.CONTENT_URI;
        }

        String contactName = phoneNum;
        String numberW86;
        if (!phoneNum.startsWith("+86")) {
            numberW86 = "+86" + phoneNum;
        } else {
            numberW86 = phoneNum;
            phoneNum = phoneNum.substring(3);
        }
        String formatNumber = getAndroidFormatNumber(phoneNum);

        ContentResolver cr = context.getContentResolver();

        Cursor pCur = cr.query(qureyUri, new String[] {
                    ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME
                },
                ContactsContract.CommonDataKinds.Phone.NUMBER + " = ? OR "
                        + ContactsContract.CommonDataKinds.Phone.NUMBER + " = ? OR "
                        + ContactsContract.CommonDataKinds.Phone.NUMBER + " = ? ",
                new String[] {
                        phoneNum, numberW86, formatNumber
                }, null);
        try {
            if (pCur != null && pCur.moveToFirst()) {
                contactName = pCur
                        .getString(pCur
                                .getColumnIndex(ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME));

            }
        } finally {
            if (pCur != null)
                pCur.close();
        }
        return contactName;
    }

    public static String getAndroidFormatNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            return number;
        }

        number = number.replaceAll(" ", "");

        if (number.startsWith("+86")) {
            number = number.substring(3);
        }

        if (number.length() != 11) {
            return number;
        }

        StringBuilder builder = new StringBuilder();
        // builder.append("+86 ");
        builder.append(number.substring(0, 3));
        builder.append(" ");
        builder.append(number.substring(3, 7));
        builder.append(" ");
        builder.append(number.substring(7));
        return builder.toString();
    }

    public static void insertGroupChat(Context context, String groupId, String groupSubject){
        if(context == null) return;
        Cursor groupCount = null;
        try {
            groupCount =  context.getContentResolver().query(Groups.CONTENT_URI, null,
            Groups.SYSTEM_ID + " = " + groupId, null, null);
            if (null != groupCount) {
                if (groupCount.getCount() > 0) {
                    groupCount.close();
                    return;
                }
            }
        } finally {
            if (groupCount != null ) {
                groupCount.close();
            }
        }
        ContentResolver resolver = context.getContentResolver();
        ContentValues values = new ContentValues();
        values.put(Groups.TITLE, groupSubject);
        values.put(Groups.SYSTEM_ID,groupId);
        values.put(Groups.SOURCE_ID,"RCS");

        try{
            Log.d(TAG," create group: title= "+groupSubject+" id= "+groupId);
            resolver.insert(Groups.CONTENT_URI, values);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    public static void UpdateGroupChatSubject(Context context,String groupId,String groupSubject){
        if(context == null) return;
        ContentResolver resolver = context.getContentResolver();
        StringBuilder where = new StringBuilder();
        where.append(Groups.SYSTEM_ID);
        where.append("="+groupId);
        ContentValues values = new ContentValues();
        values.put(Groups.TITLE, groupSubject);
        values.put(Groups.SYSTEM_ID,groupId);

        try{
            Log.d(TAG," update group: title= "+groupSubject+" id= "+groupId);
            resolver.update(Groups.CONTENT_URI, values, where.toString(), null);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    public static void deleteGroupChat(Context context,String groupId){
        if(context == null) return;
        ContentResolver resolver = context.getContentResolver();
        StringBuilder where = new StringBuilder();
        where.append(Groups.SYSTEM_ID);
        where.append("="+groupId);

        try{
            Log.d(TAG," disband group:  id= "+groupId);
            resolver.delete(Groups.CONTENT_URI, where.toString(), null);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    public static Bitmap getMyProfilePhotoOnData(Context context) {
        Bitmap bitmap = null;
        String id = getRawContactId(context);
        Uri uri = Uri.parse("content://com.android.contacts/profile/data/");
        Cursor cursor = context.getContentResolver().query(uri,
                new String[] {
                        "_id", "mimetype", "data15"
                },
                " raw_contact_id = ?  AND mimetype = ? ",
                new String[] {
                        id, "vnd.android.cursor.item/photo"
                }, null);
        try {
            if (cursor != null && cursor.moveToFirst()) {
                byte[] data = cursor.getBlob(cursor
                        .getColumnIndexOrThrow("data15"));
                bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return bitmap;
    }

    public static byte[] getMyProfilePhotoByteOnData(Context context) {
        String id = getRawContactId(context);
        Uri uri = Uri.parse("content://com.android.contacts/profile/data/");
        Cursor cursor = context.getContentResolver().query(uri,
                new String[] {
                        "_id", "mimetype", "data15"
                },
                " raw_contact_id = ?  AND mimetype = ? ",
                new String[] {
                        id, "vnd.android.cursor.item/photo"
                }, null);
        try {
            if (cursor != null && cursor.moveToFirst()) {
                byte[] data = cursor.getBlob(cursor
                        .getColumnIndexOrThrow("data15"));
                return data;
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return null;
    }

    private static String getRawContactId(Context context) {
        String rawContactId = null;
        Uri uri = Uri
                .parse("content://com.android.contacts/profile/raw_contacts/");
        Cursor cursor = context.getContentResolver().query(uri, null,
                "account_id = 1 AND contact_id != '' ", null, null);
        try {
            if (cursor != null && cursor.moveToFirst() && !cursor.isAfterLast()) {
                rawContactId = cursor.getString(cursor
                        .getColumnIndexOrThrow("_id"));
                cursor.moveToNext();
            }
        } finally {
            cursor.close();
        }

        if (rawContactId == null) {
            ContentValues values = new ContentValues();
            Uri rawContactUri = context.getContentResolver()
                    .insert(uri, values);
            rawContactId = String.valueOf(ContentUris.parseId(rawContactUri));
        }
        return rawContactId;
    };

    public static void startMyProfileActivity(Context context){
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_VIEW);
        Uri uri = Uri
                .parse("content://com.android.contacts/contacts/lookup/profile/"+ getRawContactId(context));
        intent.setData(uri);
        context.startActivity(intent);
    }

        private static class UpdatePhotosTask extends AsyncTask<Void, Void, Void> {

        private Context mContext;
        private String mNumber;

        private Handler mHandler = new Handler();

        UpdatePhotosTask(Context context, String number) {
            mContext = context;
            mNumber = number;
        }

        @Override
        protected Void doInBackground(Void... params) {
            long aContactId =getContactIdByNumber(mContext, mNumber);
            ContentResolver resolver= mContext.getContentResolver();
            Cursor c = resolver.query(RawContacts.CONTENT_URI, new String[] {
                    RawContacts._ID
            }, RawContacts.CONTACT_ID + "=" + String.valueOf(aContactId), null, null);
            final ArrayList<Long> rawContactIdList = new ArrayList<Long>();
            if(c != null){
                try {
                    if (c.moveToFirst()) {
                        // boolean hasTryToGet = false;
                        do {
                            long rawContactId = c.getLong(0);
                            if (!hasLocalSetted(resolver, rawContactId)) {
                                rawContactIdList.add(rawContactId);
                            }
                        } while (c.moveToNext());
                    }
                } finally {
                    if(c != null)
                        c.close();
                }
            }
            if (rawContactIdList.size() > 0) {
                try {
                    RcsApiManager.getProfileApi().getHeadPicByContact(aContactId,
                            new ProfileListener() {

                                @Override
                                public void onAvatarGet(final Avatar photo,
                                        final int resultCode, final String resultDesc)
                                        throws RemoteException {
                                    mHandler.post(new Runnable() {
                                        @Override
                                        public void run() {
                                            if (resultCode == 0) {
                                                if (photo != null) {
                                                    byte[] contactPhoto = Base64.decode(
                                                            photo.getImgBase64Str(),
                                                            android.util.Base64.DEFAULT);
                                                    for (long rawContactId : rawContactIdList) {
                                                        final Uri outputUri = Uri.withAppendedPath(
                                                                ContentUris
                                                                        .withAppendedId(
                                                                                RawContacts.CONTENT_URI,
                                                                                rawContactId),
                                                                RawContacts.DisplayPhoto.CONTENT_DIRECTORY);
                                                        setContactPhoto(mContext,
                                                                contactPhoto, outputUri);
                                                    }
                                                    //notify mms list
                                                    mContext.sendBroadcast(new Intent(NOTIFY_CONTACT_PHOTO_CHANGE));
                                                }
                                            } else {
                                            }
                                        }
                                    });
                                }

                                @Override
                                public void onAvatarUpdated(int arg0, String arg1)
                                        throws RemoteException {
                                }

                                @Override
                                public void onProfileGet(Profile arg0, int arg1, String arg2)
                                        throws RemoteException {
                                }

                                @Override
                                public void onProfileUpdated(int arg0, String arg1)
                                        throws RemoteException {
                                }

                                @Override
                                public void onQRImgDecode(QRCardInfo imgObj, int resultCode,
                                        String arg2) throws RemoteException {
                                }
                            });
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                }
            }
            return null;
        }

    }

    public static void updateContactPhotosByNumber(Context context,String number) {
        new UpdatePhotosTask(context,number).execute();
    }

    public static void setContactPhoto(Context context, byte[] input,
            Uri outputUri) {
        FileOutputStream outputStream = null;

        try {
            outputStream = context.getContentResolver().openAssetFileDescriptor(outputUri, "rw")
                    .createOutputStream();
            outputStream.write(input);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try{
                outputStream.close();
            }catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static boolean hasLocalSetted(ContentResolver resolver, long rawContactId) {
        Cursor c = resolver.query(ContactsContract.RawContacts.CONTENT_URI, new String[] {
                LOCAL_PHOTO_SETTED
        }, RawContacts._ID + " = ? ", new String[] {
                String.valueOf(rawContactId)
        }, null);
        long localSetted = 0;
        try {
            if (c != null && c.moveToFirst()) {
                localSetted = c.getLong(0);
            }
        } finally {
            c.close();
        }
        return (localSetted == 1) ? true : false;
    }

    public static long getContactIdByNumber(Context context, String number) {
        if (TextUtils.isEmpty(number)) {
            return -1;
        }
        String numberW86 = number;
        if (!number.startsWith("+86")) {
            numberW86 = "+86" + number;
        } else {
            numberW86 = number.substring(3);
        }
        Cursor cursor = context.getContentResolver().query(Phone.CONTENT_URI, new String[] {
                Phone.CONTACT_ID
        }, Phone.NUMBER + "=? OR " + Phone.NUMBER + "=?", new String[] {
                number, numberW86
        }, null);
        if (cursor != null) {
            try{
                if (cursor.moveToFirst()) {
                    return cursor.getInt(0);
                }
            } finally {
                cursor.close();
            }
        }
        return -1;
    }

}
