/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess.appsettings;

import static com.qapp.secprotect.utils.UtilsLog.logd;

import java.util.List;

import com.qapp.secprotect.R;

import android.os.Bundle;
import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.Loader;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

public class AppConfigFragment extends ListFragment implements
        LoaderManager.LoaderCallbacks<List<AppRecord>> {

    AppListAdapter mAppListAdapter;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return super.onCreateView(inflater, container, savedInstanceState);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
    
    @Override
    public void onResume() {
        super.onResume();
    }
    
    @Override
    public void onPause() {
        super.onPause();
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
    }
    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        setEmptyText(getString(R.string.no_record));
        mAppListAdapter = new AppListAdapter(getActivity());
        setListAdapter(mAppListAdapter);

        setListShown(false);
        getLoaderManager().initLoader(0, null, this);
    }

    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        logd(id);
    }

    @Override
    public Loader<List<AppRecord>> onCreateLoader(int id, Bundle args) {
        logd();
        return new AppListLoader(getActivity());
    }

    @Override
    public void onLoadFinished(Loader<List<AppRecord>> loader, List<AppRecord> data) {
        logd();
        mAppListAdapter.setData(data);

        if (isResumed()) {
            setListShown(true);
        } else {
            setListShownNoAnimation(true);
        }
    }

    @Override
    public void onLoaderReset(Loader<List<AppRecord>> loader) {
        logd();
        mAppListAdapter.setData(null);
    }
}
