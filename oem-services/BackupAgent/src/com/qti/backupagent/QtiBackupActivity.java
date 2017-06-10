/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.backupagent;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.Menu;
import android.widget.CompoundButton;
import android.widget.Switch;

public class QtiBackupActivity extends Activity implements
CompoundButton.OnCheckedChangeListener {
    private final static String TAG = "QtiBackupActivity";
    SharedPreferences settings;
    SharedPreferences.Editor editor;
    static public final String PREFS_NAME = "com.qti.backupagent";
    static public final String CONTACTS = "CONTACTS";

    static public final String CONTACTS_FILE = "contacts.vcf";

    @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            setContentView(R.layout.activity_backup);

            settings = getApplicationContext().getSharedPreferences(PREFS_NAME, 0);
            editor = settings.edit();

            Switch contactsSwitch = (Switch) findViewById(R.id.contacts_id);
            switchConfig(contactsSwitch);
        }

    String getSettingsTokenById(int id) {
        switch(id) {
            case R.id.contacts_id:
                return CONTACTS;
            default:
                return null;
        }
    }

    void switchConfig(Switch s) {
        if (s != null) {
            String token = getSettingsTokenById(s.getId());
            if(token != null) {
                boolean isChecked = settings.getBoolean(token, false);
                s.setChecked(isChecked);
            }
            s.setOnCheckedChangeListener(this);
        }
    }

    void saveSettingsById(int id, boolean value) {
        String token = getSettingsTokenById(id);
        if(token != null) {
            editor.putBoolean(token, value);
            editor.commit();
        }

    }

    @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            switch (buttonView.getId()) {
                case R.id.contacts_id:
                    saveSettingsById(R.id.contacts_id, isChecked);
                    if(!isChecked)
                        deleteFile(CONTACTS_FILE);
                    break;
            }
        }

}
