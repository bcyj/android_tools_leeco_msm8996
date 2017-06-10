/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.database.DataSetObserver;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.view.View;
import android.widget.LinearLayout.LayoutParams;
import android.widget.ListAdapter;
import android.widget.ListView;

public abstract class RadioPreferenceFragment extends PreferenceFragment {
    protected abstract void onSelectedChanged();
    protected abstract void handleMessage(Message msg);

    private static final String RADIO_GROUP_CATEGORY = "radio_group_category";

    protected RadioGroupPreferenceCategory mCategory;

    protected String mSelectedPreferenceKey = null;
    protected String mCurrentPreferenceKey = null;

    private static final int MSG_RESET = -1;
    protected static final int MSG_BASE = 0;

    protected Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_RESET:
                    // Reset the selected SIM mode value.
                    mSelectedPreferenceKey = mCurrentPreferenceKey;
                    if (mCategory != null) {
                        // Update the preferences.
                        mCategory.setCheckedPreference(mSelectedPreferenceKey);
                    }
                    break;
                default:
                    if (msg.what > MSG_BASE) {
                        RadioPreferenceFragment.this.handleMessage(msg);
                    } else {
                        throw new IllegalArgumentException("Do not accept this message : " + msg);
                    }
                    break;
            }
        }
    };

    // To observer the adapter's data change, if the changed, need update the list height.
    private DataSetObserver mObserver = new DataSetObserver() {
        @Override
        public void onChanged() {
            super.onChanged();
            // Update the list height.
            updateListHeight();
        }
    };

    public RadioPreferenceFragment() {
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        // As this fragment will be added to the activity, and we will handle
        // the scroll action in the activity.
        ListView list = getListView();
        list.setDivider(getResources().getDrawable(android.R.drawable.divider_horizontal_dark));
        list.setScrollContainer(false);
        // Register the observer for adapter changed.
        list.getAdapter().registerDataSetObserver(mObserver);
        // Update the list height.
        updateListHeight();

        mCategory = (RadioGroupPreferenceCategory) findPreference(RADIO_GROUP_CATEGORY);
    }

    @Override
    public void onDestroyView() {
        // Before destroy the view, un-register the observer.
        if (mObserver != null) {
            getListView().getAdapter().unregisterDataSetObserver(mObserver);
        }
        super.onDestroyView();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        mSelectedPreferenceKey = preference.getKey();

        if (mCategory != null) {
            // Update the preferences.
            mCategory.setCheckedPreference(mSelectedPreferenceKey);
        }

        // We need handle the selected changed action.
        if (!mSelectedPreferenceKey.equalsIgnoreCase(mCurrentPreferenceKey)) {
            onSelectedChanged();
        }

        return true;
    }

    public void resetSelection() {
        mHandler.sendEmptyMessage(MSG_RESET);
    }

    public void sendEmptyMessage(int what) {
        mHandler.sendEmptyMessage(what);
    }

    public void sendMessage(Message msg) {
        mHandler.sendMessage(msg);
    }

    private void updateListHeight() {
        ListView list = getListView();
        ListAdapter adapter = list.getAdapter();

        int newHeight = 0;
        for (int i = 0; i < adapter.getCount(); i++) {
            View itemView = adapter.getView(i, null, list);
            itemView.measure(0, 0);
            newHeight = newHeight + itemView.getMeasuredHeight();
        }
        newHeight = newHeight + (list.getDividerHeight() * (adapter.getCount() - 1));
        list.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, newHeight));
    }
}
