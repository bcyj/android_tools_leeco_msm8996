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

package com.android.backup.vcard;

import android.accounts.Account;
import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.os.RemoteException;
import com.android.backup.ContactsContract;
import com.android.backup.ContactsContract.Data;
import com.android.backup.ContactsContract.Groups;
import com.android.backup.ContactsContract.Contacts;
import com.android.backup.ContactsContract.RawContacts;
import com.android.backup.ContactsContract.CommonDataKinds.Email;
import com.android.backup.ContactsContract.CommonDataKinds.Event;
import com.android.backup.ContactsContract.CommonDataKinds.GroupMembership;
import com.android.backup.ContactsContract.CommonDataKinds.Im;
import com.android.backup.ContactsContract.CommonDataKinds.Nickname;
import com.android.backup.ContactsContract.CommonDataKinds.Note;
import com.android.backup.ContactsContract.CommonDataKinds.Organization;
import com.android.backup.ContactsContract.CommonDataKinds.Phone;
import com.android.backup.ContactsContract.CommonDataKinds.Photo;
import com.android.backup.ContactsContract.CommonDataKinds.StructuredName;
import com.android.backup.ContactsContract.CommonDataKinds.StructuredPostal;
import com.android.backup.ContactsContract.CommonDataKinds.Website;
import com.android.backup.vcard.VCardEntry;
import android.util.Log;
import android.net.Uri;
import android.net.Uri.Builder;
import android.os.Bundle;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.List;
import android.content.EntityIterator;
import android.content.Entity;
import android.content.Entity.NamedContentValues;
import android.content.ContentValues;
import android.text.TextUtils;
import java.util.ArrayList;
import com.android.backup.vcard.VCardEntry.*;
import com.android.backup.vcard.VCardBuilder;
//import android.pim.vcard.VCardBuilderCollection;
import com.android.backup.vcard.VCardConfig;
//import android.pim.vcard.VCardEntryConstructor;
import com.android.backup.vcard.VCardEntryCounter;
import com.android.backup.vcard.VCardParser_V21;
import com.android.backup.vcard.VCardParser_V30;
import com.android.backup.vcard.VCardSourceDetector;
import com.android.backup.vcard.exception.VCardException;
import com.android.backup.vcard.exception.VCardNestedException;
import com.android.backup.vcard.exception.VCardNotSupportedException;
import com.android.backup.vcard.exception.VCardVersionException;
import com.android.backup.vcard.VCardComposer.OneEntryHandler;
import com.android.backup.vcard.VCardEntryHandler;
import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.io.ByteArrayInputStream;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.ByteArrayOutputStream;

public class VCardManager {

    private static final String TAG = "VCardManager";

    public static final int mVCardType = VCardConfig.VCARD_TYPE_DEFAULT;

    private static final Uri sDataRequestUri;

    // huangjiufa add 20100105 for text codec translation
    public static final String CODEC_UTF8 = "UTF-8";
    public static final String CODEC_UNICODE = "UNICODE";
    public static final String CODEC_GBK = "GBK";
    public static final String CODEC_GB2312 = "GB2312";
    public static final String CODEC_BIG5 = "Big5-HKSCS";

    static {
        Uri.Builder builder = Data.CONTENT_URI.buildUpon();
        // builder.appendQueryParameter(Data.FOR_EXPORT_ONLY, "1");
        sDataRequestUri = builder.build();
    }

    public VCardManager() {
    };

