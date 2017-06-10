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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import org.xmlpull.v1.XmlPullParserException;

import com.wdstechnology.android.kryten.R;
import com.wdstechnology.android.kryten.sax.WapProvisioningDocContentHandler;
import com.wdstechnology.android.kryten.wbxml.WbxmlSaxParser;
import com.wdstechnology.android.kryten.utils.Constants;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.text.format.DateFormat;
import android.util.Log;
import android.widget.TextView;
import android.net.ConnectivityManager;
import android.os.SystemProperties;
import android.provider.Settings;

public class ProvisioningPushReceiver extends BroadcastReceiver {

    private WapProvisioningDocContentHandler mProvisioningDocument;
    private String TAG = "ProvisioningPushReceiver";
    public static boolean DEBUG_TAG = false;
    public static final String OMACPSHAREDPREF= "key_mcc_mnc";

    @SuppressWarnings("unchecked")
    @Override
    public void onReceive(Context context, Intent intent) {

        if (!(context.getResources()
                .getBoolean(R.bool.enableProvisioningWappush))
                && (!(SystemProperties.getBoolean(
                        "persist.omacp.enable", false)))) {
            Log.i(TAG, "OMA client provisioning is disabled");
            return;
        }

        byte[] document = intent.getByteArrayExtra("data");
        String sec = null;
        String mac = null;
        String from = null;
        int mSubscription;
        boolean result = false;
        SharedPreferences sharedPrefs = context.getSharedPreferences(OMACPSHAREDPREF,
                Context.MODE_PRIVATE);

        if (DEBUG_TAG) {
            Log.d(TAG, "onReceive Called ");
        }
        mSubscription = intent.getExtras().getInt("subscription");

        if (intent.getExtras() != null) {
            HashMap<String, String> provisioningParameters = (HashMap<String, String>) intent
                    .getExtras().get("contentTypeParameters");

            if (provisioningParameters != null) {
                sec = provisioningParameters.get("SEC");
                mac = provisioningParameters.get("MAC");
            }
            from = (String) intent.getExtras().get("from");
            if (DEBUG_TAG) {
                Log.d(TAG, "SEC  " + sec + " MAC " + mac + " from   " + from);
            }
        }

        result = parseXml(document, context, from, sec, mac, mSubscription);
		Log.d(TAG, "after parseXml... result"+result);
        if (result) {
            saveParamas(document, context, from, sec, mac, mSubscription,
                    sharedPrefs);
            ProvisioningNotification.createNotification(context, sec, mac,
                    document, from);
        }

    }

    public boolean parseXml(byte[] wbxmlDocument, Context mContext,
            String from, String sec, String mac, long mSubscription) {

        mProvisioningDocument = new WapProvisioningDocContentHandler();

        WbxmlSaxParser parser = new WbxmlSaxParser();
        try {
            parser.parse(new ByteArrayInputStream(wbxmlDocument),
                    mProvisioningDocument);
        } catch (XmlPullParserException e) {
            Log.d(TAG, "XmlPullParserException " + e);
            return false;
        } catch (IOException e) {
            Log.d(TAG, "IOException " + e);
            return false;
        }
        return true;
    }

