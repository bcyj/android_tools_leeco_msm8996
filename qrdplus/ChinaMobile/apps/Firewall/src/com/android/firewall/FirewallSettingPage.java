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

package com.android.firewall;

import java.lang.reflect.Method;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceManager;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.PreferenceCategory;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.net.Uri;
import android.database.Cursor;
import android.telephony.TelephonyManager;
import android.util.Log;

public class FirewallSettingPage extends PreferenceActivity implements
        OnSharedPreferenceChangeListener {
    private static final String TAG = "FirewallSettingPage";
    private static final int PHONE_TYPE_NONE = 0;
    /** Phone radio is GSM. */
    private static final int PHONE_TYPE_GSM = 1;
    /** Phone radio is CDMA. */
    private static final int PHONE_TYPE_CDMA = 2;
    public static final boolean mIsSingleCard = isSingleCard();

    public static final boolean mHasCdma = hasCdma();

    public String blockcard1 = "";
    public String blockcard2 = "";

    public static boolean isSingleCard() {
        try {
            Class<?> c = Class.forName("android.telephony.MSimTelephonyManager");
            Method hideGetDefault = c.getMethod("getDefault");
            Method hideIsMultiCard = c.getMethod("isMultiSimEnabled");
            boolean multiCard = (Boolean) hideIsMultiCard.invoke(hideGetDefault.invoke(c));
            Log.d(TAG, "MultiCard is " + multiCard);
            return !multiCard;
        } catch (Exception e) {
            e.printStackTrace();
            return true;
        }
    }

    public static boolean hasCdma() {
        try {
            Class<?> c = Class
                    .forName("android.telephony.MSimTelephonyManager");
            Method hideGetDefault = c
                    .getMethod("getDefault");
            Method hideGetDefaultSubscription = c
                    .getMethod("getDefaultSubscription");
            Method hideGetPhoneCount = c.getMethod("getPhoneCount");
            Method hideGetPhoneType = c.getMethod("getCurrentPhoneType", int.class);

            int count = (Integer) hideGetPhoneCount.invoke(hideGetDefault.invoke(c));
            int sub = (Integer) hideGetDefaultSubscription.invoke(hideGetDefault.invoke(c));

            for (int i = 0; i < count; i++) {
                int hhh = (Integer) hideGetPhoneType.invoke(hideGetDefault.invoke(c), i);
                Log.d(TAG, "GetPhoneType is " + hhh);
                if (PHONE_TYPE_CDMA == (Integer) hideGetPhoneType.invoke(hideGetDefault.invoke(c),
                        i)) {
                    Log.d(TAG, "has cdma phone");
                    return true;
                }
            }
            return false;
        } catch (Exception e) {
            e.printStackTrace();
            return true;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (mIsSingleCard)
        {
            Log.d(TAG, "mIsSingleCard");
            addPreferencesFromResource(R.xml.firewall_setting);
        }
        else if (mHasCdma)
        {
            Log.d(TAG, "cg double cards");
            addPreferencesFromResource(R.xml.firewall_setting_cg);
        }
        else
        {
            Log.d(TAG, "2g double cards");
            addPreferencesFromResource(R.xml.firewall_setting_2g);
        }
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        prefs.registerOnSharedPreferenceChangeListener(this);

        blockcard1 = mHasCdma ? getString(R.string.firewall_setting_call_block_c)
                : getString(R.string.firewall_setting_call_block_card1);
        blockcard2 = mHasCdma ? getString(R.string.firewall_setting_call_block_g)
                : getString(R.string.firewall_setting_call_block_card2);
    }

    private void GetCurrentSettings() {
        ContentResolver resolver = getContentResolver();
        CheckBoxPreference chk = (CheckBoxPreference) getPreferenceScreen().findPreference(
                "firewall_call_switcher");
        Uri uri = Uri.parse("content://com.android.firewall/setting/1");
        Cursor c = resolver.query(uri,
                new String[] { "value" },"_id = ?",
                new String[] { "1" },null);
        c.moveToFirst();
        if (c.getString(0).equals("0")) {
            chk.setChecked(false);
        } else {
            chk.setChecked(true);
        }
        c.close();
        if (mIsSingleCard)
        {
            Log.d(TAG, "GetCurrentSettings this is single card");
            ListPreference lst = (ListPreference) getPreferenceScreen().findPreference(
                    "firewall_call_mode");
            c = resolver.query(uri,
                    new String[] { "value" }, "_id = ?",
                    new String[] { "2" }, null);
            c.moveToFirst();
            lst.setValue(c.getString(0));
            switch (Integer.valueOf(c.getString(0)))
            {
                case 0: // blacklist
                    lst.setSummary(R.string.firewall_blacklist);
                    break;

                case 1: // whitelist
                    lst.setSummary(R.string.firewall_whitelist);
                    break;

                case 2: // block all
                    lst.setSummary(R.string.firewall_blockall);
                    break;

                case 3: // only contacts
                    lst.setSummary(R.string.firewall_onlycontacts);
                    break;

                default:
                    break;
            }

            c.close();
        }
        else
        {
            Log.d(TAG, "GetCurrentSettings this is double cards");
            ListPreference lst = (ListPreference) getPreferenceScreen().findPreference(
                    "firewall_call_mode");
            c = resolver.query(uri,
                    new String[] { "value" }, "_id = ?",
                    new String[] { "2" }, null);
            c.moveToFirst();
            lst.setValue(c.getString(0));
            switch (Integer.valueOf(c.getString(0)))
            {
                case 0: // blacklist
                    lst.setSummary(R.string.firewall_blacklist);
                    break;

                case 1: // whitelist
                    lst.setSummary(R.string.firewall_whitelist);
                    break;

                case 2: // block all
                    lst.setSummary(R.string.firewall_blockall);
                    break;

                case 3: // only contacts
                    lst.setSummary(R.string.firewall_onlycontacts);
                    break;

                case 4: // block c
                    lst.setSummary(blockcard1);
                    break;

                case 5: // block g
                    lst.setSummary(blockcard2);
                    break;

                default:
                    break;
            }

            c.close();
        }
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        GetCurrentSettings();
        super.onResume();
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
            String key) {
        // TODO Auto-generated method stub

        if (key.equals("firewall_call_mode")) {
            ListPreference lst = (ListPreference) getPreferenceScreen().findPreference(
                    "firewall_call_mode");
            ContentResolver resolver = getContentResolver();
            Uri uri = Uri.parse("content://com.android.firewall/setting/2");
            ContentValues values = new ContentValues();
            values.put("value", sharedPreferences.getString("firewall_call_mode", "0"));
            if (mIsSingleCard)
            {
                switch (Integer.valueOf(sharedPreferences.getString("firewall_call_mode", "0")))
                {
                    case 0: // blacklist
                        lst.setSummary(R.string.firewall_blacklist);
                        break;

                    case 1: // whitelist
                        lst.setSummary(R.string.firewall_whitelist);
                        break;

                    case 2: // block all
                        lst.setSummary(R.string.firewall_blockall);
                        break;

                    case 3: // only contacts
                        lst.setSummary(R.string.firewall_onlycontacts);
                        break;

                    default:
                        break;
                }
            }
            else
            {
                switch (Integer.valueOf(sharedPreferences.getString("firewall_call_mode", "0")))
                {
                    case 0: // blacklist
                        lst.setSummary(R.string.firewall_blacklist);
                        break;

                    case 1: // whitelist
                        lst.setSummary(R.string.firewall_whitelist);
                        break;

                    case 2: // block all
                        lst.setSummary(R.string.firewall_blockall);
                        break;

                    case 3: // only contacts
                        lst.setSummary(R.string.firewall_onlycontacts);
                        break;
                    case 4: // block c
                        lst.setSummary(blockcard1);
                        break;
                    case 5: // block g
                        lst.setSummary(blockcard2);
                        break;
                    default:
                        break;
                }
            }
            resolver.update(uri, values, null, null);

        }
        if (key.equals("firewall_sms_mode")) {
            ListPreference lst = (ListPreference) getPreferenceScreen().findPreference(
                    "firewall_sms_mode");
            ContentResolver resolver = getContentResolver();
            Uri uri = Uri.parse("content://com.android.firewall/setting/6");
            ContentValues values = new ContentValues();
            values.put("value", sharedPreferences.getString("firewall_sms_mode", "0"));
            switch (Integer.valueOf(sharedPreferences.getString("firewall_sms_mode", "0")))
            {
                case 0: // blacklist
                    lst.setSummary(R.string.firewall_blacklist);
                    break;

                case 1: // whitelist
                    lst.setSummary(R.string.firewall_whitelist);
                    break;

                case 2: // block all
                    lst.setSummary(R.string.firewall_blockall_sms);
                    break;

                case 3: // only contacts
                    lst.setSummary(R.string.firewall_onlycontacts_sms);
                    break;

                default:
                    break;
            }

            resolver.update(uri, values, null, null);
        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        final String key = preference.getKey();
        if (key.equals("firewall_call_switcher")) {
            CheckBoxPreference chk = (CheckBoxPreference) getPreferenceScreen().findPreference(
                    "firewall_call_switcher");
            ContentResolver resolver = getContentResolver();
            Uri uri = Uri.parse("content://com.android.firewall/setting/1");
            ContentValues values = new ContentValues();
            values.put("value", chk.isChecked() ? "1" : "0");
            resolver.update(uri, values, null, null);
        } else if (key.equals("firewall_call_block_c")) {
            CheckBoxPreference chk = (CheckBoxPreference) getPreferenceScreen().findPreference(
                    "firewall_call_block_c");
            ContentResolver resolver = getContentResolver();
            Uri uri = Uri.parse("content://com.android.firewall/setting/3");
            ContentValues values = new ContentValues();
            values.put("value", chk.isChecked() ? "1" : "0");
            resolver.update(uri, values, null, null);
        } else if (key.equals("firewall_call_block_g")) {
            CheckBoxPreference chk = (CheckBoxPreference) getPreferenceScreen().findPreference(
                    "firewall_call_block_g");
            ContentResolver resolver = getContentResolver();
            Uri uri = Uri.parse("content://com.android.firewall/setting/4");
            ContentValues values = new ContentValues();
            values.put("value", chk.isChecked() ? "1" : "0");
            resolver.update(uri, values, null, null);
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

}
