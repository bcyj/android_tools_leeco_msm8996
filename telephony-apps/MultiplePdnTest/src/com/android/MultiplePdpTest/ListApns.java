/******************************************************************************
 * @file    ListApns.java
 * @brief   List activity for displaying certain active APNs
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 * ---------------------------------------------------------------------------
 *
 ******************************************************************************/

package com.android.MultiplePdpTest;

import java.util.ArrayList;
import java.util.HashMap;

import android.content.Intent;
import android.content.ContentUris;
import android.database.Cursor;
import android.os.Bundle;
import android.net.Uri;
import android.app.ListActivity;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView;
import android.view.View;
import android.util.Log;
import android.content.ComponentName;
import android.content.ContentValues;
import com.android.MultiplePdpTest.ServiceTypeListActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.R.drawable;

public class ListApns extends ListActivity {
    /** This class is a list activity that solely
     *  displays all active APNs to users for a specific
     *  IP version and Network Type
     */
    public String TAG = "MPDP-TEST";
    private ListAdapter mAdapter;
    private ArrayList<String> APN_LIST;
    private ArrayList<ProfileDetails> PROFILES_ARRAY;
    private static int NAME_INDEX = 0;
    private static int TYPE_INDEX = 1;
    private static int MENU_ADD = 0;
    private static int MENU_ADD_GENERAL = 1;

    private Uri CARRIERS_CONTENT = Uri.parse("content://telephony/carriers");
    private Uri PREF_APN = Uri.parse("content://telephony/carriers/preferapn");
    private String EDIT_APN = "android.intent.action.EDIT";

    private class ProfileDetails {
        /** Simple class for holding information
         *  about an APN information gotten from the
         *  Carriers table
         */
        public String name;
        public String[] types;
        public String ipver;
        public int id;

        public ProfileDetails(String nm, String tp, String ip, int carriersId) {
            name = nm;
            if (tp == null) {
                types = new String[] {"*"};
            } else {
                types = tp.split(",");
            }
            ipver = ip;
            id = carriersId;
        }

        private boolean isTypeSupported (String type) {
            type = type.toLowerCase();
            type = type.trim();
            for (String singleType : types) {
                singleType = singleType.trim().toLowerCase();
                boolean areSame = singleType.equals(type);
                boolean singleTypeSupportsAll = singleType.equals("*") || (singleType.length() == 0);
                boolean specialCase = (singleType.equals("default")) && (type.equals("hipri"));
                if (areSame || singleTypeSupportsAll || specialCase) {
                    return true;
                }
            }
            return false;
        }

        private boolean isIpVersionSupported(String ip) {
            return (ipver.equals(ip)) || (ipver.equals("IPV4V6"));
        }

        public boolean canSupportAndIsValid(String type, String ip) {
            if (name == null) {
                Uri url = ContentUris.withAppendedId(CARRIERS_CONTENT, id);
                getContentResolver().delete(url, null, null);
                return false;
            }
            if (name.length() == 0) {
                Uri url = ContentUris.withAppendedId(CARRIERS_CONTENT, id);
                getContentResolver().delete(url, null, null);
                return false;
            }
            return isTypeSupported(type) && isIpVersionSupported(ip);
        }
    }

    private ArrayList<ProfileDetails> createProfilesArray() {
        ArrayList<ProfileDetails> retVal = (ArrayList<ProfileDetails>) new ArrayList();
        String where = "numeric=\"" + selectedNumeric() + "\"";
        String serviceType = getIntent().getStringExtra(ServiceTypeInformer.SERVICE_TYPE);
        String ipVersion = getIntent().getStringExtra(ServiceTypeInformer.IP_TYPE);
        int NAME = 0;
        int TYPE = 1;
        int IP = 2;
        int ID = 3;

        Cursor c = this.getContentResolver().query(CARRIERS_CONTENT, new String[] {"name", "type", "protocol", "_id"}, where, null, null);
        if (c.moveToFirst()) {
            while (!c.isAfterLast()) {
            ProfileDetails profile = new ProfileDetails(c.getString(NAME), c.getString(TYPE), c.getString(IP), c.getInt(ID));
            if (profile.canSupportAndIsValid(serviceType, ipVersion)) {
                retVal.add(profile);
            }
                c.moveToNext();
            }
        }
        return retVal;
    }

