/**
* Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
* Not a Contribution. Apache license notifications and license are retained
* for attribution purposes only.
*
* Copyright 2006, The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

package com.qualcomm.qti.modemtestmode;

import android.content.Context;
import android.os.Environment;
import android.util.Log;
import android.net.Uri;
import android.content.res.XmlResourceParser;
import android.database.SQLException;
import android.provider.Telephony;
import android.content.ContentValues;
import android.util.Xml;
import android.content.res.Resources;
import android.text.TextUtils;

import java.io.IOException;
import java.io.File;
import java.io.FileReader;
import java.io.FileNotFoundException;

import com.android.internal.util.XmlUtils;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;


public class ApnHandlerHelper {
    private final String TAG = "ApnHandlerHelper";
    private final String PARTNER_APNS_PATH = "etc/labtest-apns-conf.xml";
    private final Uri CARRIERS_URL = Uri.parse("content://telephony/carriers");
    private final Uri RESTORE_URL = Uri.parse("content://telephony/carriers/restore");
    private final String VISIT_AREA = "visit_area";
    private final String LAB_TEST_TAG = "labtest";

    private Context mContext;
    private String mCarrier;

    public ApnHandlerHelper(Context context, String carrier) {
        mContext = context;
        mCarrier = carrier;
    }

    public void restoreDefaultAPN() {
        //clear content://telephony/carriers and then re-initialize database
        log("+++ restoreDefaultAPN()");
        mContext.getContentResolver().delete(RESTORE_URL, null, null);
        log("--- restoreDefaultAPN()");
    }

    public void clearAllAPN() {
        log("clear content://telephony/carriers");
        mContext.getContentResolver().delete(CARRIERS_URL, null, null);
    }

    public void restoreLabTestAPN() {
        // 1. clear content://telephony/carriers
        log("+++ restoreLabTestAPN");
        clearAllAPN();

        // 2. Need parse Lab Mbn and insert to content://telephony/carriers
        Resources r = mContext.getResources();
        XmlResourceParser parser = r.getXml(com.android.internal.R.xml.apns);
        int publicversion = -1;
        try {
            XmlUtils.beginDocument(parser, "apns");
            publicversion = Integer.parseInt(parser.getAttributeValue(null, "version"));
        } catch (Exception e) {
            log("Got exception while loading APN database." + e);
        } finally {
            parser.close();
        }

        XmlPullParser confparser = null;
        File confFile = new File(Environment.getRootDirectory(), PARTNER_APNS_PATH);
        FileReader confreader = null;
        try {
            confreader = new FileReader(confFile);
            confparser = Xml.newPullParser();
            confparser.setInput(confreader);
            XmlUtils.beginDocument(confparser, "apns");

            // Sanity check. Force internal version and confidential versions to agree
            int confversion = Integer.parseInt(confparser.getAttributeValue(null, "version"));
            if (publicversion != confversion) {
                throw new IllegalStateException("Internal APNS file version doesn't match "
                        + confFile.getAbsolutePath());
            }

            loadApns(confparser);
        } catch (FileNotFoundException e) {
            // It's ok if the file isn't found. It means there isn't a confidential file
            log("File not found: '" + confFile.getAbsolutePath() + "'");
        } catch (Exception e) {
            log("Exception while parsing '" + confFile.getAbsolutePath() + "'" + e);
        } finally {
            try { if (confreader != null) confreader.close(); } catch (IOException e) { }
        }
        log("--- restoreLabTestAPN");
    }

    private void loadApns(XmlPullParser parser) {
        if (parser != null) {
            try {
                XmlUtils.nextElement(parser);
                String labtest;
                while (parser.getEventType() != XmlPullParser.END_DOCUMENT) {
                    if (!"apn".equals(parser.getName())) {
                        throw new XmlPullParserException("Expected 'apn' tag", parser, null);
                    }

                    labtest = parser.getAttributeValue(null, LAB_TEST_TAG);
                    if (TextUtils.isEmpty(labtest) || !labtest.equals(mCarrier)) {
                        XmlUtils.nextElement(parser);
                        continue;
                    }
                    ContentValues row = getRow(parser);
                    insertAddingDefaults(row);
                    XmlUtils.nextElement(parser);
                }
            } catch (XmlPullParserException e) {
                log("Got XmlPullParserException while loading apns." + e);
            } catch (IOException e) {
                log("Got IOException while loading apns." + e);
            } catch (SQLException e) {
                log("Got SQLException while loading apns." + e);
            } finally {
            }
        }
    }

    private void insertAddingDefaults(ContentValues row) {
        // Initialize defaults if any
        if (row.containsKey(Telephony.Carriers.AUTH_TYPE) == false) {
            row.put(Telephony.Carriers.AUTH_TYPE, -1);
        }
        if (row.containsKey(Telephony.Carriers.PROTOCOL) == false) {
            row.put(Telephony.Carriers.PROTOCOL, mContext.getString(R.string.default_protocol));
        }
        if (row.containsKey(Telephony.Carriers.ROAMING_PROTOCOL) == false) {
            row.put(Telephony.Carriers.ROAMING_PROTOCOL,
                    mContext.getString(R.string.default_protocol));
        }
        if (row.containsKey(Telephony.Carriers.CARRIER_ENABLED) == false) {
            row.put(Telephony.Carriers.CARRIER_ENABLED, true);
        }
        if (row.containsKey(Telephony.Carriers.BEARER) == false) {
            row.put(Telephony.Carriers.BEARER, 0);
        }
        if (row.containsKey(Telephony.Carriers.MVNO_TYPE) == false) {
            row.put(Telephony.Carriers.MVNO_TYPE, "");
        }
        if (row.containsKey(Telephony.Carriers.MVNO_MATCH_DATA) == false) {
            row.put(Telephony.Carriers.MVNO_MATCH_DATA, "");
        }

        if (row.containsKey(mContext.getString(R.string.read_only)) == false) {
            row.put(mContext.getString(R.string.read_only), false);
        }

        if (row.containsKey(mContext.getString(R.string.v_mccmnc)) == false) {
            row.put(mContext.getString(R.string.v_mccmnc), "000000");
        }
        mContext.getContentResolver().insert(CARRIERS_URL, row);
    }

    private ContentValues getRow(XmlPullParser parser) {


        ContentValues map = new ContentValues();

        String mcc = parser.getAttributeValue(null, "mcc");
        String mnc = parser.getAttributeValue(null, "mnc");
        String numeric = mcc + mnc;

        map.put(Telephony.Carriers.NUMERIC,numeric);
        map.put(Telephony.Carriers.MCC, mcc);
        map.put(Telephony.Carriers.MNC, mnc);
        map.put(Telephony.Carriers.NAME, parser.getAttributeValue(null, "carrier"));
        map.put(Telephony.Carriers.APN, parser.getAttributeValue(null, "apn"));
        map.put(Telephony.Carriers.USER, parser.getAttributeValue(null, "user"));
        map.put(Telephony.Carriers.SERVER, parser.getAttributeValue(null, "server"));
        map.put(Telephony.Carriers.PASSWORD, parser.getAttributeValue(null, "password"));
        map.put(mContext.getString(R.string.ppp_number),
                         parser.getAttributeValue(null, "ppp_number"));
        map.put(mContext.getString(R.string.localized_name),
                parser.getAttributeValue(null, "localized_name"));
        map.put(VISIT_AREA, parser.getAttributeValue(null, "visit_area"));
        // do not add NULL to the map so that insert() will set the default value
        String proxy = parser.getAttributeValue(null, "proxy");
        if (proxy != null) {
            map.put(Telephony.Carriers.PROXY, proxy);
        }
        String port = parser.getAttributeValue(null, "port");
        if (port != null) {
            map.put(Telephony.Carriers.PORT, port);
        }
        String mmsproxy = parser.getAttributeValue(null, "mmsproxy");
        if (mmsproxy != null) {
            map.put(Telephony.Carriers.MMSPROXY, mmsproxy);
        }
        String mmsport = parser.getAttributeValue(null, "mmsport");
        if (mmsport != null) {
            map.put(Telephony.Carriers.MMSPORT, mmsport);
        }
        map.put(Telephony.Carriers.MMSC, parser.getAttributeValue(null, "mmsc"));
        String type = parser.getAttributeValue(null, "type");
        if (type != null) {
            map.put(Telephony.Carriers.TYPE, type);
        }

        String auth = parser.getAttributeValue(null, "authtype");
        if (auth != null) {
            map.put(Telephony.Carriers.AUTH_TYPE, Integer.parseInt(auth));
        }

        String protocol = parser.getAttributeValue(null, "protocol");
        if (protocol != null) {
            map.put(Telephony.Carriers.PROTOCOL, protocol);
        }
        String roamingProtocol = parser.getAttributeValue(null, "roaming_protocol");
        if (roamingProtocol != null) {
            map.put(Telephony.Carriers.ROAMING_PROTOCOL, roamingProtocol);
        }

        String carrierEnabled = parser.getAttributeValue(null, "carrier_enabled");
        if (carrierEnabled != null) {
            map.put(Telephony.Carriers.CARRIER_ENABLED, Boolean.parseBoolean(carrierEnabled));
        }

        String bearer = parser.getAttributeValue(null, "bearer");
        if (bearer != null) {
            map.put(Telephony.Carriers.BEARER, Integer.parseInt(bearer));
        }

        String mvno_type = parser.getAttributeValue(null, "mvno_type");
        if (mvno_type != null) {
            String mvno_match_data = parser.getAttributeValue(null, "mvno_match_data");
            if (mvno_match_data != null) {
                map.put(Telephony.Carriers.MVNO_TYPE, mvno_type);
                map.put(Telephony.Carriers.MVNO_MATCH_DATA, mvno_match_data);
            }
        }

        String readOnly = parser.getAttributeValue(null, "read_only");
        if (readOnly != null) {
            map.put(mContext.getString(R.string.read_only), Boolean.parseBoolean(readOnly));
        }

        String v_mccmnc = parser.getAttributeValue(null, "v_mccmnc");
        if (v_mccmnc != null) {
            map.put(mContext.getString(R.string.v_mccmnc), v_mccmnc);
        }
        return map;
    }

    private void log(String msg) {
        Log.d(TAG, "MbnTest_ " + msg);
    }
}
