/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.calendarlocalaccount;

import android.accounts.AbstractAccountAuthenticator;
import android.accounts.Account;
import android.accounts.AccountAuthenticatorResponse;
import android.accounts.AccountManager;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Calendars;
import android.provider.CalendarContract.Colors;
import android.provider.SyncStateContract.Columns;
import android.util.Log;

import com.qualcomm.qti.calendarlocalaccount.R;

import java.util.TimeZone;

public class Authenticator extends AbstractAccountAuthenticator {
    private static final String TAG = "Authenticator";

    public static final String ACCOUNT_TYPE = "com.qualcomm.qti.calendarlocalaccount";

    // The local account.
    private static Account sAccount;

    // The colors could be set for this default account.
    private static final Integer[] COLORS = new Integer[] {
            Color.BLACK,
            Color.DKGRAY,
            Color.GRAY,
            Color.LTGRAY,
            Color.WHITE,
            Color.RED,
            Color.GREEN,
            Color.BLUE,
            Color.YELLOW,
            Color.CYAN,
            Color.MAGENTA
    };

    // Authentication Service context
    private final Context mContext;

    public Authenticator(Context context) {
        super(context);
        mContext = context;
        sAccount = new Account("Local", ACCOUNT_TYPE);
    }

    @Override
    public Bundle addAccount(AccountAuthenticatorResponse response,
            String accountType, String authTokenType, String[] requiredFeatures, Bundle options) {
        // 1. Check the account manager if contains the calendar local account.
        AccountManager am = AccountManager.get(mContext);
        Account[] accounts = am.getAccountsByType(ACCOUNT_TYPE);

        switch (accounts.length) {
            case 0:
                Log.d(TAG, "There isn't any calendar local account, add one local account.");
                // Add the account to account manager.
                if (am.addAccountExplicitly(sAccount, null, null)) {
                    // As this account is local account, needn't sync.
                    ContentResolver.setIsSyncable(sAccount, sAccount.type, 0);
                    ContentResolver.setSyncAutomatically(sAccount, sAccount.type, false);
                }
                break;
            case 1:
                // It means already add the calendar local account, do nothing.
                Log.d(TAG, "There is one calendar local account, needn't add to account manager.");
                break;
            default:
                // Shouldn't be here.
                Log.e(TAG, "There are " + accounts.length + " accounts as \""
                        + ACCOUNT_TYPE + "\".");
                return null;
        }

        // 2. Add the account to Calendar database.
        ContentResolver contentResolver = mContext.getContentResolver();
        if (!isExistCalendarAccount(contentResolver)) {
            // Insert the account.
            Uri res = insertAccount(contentResolver, sAccount,
                    mContext.getString(R.string.local_account_name));
            Log.d(TAG, "There isn't the local account, add the local " + sAccount
                    + " success. Account uri = " + res);
            // Insert the colors for this local account.
            for (int i = 0; i < COLORS.length; i++) {
                insertCalendarColor(contentResolver, sAccount, i, COLORS[i]);
                insertEventColor(contentResolver, sAccount, i + COLORS.length, COLORS[i]);
            }
        }

        return new Bundle();
    }

    @Override
    public Bundle editProperties(AccountAuthenticatorResponse response,
            String accountType) {
        ContentResolver contentResolver = mContext.getContentResolver();
        if (isExistCalendarAccount(contentResolver)) {
            // Update the properties as the locale.
            ContentValues values = new ContentValues();
            String newName = mContext.getString(R.string.local_account_name);
            values.put(Calendars.CALENDAR_DISPLAY_NAME, newName);
            //values.put(Calendars.OWNER_ACCOUNT, newName);
            String whereClause = Calendars.ACCOUNT_TYPE + "='" + ACCOUNT_TYPE + "'";
            int res = contentResolver.update(
                    addSyncQueryParams(Calendars.CONTENT_URI, sAccount.name, sAccount.type),
                    values, whereClause, null);
            Log.d(TAG, "Update the calendar's display name to " + newName
                    + " , update " + res + " records.");
        }
        return new Bundle();
    }

