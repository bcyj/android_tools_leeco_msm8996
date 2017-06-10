/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.data;

import android.content.Context;
import android.os.Bundle;

import java.util.ArrayList;

import com.android.setupwizard.R;
import com.android.setupwizard.ui.FinishPage;
import com.android.setupwizard.ui.GoogleAccountPage;
import com.android.setupwizard.ui.NetworkPage;
import com.android.setupwizard.ui.WelcomePage;

public class AbstractSetupData implements SetupDataCallbacks {

    private static final String TAG = AbstractSetupData.class.getSimpleName();

    protected Context mContext;
    private ArrayList<SetupDataCallbacks> mListeners = new ArrayList<SetupDataCallbacks>();
    private PageList mPageList;

    public AbstractSetupData(Context context) {
        mContext = context;
        mPageList = onNewPageList();
    }

    protected PageList onNewPageList() {
        return new PageList(new WelcomePage(mContext, this, R.string.setup_welcome),
                new NetworkPage(mContext, this, R.string.setup_network),
                new GoogleAccountPage(mContext, this, R.string.setup_google_account),
                new FinishPage(mContext, this, R.string.setup_complete)
         );
    }

    @Override
    public void onPageLoaded(Page page) {
        for (int i = 0; i < mListeners.size(); i++) {
            mListeners.get(i).onPageLoaded(page);
        }
    }

    @Override
    public void onPageTreeChanged() {
        for (int i = 0; i < mListeners.size(); i++) {
            mListeners.get(i).onPageTreeChanged();
        }
    }

    @Override
    public void onPageFinished(Page page) {
        for (int i = 0; i < mListeners.size(); i++) {
            mListeners.get(i).onPageFinished(page);
        }
    }

    @Override
    public Page getPage(String key) {
        return findPage(key);
    }

    @Override
    public Page getPage(int key) {
        return findPage(key);
    }

    public Page findPage(String key) {
        return mPageList.findPage(key);
    }

    public Page findPage(int key) {
        return mPageList.findPage(key);
    }

    public void load(Bundle savedValues) {
        for (String key : savedValues.keySet()) {
            mPageList.findPage(key).resetData(savedValues.getBundle(key));
        }
    }

    public Bundle save() {
        Bundle bundle = new Bundle();
        for (Page page : getPageList()) {
            bundle.putBundle(page.getKey(), page.getData());
        }
        return bundle;
    }

    public void addPage(int index, Page page) {
        mPageList.add(index, page);
        onPageTreeChanged();
    }

    public void removePage(Page page) {
        mPageList.remove(page);
        onPageTreeChanged();
    }

    public void registerListener(SetupDataCallbacks listener) {
        mListeners.add(listener);
    }

    public PageList getPageList() {
        return mPageList;
    }

    public void unregisterListener(SetupDataCallbacks listener) {
        mListeners.remove(listener);
    }
}
