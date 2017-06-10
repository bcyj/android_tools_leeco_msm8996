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

package com.wdstechnology.android.kryten.persister;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.util.Log;
import android.os.SystemProperties;
import android.provider.Settings;

import com.wdstechnology.android.kryten.ConfigurationDatabaseProvider;
import com.wdstechnology.android.kryten.ConfigurationListItem;
import com.wdstechnology.android.kryten.ProvisioningPushReceiver;
import com.wdstechnology.android.kryten.sax.WapProvisioningDocContentHandler;
import com.wdstechnology.android.kryten.utils.Constants;

import java.util.Map;

public class ApnPersister {

    private Context mContext;
    String TAG = "ApnPersister";
    //Email
    String inboundService = "";
    String outboundService = "";
    String inboundPort = "";
    String outboundPort = "";
    String inboundSecType = "";
    String outboundSecType = "";
    String incomingServer = "";
    String outgoingServer = "";

    private static final String EXTRA_CREATE_ACCOUNT_EMAIL = "EMAIL";
    private static final String EXTRA_CREATE_ACCOUNT_USER = "USER";
    private static final String EXTRA_CREATE_ACCOUNT_PASSWORD = "PASSWORD";
    private static final String EXTRA_CREATE_ACCOUNT_INCOMING = "INCOMING";
    private static final String EXTRA_CREATE_ACCOUNT_OUTGOING = "OUTGOING";

    public ApnPersister(Context context) {
        mContext = context;
    }

    public void save(WapProvisioningDocContentHandler provisioningDocument,
            String from) {

        Map<String, String> parameters = provisioningDocument.getParameters();
        String type = "";
        ContentValues contentValues = new ContentValues();

        contentValues.put("name", parameters.get("name"));
        contentValues.put("apn", parameters.get("apn"));
        contentValues.put("user", parameters.get("user"));
        contentValues.put("password", parameters.get("password"));

        Intent i = new Intent("com.android.email.FORCE_CREATE_ACCOUNT");//Email class
        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

		if (ConfigurationListItem.BROWSERTAG.equals(parameters.get("appIdBrowser"))) {
			contentValues.put("type", "default");
			contentValues.put("port", parameters.get("mmsport"));
			type = "default";
			insertIntoDB(parameters, contentValues,from,type);
		}
		if (ConfigurationListItem.MMSTAG.equals(parameters.get("appIdMms"))) {
			contentValues.put("type", "mms");
			contentValues.put("mmsproxy", parameters.get("mmsproxy"));
			contentValues.put("mmsport", parameters.get("mmsport"));
			contentValues.put("mmsc", parameters.get("mmsc"));
			type = "mms";
			insertIntoDB(parameters, contentValues,from,type);
		}
        if (ConfigurationListItem.POP3TAG.equals(parameters.get("appIdPop3"))) {

            contentValues.put("inbound_server_uri", parameters.get("inbound_server_uri"));
            contentValues.put("appIdPop3", parameters.get("appIdPop3"));
            contentValues.put("inbound_service", parameters.get("inbound_service"));

            if(parameters.get("inbound_service") == null || parameters.get("inbound_service").equals("")){
                inboundSecType = "pop3";
                inboundPort = ConfigurationListItem.POP3TAG;
            }else if(parameters.get("inbound_service").equalsIgnoreCase("STARTTLS")){
                inboundSecType = "pop3+tls+";
                inboundPort = ConfigurationListItem.POP3TAG;
            }else{
                inboundSecType = "pop3+ssl+";
                inboundPort = parameters.get("inbound_service");
            }

            Log.d(TAG,
                    "POP3 : SERVER URI  "
                            + parameters.get("inbound_server_uri"));
            Log.d(TAG, "POP3 : Inbound Port  " + inboundPort);
            Log.d(TAG, "POP3 : parameters.get(inbound_service):  " + parameters.get("inbound_service"));
            Log.d(TAG, "POP3 : inboundSecType  " + inboundSecType);

        }
        if (ConfigurationListItem.IMAPTAG.equals(parameters.get("appIdImap4"))) {

            contentValues.put("inbound_server_uri", parameters.get("inbound_server_uri"));
            contentValues.put("appIdImap4", parameters.get("appIdImap4"));
            contentValues.put("inbound_service", parameters.get("inbound_service"));

            if(parameters.get("inbound_service") == null || parameters.get("inbound_service").equals("")){
                inboundSecType = "imap";
                inboundPort = ConfigurationListItem.IMAPTAG;
            }else if(parameters.get("inbound_service").equalsIgnoreCase("STARTTLS")){
                inboundSecType = "imap+tls+";
                inboundPort = ConfigurationListItem.IMAPTAG;
            }else{
                inboundSecType = "imap+ssl+";
                inboundPort = parameters.get("inbound_service");
            }

            Log.d(TAG,
                    "IMAP4 : SERVER URI  "
                            + parameters.get("inbound_server_uri"));
            Log.d(TAG, "IMAP4 : Inbound Port  " + inboundPort);
            Log.d(TAG, "IMAP4 : Inbound service  " + parameters.get("inbound_service"));
            Log.d(TAG, "IMAP4 : inboundSecType  " + inboundSecType);

        }
        if (ConfigurationListItem.SMTPTAG.equals(parameters.get("appIdSmtp"))) {
            contentValues.put("outbound_server_uri", parameters.get("outbound_server_uri"));
            contentValues.put("appIdSmtp", parameters.get("appIdSmtp"));
            contentValues.put("outbound_service", parameters.get("outbound_service"));

            if(parameters.get("outbound_service") == null || parameters.get("outbound_service").equals("")){
                outboundSecType = "smtp";
                outboundPort = ConfigurationListItem.SMTPTAG;
            }else if(parameters.get("outbound_service").equalsIgnoreCase("STARTTLS")){
                outboundSecType = "smtp+tls+";
                outboundPort = ConfigurationListItem.SMTPTAG;
            }else{
                outboundSecType = "smtp+ssl+";
                outboundPort = parameters.get("outbound_service");
            }

            Log.d(TAG,
                    "SMTP : SERVER URI  "
                            + parameters.get("outbound_server_uri"));
            Log.d(TAG, "SMTP : Outbound Port  " + outboundPort);
            Log.d(TAG, "SMTP : Outbound service  " + parameters.get("outbound_service"));
            Log.d(TAG, "SMTP : outboundSecType  " + outboundSecType);

        }
        if (parameters.get("appIdPop3") != null
                || parameters.get("appIdImap4") != null) {

            i.putExtra(EXTRA_CREATE_ACCOUNT_EMAIL, parameters.get("email_id"));
            i.putExtra(EXTRA_CREATE_ACCOUNT_USER,
                    parameters.get("email_displayname"));

            String emailPassword = parameters.get("email_password");
            if (emailPassword.contains("@"))
                emailPassword = emailPassword.replace("@", "%40");// Email app expects @ as %40

            String emailId = parameters.get("email_id");
            emailId = emailId.replace("@", "%40");
            incomingServer = inboundSecType + "://" + emailId + ":"
                    + emailPassword + "@"
                    + parameters.get("inbound_server_uri") + ":" + inboundPort;
            outgoingServer = outboundSecType + "://" + emailId + ":"
                    + emailPassword + "@"
                    + parameters.get("outbound_server_uri") + ":"
                    + outboundPort;

            i.putExtra(EXTRA_CREATE_ACCOUNT_INCOMING, incomingServer);
            i.putExtra(EXTRA_CREATE_ACCOUNT_OUTGOING, outgoingServer);

            if (ProvisioningPushReceiver.DEBUG_TAG) {
                Log.d(TAG, "Sending following information to Email app");
                Log.d(TAG,
                        "Email :"
                                + i.getStringExtra(EXTRA_CREATE_ACCOUNT_EMAIL));
                Log.d(TAG,
                        "User name :"
                                + i.getStringExtra(EXTRA_CREATE_ACCOUNT_USER));
                Log.d(TAG,
                        "Incoming :"
                                + i.getStringExtra(EXTRA_CREATE_ACCOUNT_INCOMING));
                Log.d(TAG,
                        "Outgoing :"
                                + i.getStringExtra(EXTRA_CREATE_ACCOUNT_OUTGOING));
            }
            mContext.startActivity(i); // Email intent
        }

    }

