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

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceGroup;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.preference.PreferenceScreen;
import android.widget.Toast;

public class RestoreActivity extends PreferenceActivity {

    private static final String KEY_APP = "restoreApp";
    private static final String KEY_DATA = "restoreData";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.restore);

        Preference restoreApp = (Preference) findPreference(KEY_APP);
        Preference restoreData = (Preference) findPreference(KEY_DATA);

    }

    @Override
    protected void onResume() {
        super.onResume();

    }

    @Override
    public void onPause() {
        super.onPause();
    }

    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        /*if (BackupUtils.isTfCardExist()) {
            return super.onPreferenceTreeClick(preferenceScreen, preference);// modify
        } else {
            Toast.makeText(this, R.string.sdcard_not_find,
                    Toast.LENGTH_SHORT).show();
            return true;
        }*/
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

}