    private ArrayList<String> createApnListFromProfilesArray() {
        ArrayList<String> retVal = (ArrayList<String>) new ArrayList();
        for (ProfileDetails profile : PROFILES_ARRAY) {
            retVal.add(profile.name);
        }
        return retVal;
    }

    OnItemClickListener mListener = new OnItemClickListener() {
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            String name = (String) parent.getItemAtPosition(position);
            int lookfor = 0;
            for (ProfileDetails detail : PROFILES_ARRAY) {
                if (detail.name.equals(name)) {
                    lookfor = detail.id;
                    break;
                }
            }
                Uri url = ContentUris.withAppendedId(CARRIERS_CONTENT, lookfor);
                Intent intent = new Intent(EDIT_APN, url);
                intent.setComponent(new ComponentName("com.android.settings", "ApnEditor"));
                startActivity(new Intent(EDIT_APN, url));
        }
    };

    @Override
    public void onResume() {
        super.onResume();
        PROFILES_ARRAY = createProfilesArray();
        APN_LIST = createApnListFromProfilesArray();
        if (APN_LIST.isEmpty()) {
            setListAdapter(mAdapter);
            return;
        }
        mAdapter = new ArrayAdapter<String>(this, R.layout.list_item, APN_LIST);
        setListAdapter(mAdapter);
        ListView view = getListView();
        view.setOnItemClickListener(mListener);
    }

    @Override
    public void onPause() {
        super.onPause();
        APN_LIST.clear();
        PROFILES_ARRAY.clear();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        menu.add(0, MENU_ADD, 0, "New IPV" + getIntent().getStringExtra(ServiceTypeInformer.IP_TYPE) + " " +
            getIntent().getStringExtra(ServiceTypeInformer.SERVICE_TYPE).toUpperCase()+ " APN").setIcon(android.R.drawable.ic_menu_add);
        menu.add(0, MENU_ADD_GENERAL, 0, "New APN").setIcon(android.R.drawable.ic_menu_add);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == MENU_ADD) {
            Uri url = getNewApnUri();
            startActivity(new Intent(EDIT_APN, url));
            return true;
        } else if (item.getItemId() == MENU_ADD_GENERAL) {
            startActivity(new Intent(Intent.ACTION_INSERT, CARRIERS_CONTENT));
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private Uri getNewApnUri() {
        String[] fields = {
            "name",
            "apn",
            "proxy",
            "port",
            "mmsproxy",
            "mmsport",
            "server",
            "user",
            "password",
            "mmsc"
        };
        Uri mUri = getContentResolver().insert(CARRIERS_CONTENT, new ContentValues());
        ContentValues values = new ContentValues();
        for (String unSetField : fields) {
            values.put(unSetField, "");
        }
        values.put("protocol", getIntent().getStringExtra(ServiceTypeInformer.IP_TYPE));
        values.put("type", getIntent().getStringExtra(ServiceTypeInformer.SERVICE_TYPE));
        String numeric = selectedNumeric();
        if (numeric != null && numeric.length() > 4) {
            values.put("mcc", numeric.substring(0, 3));
            values.put("mnc", numeric.substring(3));
            values.put("numeric", numeric);
            values.put("current", 1);
        }
        getContentResolver().update(mUri, values, null, null);
        return mUri;
    }

    private String selectedNumeric() {
        int NUMERIC_INDEX = 0;
        Cursor c = this.getContentResolver().query(PREF_APN, new String[] {"numeric"}, null, null, null);
        if (c.moveToFirst() && c.getString(NUMERIC_INDEX) != null) {
            return c.getString(NUMERIC_INDEX);
        } else {
            return "";
        }
    }
}
