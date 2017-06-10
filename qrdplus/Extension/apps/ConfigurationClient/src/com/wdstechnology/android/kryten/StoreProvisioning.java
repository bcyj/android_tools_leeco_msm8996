/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
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

package com.wdstechnology.android.kryten;

import android.app.ActionBar;
import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import org.xmlpull.v1.XmlPullParserException;

import com.wdstechnology.android.kryten.persister.ApnPersister;
import com.wdstechnology.android.kryten.sax.WapProvisioningDocContentHandler;
import com.wdstechnology.android.kryten.wbxml.WbxmlSaxParser;

import java.io.ByteArrayInputStream;
import java.io.IOException;

public class StoreProvisioning extends Activity {

    private WapProvisioningDocContentHandler mProvisioningDocument;
    private String from;
    private long id;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // requestWindowFeature(Window.FEATURE_NO_TITLE);
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setDisplayOptions(ActionBar.DISPLAY_HOME_AS_UP,
                    ActionBar.DISPLAY_HOME_AS_UP);
            actionBar.setCustomView(R.layout.banner);
            actionBar.setTitle(getString(R.string.provisioning_received_title));
        }
        setContentView(R.layout.check_provisioning);
    }

    @Override
    protected void onStart() {
        super.onStart();

        byte[] wbxmlDocument = getIntent().getByteArrayExtra(
                "com.wdstechnology.android.kryten.provisioning-data");
        from = getIntent().getStringExtra("from");
        id = getIntent().getLongExtra("id", -1);

        mProvisioningDocument = new WapProvisioningDocContentHandler();
        WbxmlSaxParser parser = new WbxmlSaxParser();
        try {
            parser.parse(new ByteArrayInputStream(wbxmlDocument),
                    mProvisioningDocument);

            if (mProvisioningDocument.isDocumentWellFormed()) {
                if (mProvisioningDocument.isDocumentValid()) {
                    ((TextView) findViewById(R.id.check_prov_name_value))
                            .setText(mProvisioningDocument.getName());

                } else {
                    ProvisioningFailed
                            .fail(this, R.string.provisioning_failure);
                    finish();
                    return;
                }
            }

        } catch (XmlPullParserException e) {
            ProvisioningFailed.fail(this, R.string.provisioning_failure);
            finish();
            return;
        } catch (IOException e) {
            ProvisioningFailed.fail(this, R.string.provisioning_failure);
            finish();
            return;
        }
        ApnPersister persister = new ApnPersister(getApplicationContext());
        persister.save(mProvisioningDocument, from);
        ContentValues cv = new ContentValues();
        cv.put(ConfigurationDatabaseProvider.Columns.INSTALL, true);
        getApplicationContext().getContentResolver().update(
                ConfigurationDatabaseProvider.CONTENT_URI, cv,
                ConfigurationDatabaseProvider.Columns._ID + " = " + id, null);
        Toast completeMessage = Toast.makeText(StoreProvisioning.this,
                R.string.phone_now_configured, Toast.LENGTH_LONG);
        completeMessage.setGravity(Gravity.CENTER, 0, 0);
        completeMessage.show();
        StoreProvisioning.this.finish();
    }

    public static void provision(Context context, byte[] document, String from,
            long id) {
        Intent storeProvisioningIntent = new Intent(context,
                StoreProvisioning.class);
        storeProvisioningIntent.putExtra(
                "com.wdstechnology.android.kryten.provisioning-data", document);
        storeProvisioningIntent.putExtra("from", from);
        storeProvisioningIntent.putExtra("id", id);
        context.startActivity(storeProvisioningIntent);
    }

}
