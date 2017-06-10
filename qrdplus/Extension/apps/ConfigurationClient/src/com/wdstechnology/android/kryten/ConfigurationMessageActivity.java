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
import java.util.Collection;
import java.util.HashSet;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.app.PendingIntent;
import android.content.AsyncQueryHandler;
import android.content.ContentProviderOperation;
import android.content.ContentProviderOperation.Builder;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.os.Bundle;
import android.util.Log;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;
import android.widget.TextView;
import android.database.sqlite.SqliteWrapper;
import android.graphics.drawable.ColorDrawable;
import android.os.SystemProperties;

public class ConfigurationMessageActivity extends ListActivity {

    public static final String TAG = "ConfigurationMessageActivity";
    private CharSequence mTitle;
    private byte[] mDocument;
    private String from;
    private String secString;
    private String macString;
    private ConfigurationListAdapter mListAdapter;
    private ThreadListQueryHandler mQueryHandler;
    private static final int QUERY_TOKEN = 1701;

    @Override
    protected void onCreate(android.os.Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        if (ProvisioningPushReceiver.DEBUG_TAG)
            Log.d(TAG, "onCreate");

        String device = SystemProperties.get("gsm.sim.operator.numeric");

        Intent myIntent = getIntent();
        setContentView(R.layout.conversation_list_screen);
        ListView listView = getListView();
        listView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE_MODAL);
        listView.setMultiChoiceModeListener(new ModeCallback());
        mQueryHandler = new ThreadListQueryHandler(getContentResolver());

        // Tell the list view which view to display when the list is empty
        View emptyView = findViewById(R.id.empty);
        listView.setEmptyView(emptyView);

        initListAdapter();
        mTitle = getString(R.string.app_label);
        setupActionBar();

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // TODO Auto-generated method stub
        if (ProvisioningPushReceiver.DEBUG_TAG)
            Log.d(TAG, "onCreateOptionsMenu");
        getMenuInflater().inflate(R.menu.conversation_list_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
            case R.id.delete_all:
                ProvisioningNotification.clearNotification(ConfigurationMessageActivity.this);
                confirmDelete();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    // TODO Auto-generated method stub
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem item = menu.findItem(R.id.delete);
        if (item != null) {
            item.setVisible(mListAdapter.getCount() > 0);
        }
        item = menu.findItem(R.id.delete_all);
        if (item != null) {
            item.setVisible(mListAdapter.getCount() > 0);
        }
        return true;
    }

    private void setupActionBar() {
        ActionBar actionBar = getActionBar();
        actionBar.setHomeButtonEnabled(true);
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setBackgroundDrawable(new ColorDrawable(getResources()
                .getColor(R.color.holo_blue)));
        actionBar.setTitle(mTitle);
    }

    private final ConfigurationListAdapter.OnContentChangedListener mContentChangedListener =
            new ConfigurationListAdapter.OnContentChangedListener() {
                public void onContentChanged(ConfigurationListAdapter adapter) {
                    if (ProvisioningPushReceiver.DEBUG_TAG)
                        Log.d(TAG, "onContentChanged");
                    startAsyncQuery();
                }
            };

    private void initListAdapter() {
        mListAdapter = new ConfigurationListAdapter(this, null);
        mListAdapter.setOnContentChangedListener(mContentChangedListener);
        setListAdapter(mListAdapter);
        getListView().setRecyclerListener(mListAdapter);
    }