    static public VCardEntry readContactByID(Context context, int contactId)
    {

        final Map<String, List<ContentValues>> contentValuesListMap =
                new HashMap<String, List<ContentValues>>();
        final String selection = Data.RAW_CONTACT_ID + "=?";
        final String[] selectionArgs = new String[] {
            Integer.toString(contactId)
        };
        // The resolver may return the entity iterator with no data. It is
        // possiible.
        // e.g. If all the data in the contact of the given contact id are not
        // exportable ones,
        // they are hidden from the view of this method, though contact id
        // itself exists.
        boolean dataExists = false;
        Cursor entityIteratorCursor = null;
        VCardEntry contactStruct = new VCardEntry();
        ContentResolver resolver = context.getContentResolver();
        Cursor c = resolver.query(RawContacts.RECORDS_CONTENT_URI,
                new String[] {
                        RawContacts.NLOCATION, RawContacts.MEMORY_TYPE
                },
                RawContacts._ID + " = " + contactId, null, null);

        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    contactStruct.mNlocation = c.getInt(0);
                    contactStruct.mMemoryType = c.getInt(1);
                }
            } finally {
                c.close();
            }
        }

        try {
            entityIteratorCursor = resolver.query(
                    sDataRequestUri, null, selection, selectionArgs, null);
            while (entityIteratorCursor.moveToNext()) {

                if (true) {
                    String mime_type = entityIteratorCursor.getString(entityIteratorCursor
                            .getColumnIndexOrThrow(Data.MIMETYPE));

                    if (mime_type.equals(StructuredName.CONTENT_ITEM_TYPE))
                    {
                        contactStruct.mFamilyName = entityIteratorCursor
                                .getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(StructuredName.FAMILY_NAME));
                        contactStruct.mMiddleName = entityIteratorCursor
                                .getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(StructuredName.MIDDLE_NAME));
                        contactStruct.mGivenName = entityIteratorCursor
                                .getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(StructuredName.GIVEN_NAME));
                        contactStruct.mPrefix = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredName.PREFIX));
                        contactStruct.mSuffix = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredName.SUFFIX));
                        contactStruct.mPhoneticFamilyName = entityIteratorCursor
                                .getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(StructuredName.PHONETIC_FAMILY_NAME));
                        contactStruct.mPhoneticGivenName = entityIteratorCursor
                                .getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(StructuredName.PHONETIC_GIVEN_NAME));
                        contactStruct.mPhoneticMiddleName = entityIteratorCursor
                                .getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(StructuredName.PHONETIC_MIDDLE_NAME));
                        contactStruct.mDisplayName = entityIteratorCursor
                                .getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(StructuredName.DISPLAY_NAME));

                    }
                    else if (mime_type.equals(Nickname.CONTENT_ITEM_TYPE))
                    {
                        contactStruct.addNickName(entityIteratorCursor
                                .getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(Nickname.NAME)));
                    }
                    else if (mime_type.equals(Phone.CONTENT_ITEM_TYPE))
                    {
                        Integer typeAsObject = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Phone.TYPE));
                        String label = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Phone.LABEL));
                        String phoneNumber = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Phone.NUMBER));
                        if (phoneNumber != null) {
                            phoneNumber = phoneNumber.trim();
                        }
                        if (TextUtils.isEmpty(phoneNumber)) {
                            continue;
                        }
                        int type = (typeAsObject != null ? typeAsObject : Phone.TYPE_HOME);
                        int isPrimary = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Phone.IS_PRIMARY));

                        contactStruct.addPhone(type, phoneNumber, label, (isPrimary == 1) ? true
                                : false);
                    }
                    else if (mime_type.equals(Email.CONTENT_ITEM_TYPE))
                    {
                        Integer typeAsObject = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Email.TYPE));
                        int type = (typeAsObject != null ?
                                typeAsObject : Email.TYPE_OTHER);
                        String label = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Email.LABEL));
                        String emailAddress = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Email.DATA));
                        if (emailAddress != null) {
                            emailAddress = emailAddress.trim();
                        }
                        if (TextUtils.isEmpty(emailAddress)) {
                            continue;
                        }
                        int isPrimary = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Email.IS_PRIMARY));

                        contactStruct.addEmail(type, emailAddress, label, (isPrimary == 1) ? true
                                : false);
                    }

                    else if (mime_type.equals(StructuredPostal.CONTENT_ITEM_TYPE))
                    {
                        Integer type = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.TYPE));
                        String label = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.LABEL));
                        int isPrimary = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.IS_PRIMARY));
                        List<String> postalList = new ArrayList<String>();
                        postalList.add(0, entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.POBOX)));
                        postalList.add(1, entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.NEIGHBORHOOD)));
                        postalList.add(2, entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.STREET)));
                        postalList.add(3, entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.CITY)));
                        postalList.add(4, entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.REGION)));
                        postalList.add(5, entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.POSTCODE)));
                        postalList.add(6, entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(StructuredPostal.COUNTRY)));

                        contactStruct.addPostal(type, postalList, label, (isPrimary == 1) ? true
                                : false);

                    }
                    else if (mime_type.equals(Im.CONTENT_ITEM_TYPE))
                    {
                        Integer protocol = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Im.PROTOCOL));
                        Integer type = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Im.TYPE));
                        String label = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Im.LABEL));
                        String data = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Im.DATA));
                        String custom_protocoldata = entityIteratorCursor
                                .getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(Im.CUSTOM_PROTOCOL));
                        if (data != null) {
                            data = data.trim();
                        }
                        if (TextUtils.isEmpty(data)) {
                            continue;
                        }
                        int isPrimary = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Im.IS_PRIMARY));

                        contactStruct.addIm(protocol, custom_protocoldata, type, data,
                                (isPrimary == 1) ? true : false);
                    }
                    else if (mime_type.equals(Website.CONTENT_ITEM_TYPE))
                    {
                        Integer typeAsObject = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Website.TYPE));
                        int type = (typeAsObject != null ?
                                typeAsObject : Website.TYPE_OTHER);
                        String label = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Website.LABEL));
                        String website = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Website.URL));
                        if (website != null) {
                            website = website.trim();
                        }
                        if (TextUtils.isEmpty(website)) {
                            continue;
                        }
                        int isPrimary = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Website.IS_PRIMARY));

                        contactStruct.addWebsite(type, website, label, (isPrimary == 1) ? true
                                : false);
                    }
                    else if (mime_type.equals(Event.CONTENT_ITEM_TYPE))
                    {
                        Integer typeAsObject = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Event.TYPE));
                        int type = (typeAsObject != null ?
                                typeAsObject : Event.TYPE_OTHER);
                        String label = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Event.LABEL));
                        String event = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Event.START_DATE));
                        if (event != null) {
                            event = event.trim();
                        }
                        if (TextUtils.isEmpty(event)) {
                            continue;
                        }
                        if (type == Event.TYPE_BIRTHDAY) {
                            contactStruct.mBirthday = event;
                        }
                        // contactStruct.addEvent(type, event, label);
                    }
                    else if (mime_type.equals(Organization.CONTENT_ITEM_TYPE))
                    {
                        Integer type = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Organization.TYPE));
                        String label = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Organization.LABEL));
                        int isPrimary = entityIteratorCursor.getInt(entityIteratorCursor
                                .getColumnIndexOrThrow(Organization.IS_PRIMARY));

                        String company = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Organization.COMPANY));
                        if (company != null) {
                            company = company.trim();
                        }
                        String title = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Organization.TITLE));
                        if (title != null) {
                            title = title.trim();
                        }
                        ContentValues data = new ContentValues();
                        data.put("Organization.COMPANY",
                                entityIteratorCursor.getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(Organization.COMPANY)));
                        data.put("Organization.TITLE",
                                entityIteratorCursor.getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(Organization.TITLE)));
                        data.put("Organization.DEPARTMENT",
                                entityIteratorCursor.getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(Organization.DEPARTMENT)));
                        data.put("Organization.JOB_DESCRIPTION",
                                entityIteratorCursor.getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(Organization.JOB_DESCRIPTION)));
                        data.put("Organization.SYMBOL",
                                entityIteratorCursor.getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(Organization.SYMBOL)));
                        data.put("Organization.PHONETIC_NAME",
                                entityIteratorCursor.getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(Organization.PHONETIC_NAME)));
                        data.put("Organization.OFFICE_LOCATION",
                                entityIteratorCursor.getString(entityIteratorCursor
                                        .getColumnIndexOrThrow(Organization.OFFICE_LOCATION)));

                        // contactStruct.addOrganization(type, company, title,
                        // (isPrimary==1)?true:false);
                        contactStruct
                                .addNewOrganization(type, data.getAsString("Organization.COMPANY"),
                                        data.getAsString("Organization.DEPARTMENT"), data
                                                .getAsString("Organization.TITLE"),
                                        (isPrimary == 1) ? true : false);

                    }
                    else if (mime_type.equals(Photo.CONTENT_ITEM_TYPE))
                    {
                        byte[] data = entityIteratorCursor.getBlob(entityIteratorCursor
                                .getColumnIndexOrThrow(Photo.PHOTO));
                        if (data == null) {
                            continue;
                        }
                        final String photoType;
                        // Use some heuristics for guessing the format of the
                        // image.
                        // TODO: there should be some general API for detecting
                        // the file format.
                        if (data.length >= 3 && data[0] == 'G' && data[1] == 'I'
                                && data[2] == 'F') {
                            photoType = "GIF";
                        } else if (data.length >= 4 && data[0] == (byte) 0x89
                                && data[1] == 'P' && data[2] == 'N' && data[3] == 'G') {
                            // Note: vCard 2.1 officially does not support PNG,
                            // but we
                            // may have it
                            // and using X- word like "X-PNG" may not let
                            // importers know
                            // it is
                            // PNG. So we use the String "PNG" as is...
                            photoType = "PNG";
                        } else if (data.length >= 2 && data[0] == (byte) 0xff
                                && data[1] == (byte) 0xd8) {
                            photoType = "JPEG";
                        } else {
                            Log.d(TAG, "Unknown photo type. Ignore.");
                            continue;
                        }
                        // final String photoString =
                        // VCardUtils.encodeBase64(data);
                        contactStruct.addPhotoBytes(photoType, data, false);

                    }
                    else if (mime_type.equals(Note.CONTENT_ITEM_TYPE))
                    {
                        String noteStr = entityIteratorCursor.getString(entityIteratorCursor
                                .getColumnIndexOrThrow(Note.NOTE));
                        if (TextUtils.isEmpty(noteStr)) {
                            continue;
                        }
                        contactStruct.addNote(noteStr);
                    }
                }
            }
        } catch (Exception e) {
            Log.e(TAG, String.format("RemoteException at id %s (%s)",
                    contactId, e.getMessage()));
        } finally {
            if (entityIteratorCursor != null) {
                entityIteratorCursor.close();
            }
        }

        return contactStruct;

    }

    static public int save(ContentResolver resolver, VCardEntry VCardEntry)
    {
        if (VCardEntry == null)
        {
            return -1;
        }
        switch (VCardEntry.mMemoryType)
        {
            case ContactsContract.MEMORY_TYPE_PHONE:
            default:
                return saveToPhone(resolver, VCardEntry);
                // break;

            case ContactsContract.MEMORY_TYPE_UIM:
            case ContactsContract.MEMORY_TYPE_SIM:
            case ContactsContract.MEMORY_TYPE_USIM:
                return saveToUsim(resolver, VCardEntry);
                // break;
        }
    }

    static public int modify(ContentResolver resolver, VCardEntry VCardEntry)
    {
        if (VCardEntry == null)
        {
            return -1;
        }
        switch (VCardEntry.mMemoryType)
        {
            case ContactsContract.MEMORY_TYPE_PHONE:
            default:
                return modifyPhone(resolver, VCardEntry);
                // break;

            case ContactsContract.MEMORY_TYPE_UIM:
            case ContactsContract.MEMORY_TYPE_SIM:
            case ContactsContract.MEMORY_TYPE_USIM:
                return modifyUsim(resolver, VCardEntry);
                // break;
        }

    }

    static public int saveToPhone(ContentResolver resolver, VCardEntry VCardEntry)
    {
        VCardEntry.pushIntoContentResolver(resolver);
        return 1;
    }

    static public int saveToUsim(ContentResolver resolver, VCardEntry VCardEntry)
    {
        if (VCardEntry == null)
        {
            return -1;
        }
        Uri uri = null;

        if (VCardEntry.mMemoryType == ContactsContract.MEMORY_TYPE_SIM)
        {
            uri = Uri.parse("content://icc/adn");
        }
        else
        {
            uri = Uri.parse("content://icc/usim_adn");
        }

        ContentValues bundle = new ContentValues(3);
        bundle.put("index", VCardEntry.mNlocation);
        bundle.put("tag", VCardEntry.mFamilyName);
        String number = "";
        if (VCardEntry.getPhoneList() != null)
        {
            if ((VCardEntry.getPhoneList().get(0) != null))
            {
                number = VCardEntry.getPhoneList().get(0).data;
            }
        }
        bundle.put("number", number);
        Uri retUri = resolver.insert(uri, bundle);

        return (retUri != null) ? 1 : -1;
    }

    static public int modifyUsim(ContentResolver resolver, VCardEntry VCardEntry)
    {
        if (VCardEntry == null)
        {
            return -1;
        }
        Uri uri = null;

        if (VCardEntry.mMemoryType == ContactsContract.MEMORY_TYPE_SIM)
        {
            uri = Uri.parse("content://icc/adn");
        }
        else
        {
            uri = Uri.parse("content://icc/usim_adn");
        }
        ContentValues bundle = new ContentValues();
        bundle.put("index", VCardEntry.mNlocation);
        bundle.put("tag", "");
        bundle.put("number", "");
        bundle.put("newTag", VCardEntry.mFamilyName);
        String number = "";
        if (VCardEntry.getPhoneList() != null)
        {
            if ((VCardEntry.getPhoneList().get(0) != null))
            {
                number = VCardEntry.getPhoneList().get(0).data;
            }
        }
        bundle.put("newNumber", number);
        return (resolver.update(uri, bundle, null, null));
    }

    static public int modifyPhone(ContentResolver resolver, VCardEntry VCardEntry)
    {
        if (VCardEntry == null)
        {
            return -1;
        }

        Cursor c = resolver.query(RawContacts.RECORDS_CONTENT_URI, new String[] {
            RawContacts._ID
        },
                Contacts.NLOCATION + "=? AND " + Contacts.MEMORY_TYPE + "=?",
                new String[] {
                        Integer.toString(VCardEntry.mNlocation),
                        Integer.toString(VCardEntry.mMemoryType)
                }, null);

        if (c == null) {
            return -1;
        }
        long rawContactId = -1L;
        try {
            if (c.moveToFirst()) {
                rawContactId = c.getLong(0);
            }
        } finally {
            c.close();
        }

        ArrayList<ContentProviderOperation> operationList =
                new ArrayList<ContentProviderOperation>();
        String myGroupsId = null;

        ContentProviderOperation.Builder builder =
                ContentProviderOperation.newUpdate(RawContacts.CONTENT_URI);
        builder.withSelection(RawContacts._ID + " = " + rawContactId, null);
        builder.withValue(RawContacts.AGGREGATION_MODE, RawContacts.AGGREGATION_MODE_DISABLED);
        // builder.withValue(Contacts.DISPLAY_NAME,
        // contactStructEx.mDisplayName);
        operationList.add(builder.build());

        builder = ContentProviderOperation.newDelete(Data.CONTENT_URI);
        builder.withSelection(Data.RAW_CONTACT_ID + " = " + rawContactId, null);
        operationList.add(builder.build());

        // return 1;
        {
            builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
            // builder.withValueBackReference(StructuredName.RAW_CONTACT_ID, 0);
            builder.withValue(StructuredName.RAW_CONTACT_ID, rawContactId);
            builder.withValue(Data.MIMETYPE, StructuredName.CONTENT_ITEM_TYPE);

            builder.withValue(StructuredName.GIVEN_NAME, VCardEntry.mGivenName);
            builder.withValue(StructuredName.FAMILY_NAME, VCardEntry.mFamilyName);
            builder.withValue(StructuredName.MIDDLE_NAME, VCardEntry.mMiddleName);
            builder.withValue(StructuredName.PREFIX, VCardEntry.mPrefix);
            builder.withValue(StructuredName.SUFFIX, VCardEntry.mSuffix);

            builder.withValue(StructuredName.PHONETIC_GIVEN_NAME, VCardEntry.mPhoneticGivenName);
            builder.withValue(StructuredName.PHONETIC_FAMILY_NAME, VCardEntry.mPhoneticFamilyName);
            builder.withValue(StructuredName.PHONETIC_MIDDLE_NAME, VCardEntry.mPhoneticMiddleName);
            // builder.withValue(StructuredName.DISPLAY_NAME,
            // VCardEntry.getDisplayName());
            operationList.add(builder.build());
        }

        if (VCardEntry.mNickNameList != null && VCardEntry.mNickNameList.size() > 0) {
            boolean first = true;
            for (String nickName : VCardEntry.mNickNameList) {
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                // builder.withValueBackReference(Nickname.RAW_CONTACT_ID, 0);
                builder.withValue(Nickname.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, Nickname.CONTENT_ITEM_TYPE);

                builder.withValue(Nickname.TYPE, Nickname.TYPE_DEFAULT);
                builder.withValue(Nickname.NAME, nickName);
                if (first) {
                    builder.withValue(Data.IS_PRIMARY, 1);
                    first = false;
                }
                operationList.add(builder.build());
            }
        }

        if (VCardEntry.mPhoneList != null) {
            for (PhoneData phoneData : VCardEntry.mPhoneList) {
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                // builder.withValueBackReference(Phone.RAW_CONTACT_ID, 0);
                builder.withValue(Phone.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE);

                builder.withValue(Phone.TYPE, phoneData.type);
                if (phoneData.type == Phone.TYPE_CUSTOM) {
                    builder.withValue(Phone.LABEL, phoneData.label);
                }
                builder.withValue(Phone.NUMBER, phoneData.data);
                if (phoneData.isPrimary) {
                    builder.withValue(Data.IS_PRIMARY, 1);
                }
                operationList.add(builder.build());
            }
        }

        if (VCardEntry.mOrganizationList != null) {
            boolean first = true;
            for (OrganizationData organizationData : VCardEntry.mOrganizationList) {
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                // builder.withValueBackReference(Organization.RAW_CONTACT_ID,
                // 0);
                builder.withValue(Organization.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, Organization.CONTENT_ITEM_TYPE);

                // Currently, we do not use TYPE_CUSTOM.
                builder.withValue(Organization.TYPE, organizationData.type);
                builder.withValue(Organization.COMPANY, organizationData.companyName);
                builder.withValue(Organization.TITLE, organizationData.titleName);
                builder.withValue(Organization.DEPARTMENT, organizationData.departmentName);

                if (first) {
                    builder.withValue(Data.IS_PRIMARY, 1);
                }
                operationList.add(builder.build());
            }
        }

        if (VCardEntry.mEmailList != null) {
            for (EmailData emailData : VCardEntry.mEmailList) {
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                // builder.withValueBackReference(Email.RAW_CONTACT_ID, 0);
                builder.withValue(Email.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, Email.CONTENT_ITEM_TYPE);

                builder.withValue(Email.TYPE, emailData.type);
                if (emailData.type == Email.TYPE_CUSTOM) {
                    builder.withValue(Email.LABEL, emailData.label);
                }
                builder.withValue(Email.DATA, emailData.data);
                if (emailData.isPrimary) {
                    builder.withValue(Data.IS_PRIMARY, 1);
                }
                operationList.add(builder.build());
            }
        }

        if (VCardEntry.mPostalList != null) {
            for (PostalData postalData : VCardEntry.mPostalList) {
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                // VCardUtils.insertStructuredPostalDataUsingContactsStruct(
                // mVCardType, builder, postalData);
                // builder.withValueBackReference(StructuredPostal.RAW_CONTACT_ID,
                // 0);
                builder.withValue(StructuredPostal.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, StructuredPostal.CONTENT_ITEM_TYPE);

                builder.withValue(StructuredPostal.TYPE, postalData.type);
                if (postalData.type == StructuredPostal.TYPE_CUSTOM) {
                    builder.withValue(StructuredPostal.LABEL, postalData.label);
                }

                builder.withValue(StructuredPostal.POBOX, postalData.pobox);
                // Extended address is dropped since there's no relevant entry
                // in ContactsContract.
                builder.withValue(StructuredPostal.NEIGHBORHOOD, postalData.extendedAddress);
                builder.withValue(StructuredPostal.STREET, postalData.street);
                builder.withValue(StructuredPostal.CITY, postalData.localty);
                builder.withValue(StructuredPostal.REGION, postalData.region);
                builder.withValue(StructuredPostal.POSTCODE, postalData.postalCode);
                builder.withValue(StructuredPostal.COUNTRY, postalData.country);

                builder.withValue(StructuredPostal.FORMATTED_ADDRESS,
                        postalData.getFormattedAddress(mVCardType));
                if (postalData.isPrimary) {
                    builder.withValue(Data.IS_PRIMARY, 1);
                }
                operationList.add(builder.build());
            }
        }

        if (VCardEntry.mImList != null) {
            for (ImData imData : VCardEntry.mImList) {
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                // builder.withValueBackReference(Im.RAW_CONTACT_ID, 0);
                builder.withValue(Im.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, Im.CONTENT_ITEM_TYPE);

                builder.withValue(Im.TYPE, imData.type);
                if (imData.type == Im.TYPE_CUSTOM) {
                    builder.withValue(Im.LABEL, imData.customProtocol);
                }
                builder.withValue(Im.DATA, imData.data);
                builder.withValue(Im.PROTOCOL, imData.protocol);
                /*
                 * if (imData.protocol == Im.PROTOCOL_CUSTOM) {
                 * builder.withValue(Im.CUSTOM_PROTOCOL,
                 * imData.custom_protocoldata); }
                 */
                if (imData.isPrimary) {
                    builder.withValue(Data.IS_PRIMARY, 1);
                }
                operationList.add(builder.build());
            }
        }

        if (VCardEntry.mNoteList != null) {
            for (String note : VCardEntry.mNoteList) {
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                // builder.withValueBackReference(Note.RAW_CONTACT_ID, 0);
                builder.withValue(Note.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, Note.CONTENT_ITEM_TYPE);

                builder.withValue(Note.NOTE, note);
                operationList.add(builder.build());
            }
        }

        if (VCardEntry.mPhotoList != null) {
            boolean first = true;
            for (PhotoData photoData : VCardEntry.mPhotoList) {
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                // builder.withValueBackReference(Photo.RAW_CONTACT_ID, 0);
                builder.withValue(Photo.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, Photo.CONTENT_ITEM_TYPE);
                builder.withValue(Photo.PHOTO, photoData.photoBytes);
                if (first) {
                    builder.withValue(Data.IS_PRIMARY, 1);
                    first = false;
                }
                operationList.add(builder.build());
            }
        }

        if (VCardEntry.mWebsiteList != null) {
            for (String website : VCardEntry.mWebsiteList) {
                builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
                // builder.withValueBackReference(Website.RAW_CONTACT_ID, 0);
                // builder.withValueBackReference(Website.RAW_CONTACT_ID,
                // rawContactId);
                builder.withValue(Website.RAW_CONTACT_ID, rawContactId);
                builder.withValue(Data.MIMETYPE, Website.CONTENT_ITEM_TYPE);
                builder.withValue(Website.URL, website);
                // There's no information about the type of URL in vCard.
                // We use TYPE_HOME for safety.
                builder.withValue(Website.TYPE, Website.TYPE_HOME);
                operationList.add(builder.build());
            }
        }

        /*
         * if (VCardEntry.mWebsiteList != null) { for (Website websiteData :
         * VCardEntry.mWebsiteList) { builder =
         * ContentProviderOperation.newInsert(Data.CONTENT_URI);
         * //builder.withValueBackReference(Website.RAW_CONTACT_ID, 0);
         * builder.withValue(Website.RAW_CONTACT_ID, rawContactId);
         * builder.withValue(Data.MIMETYPE, Website.CONTENT_ITEM_TYPE);
         * builder.withValue(Website.TYPE, websiteData.type); if
         * (websiteData.type == Website.TYPE_OTHER) {
         * builder.withValue(Website.LABEL, websiteData.label); }
         * builder.withValue(Website.URL, websiteData.data); if
         * (websiteData.isPrimary) { builder.withValue(Data.IS_PRIMARY, 1); }
         * operationList.add(builder.build()); } }
         */

        if (!TextUtils.isEmpty(VCardEntry.mBirthday)) {
            builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
            // builder.withValueBackReference(Event.RAW_CONTACT_ID, 0);
            builder.withValue(Event.RAW_CONTACT_ID, rawContactId);
            builder.withValue(Data.MIMETYPE, Event.CONTENT_ITEM_TYPE);
            builder.withValue(Event.START_DATE, VCardEntry.mBirthday);
            builder.withValue(Event.TYPE, Event.TYPE_BIRTHDAY);
            operationList.add(builder.build());
        }

        if (myGroupsId != null) {
            builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
            // builder.withValueBackReference(GroupMembership.RAW_CONTACT_ID,
            // 0);
            builder.withValue(GroupMembership.RAW_CONTACT_ID, rawContactId);
            builder.withValue(Data.MIMETYPE, GroupMembership.CONTENT_ITEM_TYPE);
            builder.withValue(GroupMembership.GROUP_SOURCE_ID, myGroupsId);
            operationList.add(builder.build());
        }

        try {
            resolver.applyBatch(ContactsContract.AUTHORITY, operationList);
        } catch (RemoteException e) {
            Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
        } catch (OperationApplicationException e) {
            Log.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
        }
        return 1;
    }

    static public Uri getContactsUriByLocation(Context context, int location, int device)
    {
        int id = -1;
        Cursor c = context.getContentResolver().query(Contacts.RECORDS_CONTENT_URI,
                new String[] {
                    Contacts._ID
                },
                Contacts.MEMORY_TYPE + " = " + device, null, null);

        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    id = c.getInt(0);
                }
            } finally {
                c.close();
            }
        }
        return ContentUris.withAppendedId(Contacts.CONTENT_URI, id);
    }

    /**
     * Output {@link RawContacts} matching the requested selection in the vCard
     * format to the given {@link OutputStream}. This method returns silently if
     * any errors encountered.
     */
    static public String readContactsAsVCard(Context context, Uri uri, String selection,
            String[] selectionArgs) {

        VCardComposer composer = new VCardComposer(context, mVCardType, false);
        HandlerForOutputVcardString handlerForOutputVcardString = new HandlerForOutputVcardString(
                context);
        composer.addHandler(handlerForOutputVcardString);

        // No extra checks since composer always uses restricted views
        if (!composer.init(uri, selection, selectionArgs, null))
            return "";

        while (!composer.isAfterLast()) {
            if (!composer.createOneEntry()) {
                Log.w(TAG, "Failed to output a contact.");
            }
        }
        composer.terminate();

        return handlerForOutputVcardString.getVcardString();
    }

    /**
     * <p>
     * An useful example handler, which emits VCard String to outputstream one
     * by one.
     * </p>
     * <p>
     * The input OutputStream object is closed() on {{@link #onTerminate()}.
     * Must not close the stream outside.
     * </p>
     */
    static private class HandlerForOutputVcardString implements OneEntryHandler {
        @SuppressWarnings("hiding")
        private String mVcard = "";

        /**
         * Input stream will be closed on the detruction of this object.
         */
        public HandlerForOutputVcardString(Context context) {
        }

        public boolean onInit(Context context) {
            return true;
        }

        public boolean onEntryCreated(String vcard) {
            mVcard += vcard;
            return true;
        }

        public void onTerminate() {

        }

        @Override
        public void finalize() {
            onTerminate();
        }

        // called after finalize
        public String getVcardString()
        {
            return mVcard;
        }

    }

    /**
     * EntryHandler implementation which commits the entry to Contacts Provider
     */
    static public class EntryCommitterEx implements VCardEntryHandler {

        private ContentResolver mContentResolver;
        private long mTimeToCommit;
        boolean mIsUpdate = false;
        int mNlocation = -1;

        public EntryCommitterEx(ContentResolver resolver, boolean isUpdate, int nlocation) {
            mContentResolver = resolver;
            mIsUpdate = isUpdate;
            mNlocation = nlocation;
        }

        public void onStart() {
        }

        public void onEnd() {
            if (VCardConfig.showPerformanceLog()) {
                Log.d(TAG, String.format("time to commit entries: %d ms", mTimeToCommit));
            }
        }

        public void onEntryCreated(final VCardEntry contactStruct) {
            long start = System.currentTimeMillis();
            if (mIsUpdate)
            {
                Cursor c = mContentResolver.query(RawContacts.CONTENT_URI,
                        new String[] {
                            RawContacts._ID
                        },
                        Contacts.NLOCATION + "=? AND " + Contacts.MEMORY_TYPE + "=?",
                        new String[] {
                                Integer.toString(mNlocation),
                                Integer.toString(ContactsContract.MEMORY_TYPE_PHONE)
                        },
                        null
                        );
                if (c == null) {
                    return;
                }
                long rawContactId = -1L;
                try {
                    if (c.moveToFirst()) {
                        rawContactId = c.getLong(0);
                    }
                } finally {
                    c.close();
                }
                contactStruct.updateContentResolver(mContentResolver, rawContactId);
            }
            else
            {
                contactStruct.pushIntoContentResolver(mContentResolver);
            }
            mTimeToCommit += System.currentTimeMillis() - start;
        }
    }

    static public boolean saveToPhoneFromVcardData(Context context, byte[] vdata, int nlocation)
    {
        VCardSourceDetector detector = new VCardSourceDetector();
        String charset = detector.getEstimatedCharset();
        return ParseOneVCard(context, vdata, null, charset, detector, false, nlocation);
    }

    static public boolean modifyPhoneFromVcardData(Context context, byte[] vdata, int nlocation)
    {
        VCardSourceDetector detector = new VCardSourceDetector();
        String charset = detector.getEstimatedCharset();
        return ParseOneVCard(context, vdata, null, charset, detector, true, nlocation);
    }

    static private boolean ParseOneVCard(Context context, byte[] vdata, Account account,
            String charset, VCardSourceDetector detector, boolean isUpdate, int nlocation) {
        VCardEntryConstructor builder;
        int vcardType = mVCardType;
        if (charset != null) {
            builder = new VCardEntryConstructor(charset, charset, false, vcardType, account);
        } else {
            charset = VCardConfig.DEFAULT_CHARSET;
            builder = new VCardEntryConstructor(null, null, false, vcardType, account);
        }
        builder.addEntryHandler(new EntryCommitterEx(context.getContentResolver(),
                isUpdate, nlocation));

        try {
            if (!readOneVCardStream(vdata, charset, builder, detector, false)) {
                return false;
            }
        } catch (VCardNestedException e) {
            Log.e(TAG, "Never reach here.");
        }
        return true;
    }

    static private boolean readOneVCardStream(byte[] vdata, String charset,
            VCardInterpreter builder, VCardSourceDetector detector,
            boolean throwNestedException)
            throws VCardNestedException {
        InputStream is;
        try {
            is = new ByteArrayInputStream(vdata);
            VCardParser_V21 mVCardParser = new VCardParser_V21(detector);

            try {
                mVCardParser.parse(is, charset, builder, false);
            } catch (VCardVersionException e1) {
                try {
                    is.close();
                } catch (IOException e) {
                }
                is = new ByteArrayInputStream(vdata);

                try {
                    mVCardParser = new VCardParser_V30();
                    mVCardParser.parse(is, charset, builder, false);
                } catch (VCardVersionException e2) {
                    throw new VCardException("vCard with unspported version.");
                }
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                    }
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "IOException was emitted: " + e.getMessage());
            return false;
        } catch (VCardNotSupportedException e) {
            Log.e(TAG, "VCardNotSupportedException : " + e.getMessage());
            if ((e instanceof VCardNestedException) && throwNestedException) {
                throw (VCardNestedException) e;
            }
            return false;
        } catch (VCardException e) {
            Log.e(TAG, "VCardException : " + e.getMessage());
            return false;
        }
        return true;
    }

    // huangjiufa add 20100105 for text codec translation
    static public String getTextcodecFromContent(byte[] str, int size)
    {

        if (str[0] == '\0' || size <= 0) {
            return null;
        }

        boolean bIsUtf8 = true;
        boolean bIsGbk = true;
        boolean bIsGb2312 = true;

        if ((str[0] == (byte) 0xff && str[1] == (byte) 0xfe)
                || (str[0] == (byte) 0xfe && str[1] == (byte) 0xff)) {
            return CODEC_UNICODE;
        }

        if (str[0] == 0) {
            return CODEC_UNICODE;
        }

        for (int i = 0; i < size;) {
            if (str[i] >= 0 && str[i] <= 0x7f) {
                i++;
            }
            else if ((i + 1 < size)
                    && ((str[i] & (byte) 0xe0) == (byte) 0xc0)
                    && ((str[i + 1] & (byte) 0xc0) == (byte) 0x80)) {
                i += 2;
            }
            else if ((i + 2 < size)
                    && ((str[i] & (byte) 0xf0) == (byte) 0xe0)
                    && ((str[i + 1] & (byte) 0xc0) == (byte) 0x80)
                    && ((str[i + 2] & (byte) 0xc0) == (byte) 0x80)) {
                i += 3;
            }
            else {
                bIsUtf8 = false;
                break;
            }
        }

        if (bIsUtf8) {
            // System.out.println(TAG,"4704");
            return CODEC_UTF8;
        }

        for (int i = 0; i + 2 < size;) {
            if (str[i] >= 0 && str[i] <= 0x7f) {
                i++;
            }
            else if (str[i] < 0 && str[i] >= (byte) 0x81) {
                if (str[i] != (byte) 0xff) {
                    if ((str[i] >= 0 && str[i] <= 0x7f)
                            || (str[i] < 0 && str[i] < (byte) 0xa1)) {
                        bIsGb2312 = false;
                    }
                    if (str[i + 1] >= 0x40 || str[i + 1] < 0) {
                        if ((str[i + 1] != (byte) 0xff) && (str[i + 1] != 0x7f)) {
                            if ((str[i + 1] >= 0 && str[i + 1] <= 0x7f)
                                    || (str[i + 1] < 0 && str[i + 1] < (byte) 0xa1)) {
                                bIsGb2312 = false;
                            }
                            i += 2;
                        }
                        else {
                            bIsGbk = false;
                            bIsGb2312 = false;
                            break;
                        }
                    }
                    else {
                        bIsGbk = false;
                        bIsGb2312 = false;
                        break;
                    }
                }
                else {
                    bIsGbk = false;
                    bIsGb2312 = false;
                    break;
                }
            }
            else {
                bIsGbk = false;
                bIsGb2312 = false;
                break;
            }
        }

        if (bIsGb2312) {
            return CODEC_GB2312;
        }
        if (bIsGbk) {
            return CODEC_GBK;
        }

        return CODEC_UNICODE;
    }

    /*
     * this func can read a File's content and get text codec the param "f" is
     * the File you want to operate
     */
    static public String getTextcodecFromFile(File f) throws IOException {
        InputStream is = null;
        String ret = "UTF-8";
        try {
            is = new BufferedInputStream(new FileInputStream(f));
            long contentLength = f.length();
            ByteArrayOutputStream outstream = new ByteArrayOutputStream(
                    contentLength > 0 ? (int) contentLength : 1024);
            byte[] buffer = new byte[4096];
            int len;
            while ((len = is.read(buffer)) > 0) {
                outstream.write(buffer, 0, len);
                // only parse 4096 byte
                break;
            }
            byte[] b_tmp = outstream.toByteArray();
            ret = getTextcodecFromContent(b_tmp, outstream.size());
            outstream.close();
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (Exception e) {
                }
            }
        }
        return ret;
    }

    // zhl add for pim
    static public String pimReadContactsAsVCard(Context context, Uri uri, String selection,
            String[] selectionArgs) {

        VCardComposer composer = new VCardComposer(context, mVCardType, false);
        HandlerForOutputVcardString handlerForOutputVcardString = new HandlerForOutputVcardString(
                context);
        composer.addHandler(handlerForOutputVcardString);

        final boolean isPim = true;
        // No extra checks since composer always uses restricted views
        if (!composer.init(uri, selection, selectionArgs, null))
            return "";

        while (!composer.isAfterLast()) {
            if (!composer.createOneEntry()) {
                Log.w(TAG, "Failed to output a contact.");
            }
        }
        composer.terminate();

        return handlerForOutputVcardString.getVcardString();
    }

    static public boolean pimSaveToPhoneFromVcardData(Context context, byte[] vdata, int nlocation)
    {
        VCardSourceDetector detector = new VCardSourceDetector();
        String charset = detector.getEstimatedCharset();
        return pimParseOneVCard(context, vdata, null, charset, detector, false, nlocation);
    }

    static public boolean pimModifyPhoneFromVcardData(Context context, byte[] vdata, int nlocation)
    {
        VCardSourceDetector detector = new VCardSourceDetector();
        String charset = detector.getEstimatedCharset();
        return pimParseOneVCard(context, vdata, null, charset, detector, true, nlocation);
    }

    static private boolean pimParseOneVCard(Context context, byte[] vdata, Account account,
            String charset, VCardSourceDetector detector, boolean isUpdate, int nlocation) {
        VCardEntryConstructor builder;
        int vcardType = mVCardType;

        final boolean pimUse = true;

        if (charset != null) {
            builder = new VCardEntryConstructor(charset, charset, false, vcardType, account);
        } else {
            charset = VCardConfig.DEFAULT_CHARSET;
            builder = new VCardEntryConstructor(null, null, false, vcardType, account);
        }
        builder.addEntryHandler(new EntryCommitterEx(context.getContentResolver(),
                isUpdate, nlocation));

        try {
            if (!readOneVCardStream(vdata, charset, builder, detector, false)) {
                return false;
            }
        } catch (VCardNestedException e) {
            Log.e(TAG, "Never reach here.");
        }
        return true;
    }

    // add end

}
