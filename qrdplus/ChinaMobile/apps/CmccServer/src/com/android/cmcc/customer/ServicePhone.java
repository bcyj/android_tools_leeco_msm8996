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
package com.android.cmcc.customer;

import com.android.cmcc.R;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.Toast;

public class ServicePhone extends Activity {
    private static final int MENU_ITEM_SAVE = 1;
    private static final int MENU_ITEM_CANCEL = 2;
    private String oldPhone;
    private Intent passintent;
    EditText phoneedit;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.service_phone_view);
        setTitle(R.string.service_phone);
        this.passintent = getIntent();
        EditText servicePhone = (EditText) findViewById(R.id.service_phone_text);
        this.phoneedit = servicePhone;
        if (this.passintent != null) {
            this.oldPhone = this.passintent.getExtras().getString(
                    "servicephone");
            this.phoneedit.setText(this.oldPhone);
        }
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(Menu.NONE, 1, Menu.NONE, R.string.menu_save);
        menu.add(Menu.NONE, 2, Menu.NONE, R.string.menu_cancel);
        return true;
    }

    public boolean onKeyDown(int paramInt, KeyEvent paramKeyEvent) {
        String str1 = this.phoneedit.getText().toString();

        if (paramInt == 4) {
            if (str1.length() > 0) {
                saveServicePhone();
            } else {
                finish();
            }
            return true;
        } else {
            return super.onKeyDown(paramInt, paramKeyEvent);
        }
    }

    private void saveServicePhone() {
        String str1 = this.phoneedit.getText().toString();
        if (str1.length() > 0) {
            if (str1.compareTo(this.oldPhone) != 0) {
                SharedPreferences.Editor localEditor = getSharedPreferences(
                        "managerinfo", 0).edit();
                localEditor.putString("servicephone", str1);
                localEditor.commit();
                Toast.makeText(this, R.string.save_notify, Toast.LENGTH_SHORT)
                        .show();
                Intent localIntent = new Intent();
                Uri localUri = Uri.parse("***saved service phone**");
                localIntent.setData(localUri);
                setResult(-1, localIntent);
                finish();
            } else {
                setResult(0);
                finish();
                return;
            }
        } else {
            Toast.makeText(this, R.string.servicephone_error, Toast.LENGTH_LONG)
                    .show();
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_ITEM_SAVE:
            saveServicePhone();
            break;
        case MENU_ITEM_CANCEL:
            if (this.phoneedit.getText().toString().length() > 0) {
                setResult(0);
                finish();
                break;
            } else {
                Toast.makeText(this, R.string.servicephone_error,
                        Toast.LENGTH_LONG).show();
            }
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onResume() {
        SharedPreferences sp = this.getSharedPreferences("managerinfo", 0);
        String str = sp.getString("servicephone", "");
        this.phoneedit.setText(str);
        super.onResume();
    }
}