    @Override
    public Bundle confirmCredentials(AccountAuthenticatorResponse response,
            Account account, Bundle options) {
        return null;
    }

    @Override
    public Bundle getAuthToken(AccountAuthenticatorResponse response,
            Account account, String authTokenType, Bundle loginOptions) {
        return null;
    }

    @Override
    public String getAuthTokenLabel(String authTokenType) {
        return null;

    }

    @Override
    public Bundle hasFeatures(AccountAuthenticatorResponse response,
            Account account, String[] features) {
        final Bundle result = new Bundle();
        result.putBoolean(AccountManager.KEY_BOOLEAN_RESULT, false);
        return result;
    }

    @Override
    public Bundle updateCredentials(AccountAuthenticatorResponse response,
            Account account, String authTokenType, Bundle loginOptions) {
        return null;
    }

    private boolean isExistCalendarAccount(ContentResolver contentResolver) {
        Cursor cursor = contentResolver.query(Calendars.CONTENT_URI, null,
                Columns.ACCOUNT_TYPE + "='" + ACCOUNT_TYPE + "'",
                null, null);
        try {
            if (cursor == null) {
                Log.e(TAG, "Check if the Calendar already contains the local account, "
                        + "but the cursor is null.");
                return true;
            }

            if (cursor.getCount() == 0) {
                Log.d(TAG, "There isn't the local account.");
                return false;
            } else {
                Log.d(TAG, "Already contains " + cursor.getCount() + " local account in Calendar.");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }

        return true;
    }

    private Uri insertAccount(ContentResolver contentResolver, Account account,
            String displayName) {
        ContentValues values = new ContentValues();
        values.put(Calendars.NAME, account.name);
        values.put(Calendars.ACCOUNT_NAME, account.name);
        values.put(Calendars.ACCOUNT_TYPE, account.type);
        values.put(Calendars.OWNER_ACCOUNT, account.name);
        values.put(Calendars.CALENDAR_DISPLAY_NAME, displayName);
        values.put(Calendars.CALENDAR_TIME_ZONE, TimeZone.getDefault().getDisplayName());
        values.put(Calendars.VISIBLE, 1);
        values.put(Calendars.CALENDAR_ACCESS_LEVEL, Calendars.CAL_ACCESS_OWNER);
        values.put(Calendars.CALENDAR_COLOR, Color.MAGENTA);
        values.put(Calendars.SYNC_EVENTS, 1);
        values.put(Calendars.CAL_SYNC1, account.name);

        return contentResolver.insert(
                addSyncQueryParams(Calendars.CONTENT_URI, account.name, account.type), values);
    }

    private Uri insertCalendarColor(ContentResolver contentResolver, Account account,
            int colorKey, int color) {
        return insertColor(contentResolver, account, Colors.TYPE_CALENDAR, colorKey, color);
    }

    private Uri insertEventColor(ContentResolver contentResolver, Account account,
            int colorKey, int color) {
        return insertColor(contentResolver, account, Colors.TYPE_EVENT, colorKey, color);
    }

    private Uri insertColor(ContentResolver contentResolver, Account account,
            int type, int colorKey, int color) {
        ContentValues values = new ContentValues();
        values.put(Colors.ACCOUNT_NAME, account.name);
        values.put(Colors.ACCOUNT_TYPE, account.type);
        values.put(Colors.COLOR_TYPE, type);
        values.put(Colors.COLOR_KEY, colorKey);
        values.put(Colors.COLOR, color);

        return contentResolver.insert(
                addSyncQueryParams(Colors.CONTENT_URI, account.name, account.type), values);
    }

    private Uri addSyncQueryParams(Uri uri, String account, String accountType) {
        return uri.buildUpon().appendQueryParameter(CalendarContract.CALLER_IS_SYNCADAPTER, "true")
                .appendQueryParameter(Calendars.ACCOUNT_NAME, account)
                .appendQueryParameter(Calendars.ACCOUNT_TYPE, accountType).build();
    }

}