    public void saveParamas(byte[] wbxmlDocument, Context mContext,
            String from, String sec, String mac, long mSubscription,
            SharedPreferences sharedPrefs) {
        Date date = new Date();
        Map<String, String> parameters = mProvisioningDocument.getParameters();

        ContentValues contentValues = new ContentValues();
        contentValues.put(ConfigurationDatabaseProvider.Columns.NAME,
                parameters.get("name"));
        contentValues.put(ConfigurationDatabaseProvider.Columns.APN,
                parameters.get("apn"));
        contentValues.put(ConfigurationDatabaseProvider.Columns.USER,
                parameters.get("user"));
        contentValues.put(ConfigurationDatabaseProvider.Columns.PASSWORD,
                parameters.get("password"));
        contentValues.put(ConfigurationDatabaseProvider.Columns.DATE,
                date.getTime());
        contentValues.put(ConfigurationDatabaseProvider.Columns.SIM, from);
        contentValues.put(ConfigurationDatabaseProvider.Columns.MAC, mac);
        contentValues.put(ConfigurationDatabaseProvider.Columns.SEC, sec);
        contentValues.put(ConfigurationDatabaseProvider.Columns.DOC, wbxmlDocument);
        contentValues.put(ConfigurationDatabaseProvider.Columns.INSTALL, false);
        contentValues.put(ConfigurationDatabaseProvider.Columns.AUTHTYPE,
                parameters.get("authtype"));
        contentValues.put(ConfigurationDatabaseProvider.Columns.EMAIL_DISPLAYNAME,
                parameters.get("email_displayname"));
        contentValues.put(ConfigurationDatabaseProvider.Columns.EMAIL_ID,
                parameters.get("email_id"));
        contentValues.put(ConfigurationDatabaseProvider.Columns.EMAIL_PASSWORD,
                parameters.get("email_password"));

        String appIdBrowser = parameters.get("appIdBrowser");
        String appIdMms = parameters.get("appIdMms");
        String appIdPop3 = parameters.get("appIdPop3");
        String appIdImap4 = parameters.get("appIdImap4");
        String appIdSmtp = parameters.get("appIdSmtp");

        if(appIdBrowser != null){
            if (DEBUG_TAG)
            Log.i(TAG,"BROWSER APN");
            contentValues.put(ConfigurationDatabaseProvider.Columns.TYPE,
                    "default");
            contentValues.put(ConfigurationDatabaseProvider.Columns.PORT,
                    parameters.get("mmsport"));
            contentValues.put(ConfigurationDatabaseProvider.Columns.PROXY,
                    parameters.get("mmsproxy"));
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.APPID_BROWSER, appIdBrowser);
            appIdBrowser = null;
        }
        if(appIdMms != null){
            if (DEBUG_TAG)
                Log.i(TAG,"MMS APN");
            contentValues.put(ConfigurationDatabaseProvider.Columns.TYPE,
                    "mms");
            contentValues.put(ConfigurationDatabaseProvider.Columns.PORT,
                    parameters.get("mmsport"));
            contentValues.put(ConfigurationDatabaseProvider.Columns.PROXY,
                    parameters.get("mmsproxy"));
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.MMSPROXY,
                    parameters.get("mmsproxy"));
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.MMSPORT,
                    parameters.get("mmsport"));
            contentValues.put(ConfigurationDatabaseProvider.Columns.MMSC,
                    parameters.get("mmsc"));
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.APPID_MMS, appIdMms);
            appIdMms = null;
        }
        if(appIdPop3 != null){
            if (DEBUG_TAG)
                Log.i(TAG,"EMAIL Settings for POP3");
            contentValues
                    .put(ConfigurationDatabaseProvider.Columns.INBOUND_SERVER_URI,
                            parameters.get("inbound_server_uri"));
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.INBOUND_PORTNBR, appIdPop3);
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.APPID_POP3, appIdPop3);
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.INBOUND_SERVICE,
                    parameters.get("inbound_service"));

            appIdPop3 = null;
        }
        if(appIdImap4 != null){
            if (DEBUG_TAG)
                Log.i(TAG,"EMAIL Settings for IMAP4");
            contentValues
                    .put(ConfigurationDatabaseProvider.Columns.INBOUND_SERVER_URI,
                            parameters.get("inbound_server_uri"));
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.INBOUND_PORTNBR, appIdImap4);

            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.APPID_IMAP4, appIdImap4);
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.INBOUND_SERVICE,
                    parameters.get("inbound_service"));

            appIdImap4 = null;
        }
        if(appIdSmtp != null){
            contentValues
                    .put(ConfigurationDatabaseProvider.Columns.OUTBOUND_SERVER_URI,
                            parameters.get("outbound_server_uri"));
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.OUTBOUND_PORTNBR, appIdSmtp);
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.APPID_SMTP, appIdSmtp);
            contentValues.put(
                    ConfigurationDatabaseProvider.Columns.OUTBOUND_SERVICE,
                    parameters.get("outbound_service"));

            appIdSmtp = null;
        }

        String mcc = "" ;
        String mnc = "" ;

        String device = SystemProperties.get("gsm.sim.operator.numeric");
        int curId = -1;
        if (mSubscription == 0) {

            curId = Settings.Secure.getInt(mContext.getContentResolver(),
                    Constants.MOBILE_DATA_SIM,
                    Constants.MOBILE_DATA_NETWORK_SLOT_A);
        } else {
            curId = Settings.Secure.getInt(mContext.getContentResolver(),
                    Constants.MOBILE_DATA_SIM,
                    Constants.MOBILE_DATA_NETWORK_SLOT_B);
        }

        String sim1 = "";
        String sim2 = "";
        if (device.contains(",")) {
            String sim[] = device.split(",");
            sim1 = sim[0];
            sim2 = sim[1];

        } if(curId == 0){
            sim1 = device;
        }else {
            sim2 = device;
        }

        if (curId == 0) {

            contentValues.put(ConfigurationDatabaseProvider.Columns.SIM, "GSM");
            if ((sim1 != null && !sim1.equals(""))
                    && ((mcc == null) || mcc.equals("")))
                mcc = sim1.substring(0, 3);

            if ((sim1 != null && !sim1.equals(""))
                    && ((mnc == null) || mnc.equals("")))
                mnc = sim1.substring(3);

        } else {

            contentValues
                    .put(ConfigurationDatabaseProvider.Columns.SIM, "GSM2");

            if ((sim2 != null && !sim2.equals(""))
                    && ((mcc == null) || mcc.equals("")))
                mcc = sim2.substring(0, 3);

            if ((sim2 != null && !sim2.equals(""))
                    && ((mnc == null) || mnc.equals("")))
                mnc = sim2.substring(3);

        }

        contentValues.put(ConfigurationDatabaseProvider.Columns.MCC, mcc);
        contentValues.put(ConfigurationDatabaseProvider.Columns.MNC, mnc);
        contentValues.put(ConfigurationDatabaseProvider.Columns.NUMERIC, mcc
                + mnc);
        Editor editor = sharedPrefs.edit();
        editor.putString("mcc", mcc);
        editor.putString("mnc", mnc);
        editor.commit();

        if (DEBUG_TAG) {
            Log.d(TAG,
                    "NAME  " + parameters.get("name") + " APN "
                            + parameters.get("apn"));
            Log.d(TAG, "USER  " + parameters.get("user") + " PASSWORD "
                    + parameters.get("password"));
            Log.d(TAG, "APPID  " + parameters.get("appid") + " PORT "
                    + parameters.get("mmsport"));
            Log.d(TAG, "PROXY  " + parameters.get("mmsproxy") + " MMSC "
                    + parameters.get("mmsc"));
            Log.d(TAG, "AUTHTYPE  " + parameters.get("authtype"));

            if(appIdPop3 != null){
            Log.d(TAG,
                    "POP3 : SERVER URI  "
                            + parameters.get("inbound_server_uri"));
            Log.d(TAG, "POP3 : PORT  " + appIdPop3);
            Log.d(TAG, "POP3 : SERVICE  " + parameters.get("inbound_service"));
            }

            if(appIdImap4 != null){
            Log.d(TAG,
                    "IMAP4 : SERVER URI  "
                            + parameters.get("inbound_server_uri"));
            Log.d(TAG, "IMAP4 : appIdImap4  " + appIdImap4);
            Log.d(TAG, "IMAP4 : SERVICE  " + parameters.get("inbound_service"));
            }
            Log.d(TAG,
                    "SMTP : SERVER URI  "
                            + parameters.get("outbound_server_uri"));
            Log.d(TAG, "SMTP : appIdSmtp  " + appIdSmtp);
            Log.d(TAG, "SMTP : SERVICE  " + parameters.get("outbound_service"));
        }

        if (mProvisioningDocument.isDocumentWellFormed()
                && mProvisioningDocument.isDocumentValid()) {
            if (DEBUG_TAG) {
                Log.d(TAG, "document is valid");
            }
            mContext.getContentResolver().insert(
                    ConfigurationDatabaseProvider.CONTENT_URI, contentValues);
            return ;
        } else {
            Log.d(TAG, "app_id is invalid or apn is null");
            return ;
        }
    }
}
