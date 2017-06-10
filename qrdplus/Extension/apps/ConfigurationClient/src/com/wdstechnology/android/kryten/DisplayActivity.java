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

import java.util.ArrayList;

import com.wdstechnology.android.kryten.persister.ApnPersister;
import com.wdstechnology.android.kryten.ConfigurationListItem;

import android.app.ActionBar;
import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.text.Html;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class DisplayActivity extends Activity {

    private byte[] mDocument;
    private String from;
    private String secString;
    private String macString;
    private String TAG = "DisplayActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        if (ProvisioningPushReceiver.DEBUG_TAG)
            Log.d(TAG, "onCreate");
        setContentView(R.layout.provisioning_display);
        setupActionBar();
        final long id = getIntent().getLongExtra("id", -1);
        if (ProvisioningPushReceiver.DEBUG_TAG)
            Log.d(TAG, "onCreate id  " + id);
        Cursor cur = getApplicationContext().getContentResolver().
                query(ConfigurationDatabaseProvider.CONTENT_URI,
                        null, ConfigurationDatabaseProvider.Columns._ID + " =" + id,
                        null, null);
        StringBuilder sb = new StringBuilder();
        if (cur != null) {
            cur.moveToFirst();
            mDocument = cur.getBlob(cur.getColumnIndex(ConfigurationDatabaseProvider.Columns.DOC));
            from = cur.getString(cur.getColumnIndex(ConfigurationDatabaseProvider.Columns.SIM));
            String name = cur.getString(cur
                    .getColumnIndex(ConfigurationDatabaseProvider.Columns.NAME));
            secString = cur
                    .getString(cur.getColumnIndex(ConfigurationDatabaseProvider.Columns.SEC));
            macString = cur
                    .getString(cur.getColumnIndex(ConfigurationDatabaseProvider.Columns.MAC));
            if (from.equals("GSM2"))
            {
                sb.append("<b>Application(SIM2):</b><br/>");
            }
            else
                sb.append("<b>Application(SIM1):</b><br/>");
            sb.append(name);

            ArrayList<String> appid = ConfigurationListItem.getApplicationName(cur);
            String apn = "";
            if (appid.contains(ConfigurationListItem.BROWSERTAG)) {
                sb.append("[Browser]<br/><br/>");
                apn = cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.APN));
                sb.append("<b>APN:</b><br/>   ");
                sb.append(apn + "<br/>");
            }
            if (appid.contains(ConfigurationListItem.MMSTAG)) {
                sb.append("\n[Mms]<br/><br/>");
                String mmsc = "";

                mmsc = cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.MMSC));
                sb.append("<b>MMSC:</b><br/>   ");
                sb.append(mmsc + "<br/>");
            }
            if(appid.contains(ConfigurationListItem.POP3TAG)){
                sb.append("\n[EMAIL]<br/><br/>");
                String inboundServerUri = "";
                String outboundServerUri = "";
                String outboundPortNbr = "";
                String inboundPortNbr = "";
                String inboundService = "";
                String outboundService = "";

                inboundServerUri = cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.INBOUND_SERVER_URI));
                outboundServerUri = cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.OUTBOUND_SERVER_URI));
                inboundPortNbr = cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.INBOUND_PORTNBR));
                outboundPortNbr = cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.OUTBOUND_PORTNBR));
                if(cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.INBOUND_SERVICE)) != null)
                    inboundService = cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.INBOUND_SERVICE));
                else
                    inboundService = "None";
                if(cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.OUTBOUND_SERVICE)) != null)
                    outboundService = cur.getString(cur
                        .getColumnIndex(ConfigurationDatabaseProvider.Columns.OUTBOUND_SERVICE));
                else
                    outboundService = "None";

                sb.append("<b>INBOUND SERVER ADDRESS:</b><br/>   ");
                sb.append(inboundServerUri + "<br/>");
                sb.append("\n<b>INBOUND PORT:</b><br/>   ");
                sb.append(inboundPortNbr + "<br/>");
                sb.append("\n<b>INBOUND SECURITY TYPE:</b><br/>   ");
                sb.append(inboundService + "<br/>");
                sb.append("\n<b>OUTBOUND SERVER ADDRESS:</b><br/>   ");
                sb.append(outboundServerUri + "<br/>");
                sb.append("\n<b>OUTBOUND PORT:</b><br/>   ");
                sb.append(outboundPortNbr + "<br/>");
                sb.append("\n<b>OUTBOUND SECURITY TYPE:</b><br/>   ");
                sb.append(outboundService + "<br/>");

            }

        }
        if (ProvisioningPushReceiver.DEBUG_TAG)
            Log.d(TAG, "onCreate sb  " + sb.toString());
        TextView tv = (TextView) findViewById(R.id.message);
        tv.setText(Html.fromHtml(sb.toString()));
        ((Button) findViewById(R.id.accept)).setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                if (ProvisioningPushReceiver.DEBUG_TAG)
                    Log.d(TAG, "onClick ok");
                Intent intent = new Intent();
                if (secString != null) {
                    intent.putExtra("com.wdstechnology.android.kryten.SEC", secString);
                }
                if (macString != null) {
                    intent.putExtra("com.wdstechnology.android.kryten.MAC", macString);
                }
                intent.putExtra("com.wdstechnology.android.kryten.provisioning-data", mDocument);
                intent.putExtra("from", from);
                intent.setClass(getApplicationContext(), ProvisioningValidation.class);
                intent.putExtra("id", id);
                startActivity(intent);
                finish();
            }
        });
        ((Button) findViewById(R.id.decline)).setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                if (ProvisioningPushReceiver.DEBUG_TAG)
                    Log.d(TAG, "onClick cancel");
                DisplayActivity.this.finish();
            }
        });

    }

    private void setupActionBar() {
        ActionBar actionBar = getActionBar();
        actionBar.setHomeButtonEnabled(true);
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setBackgroundDrawable(new ColorDrawable(getResources()
                .getColor(R.color.holo_blue)));
        actionBar.setTitle(getString(R.string.app_label));
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

}
