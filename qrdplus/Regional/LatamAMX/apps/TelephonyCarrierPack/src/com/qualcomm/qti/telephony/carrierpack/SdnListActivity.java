/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.qualcomm.qti.telephony.carrierpack;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.CursorAdapter;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;

import static android.view.Window.PROGRESS_VISIBILITY_OFF;
import static android.view.Window.PROGRESS_VISIBILITY_ON;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.IIccPhoneBook;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;

/* EF_SDN file from the SIM card if present will be read and the name,
 * number will be displayed here
 *
 * Logic of this activity is similar to ADNList from base AOSP
 */

public class SdnListActivity extends ListActivity
    implements View.OnCreateContextMenuListener{
    private static final String TAG = "SdnList";
    private static final boolean DBG = true;

    private static final String[] COLUMN_NAMES = new String[] {
        "name",
        "number"
    };

    private static final int NAME_COLUMN = 0;
    private static final int NUMBER_COLUMN = 1;

    private static final int CONTEXT_MENU_CALL_CONTACT = 1;
    private static final int CONTEXT_MENU_MSG_CONTACT = 2;
    private static final String SCHEME_SMS = "smsto";

    private static final int[] VIEW_NAMES = new int[] {
        android.R.id.text1,
        android.R.id.text2
    };

    private static final int QUERY_TOKEN = 0;

    private QueryHandler mQueryHandler;
    private CursorAdapter mCursorAdapter;
    private Cursor mCursor = null;

    private TextView mEmptyText;

    private int mInitialSelection = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().requestFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.activity_sdn_list);
        mEmptyText = (TextView) findViewById(android.R.id.empty);
        mQueryHandler = new QueryHandler(getContentResolver());
        getListView().setOnCreateContextMenuListener(this);

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        query();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mCursor != null) {
            mCursor.deactivate();
        }
    }

    private Uri resolveIntent() {
        Intent intent = getIntent();
        if (intent.getData() == null) {
            intent.setData(Uri.parse("content://icc/sdn"));
        }
        return intent.getData();
    }

    private void query() {
        Uri uri = resolveIntent();
        if (DBG) log("query: starting an async query");
        mQueryHandler.startQuery(QUERY_TOKEN, null, uri, COLUMN_NAMES,
                null, null, null);
        displayProgress(true);
    }

    private void reQuery() {
        query();
    }

    private void setAdapter() {
        // NOTE:
        // As it it written, the positioning code below is NOT working.
        // However, this current non-working state is in compliance with
        // the UI paradigm, so we can't really do much to change it.

        // In the future, if we wish to get this "positioning" correct,
        // we'll need to do the following:
        //   1. Change the layout to in the cursor adapter to:
        //     android.R.layout.simple_list_item_checked
        //   2. replace the selection / focus code with:
        //     getListView().setChoiceMode(ListView.CHOICE_MODE_SINGLE);
        //     getListView().setItemChecked(mInitialSelection, true);

        // Since the positioning is really only useful for the dialer's
        // SpecialCharSequence case (dialing '2#' to get to the 2nd
        // contact for instance), it doesn't make sense to mess with
        // the usability of the activity just for this case.

        // These artifacts include:
        //  1. UI artifacts (checkbox and highlight at the same time)
        //  2. Allowing the user to edit / create new SIM contacts when
        //    the user is simply trying to retrieve a number into the d
        //    dialer.

        if (mCursorAdapter == null) {
            mCursorAdapter = newAdapter();
            setListAdapter(mCursorAdapter);
        } else {
            mCursorAdapter.changeCursor(mCursor);
        }

        if (mInitialSelection >= 0 && mInitialSelection < mCursorAdapter.getCount()) {
            setSelection(mInitialSelection);
            getListView().setFocusableInTouchMode(true);
            boolean gotfocus = getListView().requestFocus();
        }
    }

    protected CursorAdapter newAdapter() {
        return new SimpleCursorAdapter(this,
                    android.R.layout.simple_list_item_2,
                    mCursor, COLUMN_NAMES, VIEW_NAMES);
    }

    protected void displayProgress(boolean loading) {
        if (DBG) log("displayProgress: " + loading);

        mEmptyText.setText(loading ? R.string.simContacts_emptyLoading:
            (isAirplaneModeOn(this) ? R.string.simContacts_airplaneMode :
                R.string.simContacts_empty));
        getWindow().setFeatureInt(
                Window.FEATURE_INDETERMINATE_PROGRESS,
                loading ? PROGRESS_VISIBILITY_ON : PROGRESS_VISIBILITY_OFF);
    }

    private static boolean isAirplaneModeOn(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, 0) != 0;
    }

    protected class QueryHandler extends AsyncQueryHandler {
        public QueryHandler(ContentResolver cr) {
            super(cr);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor c) {
            if (DBG) log("onQueryComplete: cursor.count=" + c.getCount());
            mCursor = c;
            setAdapter();
            displayProgress(false);
            // Cursor is refreshed and inherited classes may have menu items depending on it.
            invalidateOptionsMenu();
        }
    }

    private void showAlertDialog(String value) {
        AlertDialog alertDialog = new AlertDialog.Builder(this).create();
        alertDialog.setTitle(getString(R.string.result));
        alertDialog.setMessage(value);
        alertDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        alertDialog.setButton(getString(R.string.ok), new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
            }
        });
        alertDialog.show();
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View view, ContextMenuInfo menuInfoIn) {
        AdapterView.AdapterContextMenuInfo menuInfo;
        try {
             menuInfo = (AdapterView.AdapterContextMenuInfo) menuInfoIn;
        } catch (ClassCastException e) {
            Log.e(TAG, "bad menuInfoIn", e);
            return;
        }

        Cursor cursor = (Cursor) mCursorAdapter.getItem(menuInfo.position);

        String number = cursor.getString(NUMBER_COLUMN);
        Uri numberUri = null;
        numberUri = Uri.fromParts("tel", number, null);

        String name = cursor.getString(NAME_COLUMN);
        if (name.isEmpty()) {
            menu.setHeaderTitle(number);
        } else {
            menu.setHeaderTitle(name);
       }

        if (numberUri != null) {
            Intent intent = new Intent(Intent.ACTION_CALL_PRIVILEGED, numberUri);
            menu.add(0, CONTEXT_MENU_CALL_CONTACT, 0,
                     getResources().getString(R.string.callTo, number))
                    .setIntent(intent);
        }
        if (numberUri != null) {
            Intent numberIntent =
                    new Intent(Intent.ACTION_SENDTO, Uri.fromParts(SCHEME_SMS, number, null));
            log("numberUri = " + numberUri) ;
            menu.add(0, CONTEXT_MENU_MSG_CONTACT, 0,
                     R.string.sendTextMessage)
                    .setIntent(numberIntent);
        }
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case CONTEXT_MENU_CALL_CONTACT:
            case CONTEXT_MENU_MSG_CONTACT:
                startActivity(item.getIntent());
                return true;
            default:
                return super.onContextItemSelected(item);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
        default:
        }
        return super.onOptionsItemSelected(item);
    }

    protected void log(String msg) {
        Log.d(TAG, "[SDNList] " + msg);
    }

}

