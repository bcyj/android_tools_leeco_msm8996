/******************************************************************************
 * @file    ServiceTypeListActivity.java
 * @brief   Main activity for Multiple PDP Test application
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
import java.util.Arrays;
import java.util.HashMap;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.app.ListActivity;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.AdapterView.OnItemClickListener;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

import com.android.MultiplePdpTest.TypeManager;


public class ServiceTypeListActivity extends ListActivity {
    /** Main page of app
     *  Displays all available Service types to user
     *  And option to edit data profiles from the main page
     */

    private static final String SETTINGS_WORK = "com.android.MultiplePdpTest.SELECTED_SERVICE_ACTIVITY";
    public static final String SELECTED_SERVICE_TYPE = "com.android.MultiplePdpTest.SELECTED_SERVICE_TYPE";
    private static final String EDIT_PROFILES = "android.settings.APN_SETTINGS";
    private Uri CARRIERS_CONTENT = Uri.parse("content://telephony/carriers");
    private Uri PREF_APN = Uri.parse("content://telephony/carriers/preferapn");

    public static boolean DBG = true;
    public static boolean sInitialized = false;

    private String TAG = "MPDP-TEST";

    // TODO: look into the implementation of this typeManager
    // there might be a better way to maintain a hash map of the data connections that have been update
    public static TypeManager typeManager;

    public static final String APN_ENTRY_EDIT_PROFILES = ">> Edit Profiles";

    // APN_TYPES and APN_FEATURE_TYPES must be in sync
    // @TODO: remove this dependency
    public static final String[] APN_TYPES = new String[] {
        PhoneConstants.APN_TYPE_MMS,
        PhoneConstants.APN_TYPE_SUPL,
        PhoneConstants.APN_TYPE_DUN,
        PhoneConstants.APN_TYPE_HIPRI,
        PhoneConstants.APN_TYPE_FOTA,
        PhoneConstants.APN_TYPE_IMS,
        PhoneConstants.APN_TYPE_CBS,
    };

    public static final String[] APN_FEATURE_TYPES = new String[] {
            Phone.FEATURE_ENABLE_MMS,
            Phone.FEATURE_ENABLE_SUPL,
            Phone.FEATURE_ENABLE_DUN,
            Phone.FEATURE_ENABLE_HIPRI,
            Phone.FEATURE_ENABLE_FOTA,
            Phone.FEATURE_ENABLE_IMS,
            Phone.FEATURE_ENABLE_CBS,
    };

    public static ArrayList<String> LIST_ENTRIES = new ArrayList<String>();

    private OnItemClickListener mClickListener = new OnItemClickListener() {
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            if (LIST_ENTRIES.get(position).equals(APN_ENTRY_EDIT_PROFILES)) {
                Intent mIntent = new Intent(EDIT_PROFILES);
                startActivity(mIntent);
            } else {
                Intent mIntent = new Intent(SETTINGS_WORK);
                mIntent.putExtra(SELECTED_SERVICE_TYPE, position);
                startActivity(mIntent);
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        typeManager = new TypeManager(getBaseContext());

        LIST_ENTRIES.clear();
        LIST_ENTRIES.addAll(Arrays.asList(APN_TYPES));
        LIST_ENTRIES.add(APN_ENTRY_EDIT_PROFILES);

        setListAdapter(new ArrayAdapter<String>(this, R.layout.list_item, LIST_ENTRIES));

        ListView mView = getListView();
        mView.setTextFilterEnabled(true);
        mView.setOnItemClickListener(mClickListener);
        sInitialized = true;
        }
}