    public void insertIntoDB(Map<String, String> parameters,
            ContentValues contentValues, String from, String type) {
        if(ProvisioningPushReceiver.DEBUG_TAG)
        Log.d(TAG,"insertIntoDB.....type="+type);

        SharedPreferences sharedpreferences = mContext.getSharedPreferences(
                ProvisioningPushReceiver.OMACPSHAREDPREF, Context.MODE_PRIVATE);
        String mcc = sharedpreferences.getString("mcc", null);
        String mnc = sharedpreferences.getString("mnc", null);

        contentValues.put("mcc", mcc);
        contentValues.put("mnc", mnc);
        String strFilter = "mcc=\"" + mcc + "\" and mnc=\"" + mnc
                + "\" and type=\"" + type + "\"";
        contentValues.put("numeric", mcc + mnc);
        String authType = parameters.get("authtype");
        if (authType != null && authType.equals("PAP"))
            contentValues.put("authtype", 1);
        else if (authType != null && authType.equals("CHAP"))
            contentValues.put("authtype", 2);
        else
            contentValues.put("authtype", -1);

        if(ProvisioningPushReceiver.DEBUG_TAG){
            Log.d(TAG,"mcc = "+mcc);
            Log.d(TAG,"mnc = "+mnc);
            Log.d(TAG,"mmsproxy = "+parameters.get("mmsproxy"));
            Log.d(TAG,"mmsport = "+parameters.get("mmsport"));
            Log.d(TAG,"mmsc = "+parameters.get("mmsc"));
            if(parameters.get("appIdPop3") != null){
                Log.d(TAG,
                        "POP3 : SERVER URI  "
                                + parameters.get("inbound_server_uri"));
                Log.d(TAG, "POP3 : PORT  " + parameters.get("appIdPop3"));
                Log.d(TAG, "POP3 : SERVICE  " + parameters.get("inbound_service"));
            }
            if(parameters.get("appIdImap4") != null){
                Log.d(TAG,
                        "IMAP4 : SERVER URI  "
                                + parameters.get("inbound_server_uri"));
                Log.d(TAG, "IMAP4 : PORTNBR  " + parameters.get("appIdImap4"));
                Log.d(TAG, "IMAP4 : SERVICE  " + parameters.get("inbound_service"));

            }
            if(parameters.get("appIdSmtp") != null){
            Log.d(TAG,
                    "SMTP : SERVER URI  "
                            + parameters.get("outbound_server_uri"));
            Log.d(TAG, "SMTP : PORTNBR  " + parameters.get("appIdSmtp"));
            Log.d(TAG, "SMTP : SERVICE  " + parameters.get("outbound_service"));
            }
        }

        Uri uri;
        uri = Uri.parse("content://telephony/carriers");
        int result = mContext.getContentResolver().update(uri, contentValues,
                strFilter, null);
        if (result < 1)
            mContext.getContentResolver().insert(uri, contentValues);

    }

}
