/**
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.homelocation;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.DownloadManager.Query;
import android.app.ExpandableListActivity;
import android.app.SearchManager;
import android.content.IContentProvider;
import android.content.Intent;
import android.os.Bundle;
import android.provider.SearchRecentSuggestions;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class AreaListActivity extends ExpandableListActivity {

    private String query;
    private static final int MENU_ID_SCAN = 1;
    private SearchRecentSuggestions mSearchRecentSuggestions;

    private void clearSearchHistory() {
        mSearchRecentSuggestions.clearHistory();
    }

    private void saveSearchHistory(String search) {
        mSearchRecentSuggestions.saveRecentQuery(search, null);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        clearSearchHistory();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.area_list);
        mSearchRecentSuggestions = new SearchRecentSuggestions(this,
                SearchSuggestionProvider.AUTHORITY, SearchSuggestionProvider.DATABASE_MODE_QUERIES);
        getExpandableListView().setAdapter(new AreaListAdapter(this));
    }

    @Override
    public boolean onSearchRequested() {
        startSearch(null, false, null, false);
        return true;
    }

    // query database
    public void query() {
        String address = null;
        if (getContentResolver().acquireProvider(GeocodedLocation.CONTENT_URI) != null) {
            Bundle result = getContentResolver().call(GeocodedLocation.CONTENT_URI, "getLocation",
                    query, null);
            if (result != null) {
                address = result.getString("location");
            }
        }
        if (address == null) {
            ResultDialog.newInstance(query, getString(R.string.result)).show(
                    getFragmentManager(), "dialog");
        } else {
            ResultDialog.newInstance(query, address).show(getFragmentManager(), "dialog");
        }
        saveSearchHistory(query);
    }

    // query result
    @Override
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        // Get the value in the search box
        query = intent.getStringExtra(SearchManager.QUERY);
        if (TextUtils.isEmpty(query)) {
            return;
        }
        if (TextUtils.isDigitsOnly(query)) {
            query();
        } else {
            ResultDialog.newInstance(query, getString(R.string.numerror)).show(
                    getFragmentManager(), "dialog");
        }
    }

    // show result
    private void showquery(String result) {
        AlertDialog.Builder builder = new AlertDialog.Builder(
                AreaListActivity.this).setTitle(query).setMessage(result);
        AlertDialog dlg = builder.create();
        dlg.setCanceledOnTouchOutside(true);
        dlg.show();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(Menu.NONE, MENU_ID_SCAN, 0, R.string.search_networks).setShowAsAction(
                MenuItem.SHOW_AS_ACTION_IF_ROOM);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_ID_SCAN:
                onSearchRequested();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    public static class ResultDialog extends DialogFragment {

        public static ResultDialog newInstance(String title, String result) {
            ResultDialog frag = new ResultDialog();
            Bundle args = new Bundle();
            args.putString("title", title);
            args.putString("message", result);
            frag.setArguments(args);
            return frag;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            String title = getArguments().getString("title");
            String message = getArguments().getString("message");
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle(title);
            builder.setMessage(message);
            AlertDialog dlg = builder.create();
            dlg.setCanceledOnTouchOutside(true);
            return dlg;
        }
    }
}
