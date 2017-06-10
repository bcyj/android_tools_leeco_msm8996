/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.android.nfc;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.content.Context;
import com.android.nfc.R;

public class ConfirmNfcEnableActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final Context appContext =   getApplicationContext();
        AlertDialog.Builder builder = new AlertDialog.Builder(this, AlertDialog.THEME_HOLO_DARK);
        Intent launchIntent = getIntent();
        final String pkgName = launchIntent.getStringExtra("com.android.nfc.action.EXTRA_PKG");
        Resources res = getResources();
        builder.setMessage("Are you sure you want to enable NFC?")
               .setCancelable(false)
               .setPositiveButton(res.getString(R.string.pair_yes),
                       new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialog, int id) {
                       Intent allowIntent = new Intent("com.android.nfc.action.ALLOW_NFC_ENABLE");
                       allowIntent.putExtra("com.android.nfc.action.EXTRA_PKG", pkgName);
                       appContext.sendBroadcast(allowIntent);
                       ConfirmNfcEnableActivity.this.finish();
                   }
               })
               .setNegativeButton(res.getString(R.string.pair_no),
                       new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialog, int id) {
                       Intent denyIntent = new Intent("com.android.nfc.action.DENY_NFC_ENABLE");
                       denyIntent.putExtra("com.android.nfc.action.EXTRA_PKG", pkgName);
                       sendBroadcast(denyIntent);
                       ConfirmNfcEnableActivity.this.finish();
                   }
               });
        AlertDialog alert = builder.create();
        alert.show();
    }
}

