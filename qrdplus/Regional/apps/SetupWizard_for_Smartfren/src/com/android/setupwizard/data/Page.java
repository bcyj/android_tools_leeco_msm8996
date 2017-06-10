/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.data;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;


public abstract class Page implements PageNode {

    public static final String KEY_PAGE_ARGUMENT = "key_arg";

    private SetupDataCallbacks mCallbacks;

    private Bundle mData = new Bundle();
    private String mTitle;
    private int mTitleResourceId;
    private boolean mRequired = false;
    private boolean mCompleted = false;

    protected Page(Context context, SetupDataCallbacks callbacks, int titleResourceId) {
        mCallbacks = callbacks;
        mTitleResourceId = titleResourceId;
        mTitle = context.getString(mTitleResourceId);
    }

    public Bundle getData() {
        return mData;
    }

    public String getTitle() {
        return mTitle;
    }

    public boolean isRequired() {
        return mRequired;
    }

    @Override
    public Page findPage(String key) {
        return getKey().equals(key) ? this : null;
    }

    @Override
    public Page findPage(int id) {
        return getId() == id ? this : null;
    }

    public abstract Fragment createFragment();

    public abstract int getNextButtonResId();

    public int getId() {
        return mTitleResourceId;
    }

    public String getKey() {
        return mTitle;
    }

    public boolean isCompleted() {
        return mCompleted;
    }

    public void setCompleted(boolean completed) {
        mCompleted = completed;
    }

    public void resetData(Bundle data) {
        mData = data;
        notifyDataChanged();
    }

    public void notifyDataChanged() {
        mCallbacks.onPageLoaded(this);
    }

    public Page setRequired(boolean required) {
        mRequired = required;
        return this;
    }

}
