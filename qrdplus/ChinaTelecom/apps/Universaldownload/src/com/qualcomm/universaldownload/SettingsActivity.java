/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import android.content.Intent;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

public class SettingsActivity extends PreferenceActivity {
    private static final String PREF_MANULLY_UPDATE = "pref_key_update_manually";
    private static final String PREF_AUTO_UPDATE = "pref_key_update_auto";
    private static final String PREF_SERVER_ADDR = "pref_key_server_addr";
    private Preference mUserupdate;
    private CheckBoxPreference mAutoUpdate;
    private EditTextPreference mServerAddr;
    private DownloadSharePreference mPref;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.universal_download_preference);
        mPref = DownloadSharePreference.Instance(this);
        mUserupdate = findPreference(PREF_MANULLY_UPDATE);
        mAutoUpdate = (CheckBoxPreference) findPreference(PREF_AUTO_UPDATE);
        mServerAddr = (EditTextPreference) findPreference(PREF_SERVER_ADDR);
        mAutoUpdate.setChecked(mPref.getAutoUpdate());
        mServerAddr.setText(mPref.getServerAddr());
        mServerAddr.setSummary(mPref.getServerAddr());
    }

    @Override
    protected void onResume() {
        super.onResume();
        mAutoUpdate.setOnPreferenceChangeListener(mOnPreferenceChangedListener);
        mServerAddr.setOnPreferenceChangeListener(mOnPreferenceChangedListener);
        mUserupdate.setEnabled(NetManager.isCTCardInsert(this));
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen screen, Preference preference) {
        if(preference == mUserupdate) {
            Intent start = new Intent();
            start.setClass(this, ManualUpdateActivity.class);
            startActivity(start);
        }
        return super.onPreferenceTreeClick(screen, preference);
    }

    private Preference.OnPreferenceChangeListener mOnPreferenceChangedListener =
            new Preference.OnPreferenceChangeListener() {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newvalue) {
            if(preference == mAutoUpdate) {
                Boolean checked = (Boolean) newvalue;
                mPref.setAutoUpdate(checked);
                mPref.save();
                return true;
            } else if(preference == mServerAddr) {
                String address = (String) newvalue;
                mPref.setServerAddress(address);
                mPref.save();
                mServerAddr.setSummary(address);
                return true;
            }
            return false;
        }
    };

}