    private void startAsyncQuery() {
        try {
            setTitle(getString(R.string.refreshing));
            setProgressBarIndeterminateVisibility(true);
            mQueryHandler.startQuery(QUERY_TOKEN, null,
                    ConfigurationDatabaseProvider.CONTENT_URI, null, null,
                    null, ConfigurationDatabaseProvider.Columns.DATE + " DESC");
        } catch (SQLiteException e) {
            SqliteWrapper.checkSQLiteException(this, e);
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        if (ProvisioningPushReceiver.DEBUG_TAG)
            Log.d(TAG, "onListItemClick id " + id);
        Intent intent = new Intent();
        intent.setClass(ConfigurationMessageActivity.this,
                DisplayActivity.class);
        intent.putExtra("id", id);
        startActivity(intent);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

    private final class ThreadListQueryHandler extends AsyncQueryHandler {
        public ThreadListQueryHandler(ContentResolver contentResolver) {
            super(contentResolver);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            if (cursor == null) {
                Log.e(TAG, "onQueryComplete, cursor is null!");
                return;
            }
            switch (token) {
                case QUERY_TOKEN:
                    mListAdapter.changeCursor(cursor);
                    invalidateOptionsMenu();
                    setTitle(mTitle);
                    setProgressBarIndeterminateVisibility(false);

                    break;

                default:
                    Log.e(TAG, "onQueryComplete called with unknown token " + token);
                    cursor.close();
            }
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (ProvisioningPushReceiver.DEBUG_TAG)
            Log.d(TAG, "onStart");
        ProvisioningNotification
                .clearNotification(ConfigurationMessageActivity.this);

        startAsyncQuery();
    }

    public static PendingIntent createPendingValidationActivity(
            Context context, String sec, String mac, byte[] document,
            String from) {

        if (ProvisioningPushReceiver.DEBUG_TAG)
            Log.d(TAG, "createPendingValidationActivity ");
        Intent validationIntent = new Intent(context,
                ConfigurationMessageActivity.class);
        validationIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if (sec != null) {
            validationIntent.putExtra("com.wdstechnology.android.kryten.SEC",
                    sec);
        }
        if (mac != null) {
            validationIntent.putExtra("com.wdstechnology.android.kryten.MAC",
                    mac);
        }
        validationIntent.putExtra(
                "com.wdstechnology.android.kryten.provisioning-data", document);
        validationIntent.putExtra("from", from);

        return PendingIntent.getActivity(context, 0, validationIntent,
                PendingIntent.FLAG_CANCEL_CURRENT);
    }

    public static ArrayList<Long> mSelectedThreadIds;
    public ActionMode mActionMode;

    private class ModeCallback implements ListView.MultiChoiceModeListener {
        private View mMultiSelectActionBarView;
        private TextView mSelectedConvCount;

        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            MenuInflater inflater = getMenuInflater();
            mActionMode = mode;
            mSelectedThreadIds = new ArrayList<Long>();
            inflater.inflate(R.menu.conversation_multi_select_menu, menu);

            if (mMultiSelectActionBarView == null) {
                mMultiSelectActionBarView = (ViewGroup) LayoutInflater
                        .from(ConfigurationMessageActivity.this)
                        .inflate(
                                R.layout.conversation_list_multi_select_actionbar,
                                null);

                mSelectedConvCount = (TextView) mMultiSelectActionBarView
                        .findViewById(R.id.selected_conv_count);
            }
            mode.setCustomView(mMultiSelectActionBarView);
            ((TextView) mMultiSelectActionBarView.findViewById(R.id.title))
                    .setText(R.string.select_conversations);
            return true;
        }

        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            if (mMultiSelectActionBarView == null) {
                ViewGroup v = (ViewGroup) LayoutInflater
                        .from(ConfigurationMessageActivity.this)
                        .inflate(
                                R.layout.conversation_list_multi_select_actionbar,
                                null);
                mode.setCustomView(v);

                mSelectedConvCount = (TextView) v
                        .findViewById(R.id.selected_conv_count);
            }
            return true;
        }

        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
                case R.id.delete:
                    if (mSelectedThreadIds.size() > 0) {
                        ProvisioningNotification
                                .clearNotification(ConfigurationMessageActivity.this);
                        confirmDelete(mSelectedThreadIds, mQueryHandler);
                    }
                    break;

                default:
                    break;
            }
            return true;
        }

        public void onDestroyActionMode(ActionMode mode) {
            mActionMode = null;
            mSelectedThreadIds = null;
        }

        public void onItemCheckedStateChanged(ActionMode mode, int position,
                long id, boolean checked) {
            ListView listView = getListView();
            final int checkedCount = listView.getCheckedItemCount();
            mSelectedConvCount.setText(Integer.toString(checkedCount));
            if (checked) {
                mSelectedThreadIds.add(id);
            } else {
                mSelectedThreadIds.remove(id);
            }

            ConfigurationListAdapter adapter = (ConfigurationListAdapter) getListView()
                    .getAdapter();
            adapter.notifyDataSetChanged();
        }
    }

    public void confirmDelete(final ArrayList<Long> ids,
            AsyncQueryHandler handler) {
        AlertDialog confirm = new AlertDialog.Builder(
                ConfigurationMessageActivity.this)
                .setMessage(getString(R.string.confirm_delete))
                .setTitle(getString(R.string.confirm))
                .setPositiveButton(getString(android.R.string.yes),
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                // TODO Auto-generated method stub
                                ArrayList<ContentProviderOperation> mOperations =
                                        new ArrayList<ContentProviderOperation>();
                                for (int i = 0; i < mSelectedThreadIds.size(); i++) {
                                    ContentProviderOperation co = ContentProviderOperation
                                            .newDelete(
                                                    ConfigurationDatabaseProvider.CONTENT_URI)
                                            .withSelection(
                                                    ConfigurationDatabaseProvider.Columns._ID
                                                            + " = "
                                                            + ids.get(i), null)
                                            .build();
                                    mOperations.add(co);
                                }
                                try {
                                    ConfigurationMessageActivity.this
                                            .getContentResolver()
                                            .applyBatch(
                                                    ConfigurationDatabaseProvider.AUTHORITY,
                                                    mOperations);
                                } catch (android.os.RemoteException e) {

                                } catch (android.content.OperationApplicationException e) {

                                }
                                if (mActionMode != null) {
                                    mActionMode.finish();
                                }
                            }
                        })
                .setNegativeButton(getString(android.R.string.no),
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                // TODO Auto-generated method stub

                            }
                        }).create();
        confirm.show();
    }

    public void confirmDelete() {
        AlertDialog confirm = new AlertDialog.Builder(
                ConfigurationMessageActivity.this)
                .setMessage(getString(R.string.confirm_delete_all))
                .setTitle(getString(R.string.confirm))
                .setPositiveButton(getString(android.R.string.yes),
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                ConfigurationMessageActivity.this
                                        .getContentResolver()
                                        .delete(ConfigurationDatabaseProvider.CONTENT_URI,
                                                null, null);
                            }
                        })
                .setNegativeButton(getString(android.R.string.no),
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                // TODO Auto-generated method stub

                            }
                        }).create();
        confirm.show();

    }

}
