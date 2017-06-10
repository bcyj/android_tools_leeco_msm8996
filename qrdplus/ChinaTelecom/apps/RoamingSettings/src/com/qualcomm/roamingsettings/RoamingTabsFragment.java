/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.roamingsettings;

import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TabHost;
import android.widget.TabHost.TabSpec;
import android.widget.TabWidget;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;

import java.util.HashMap;

public class RoamingTabsFragment extends Fragment {
    private static final String TAG = "RoamingSettingsApp";

    private final static String SUB = PhoneConstants.SUBSCRIPTION_KEY;
    private final static String COMMA = ",";

    private View mViewRoot;

    private TabHost mTabHost;
    private TabManager mTabManager;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mViewRoot = inflater.inflate(R.layout.roaming_tabs,
                container, false);
        mTabHost = (TabHost) mViewRoot.findViewById(android.R.id.tabhost);
        mTabHost.setup();
        mTabManager = new TabManager(this, mTabHost, android.R.id.tabcontent);

        int phoneCount = TelephonyManager.getDefault().getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            String tag = SUB + COMMA + i;
            // Add the tab to TabHost.
            String slotNumber = getResources().getStringArray(
                    R.array.slot_number)[i];
            String tabName = getString(R.string.slot_name, slotNumber);
            mTabManager.addTab(mTabHost.newTabSpec(tag).setIndicator(tabName), i);

            // If there is only one sub, we needn't to show the tab.
            if (phoneCount == PhoneConstants.MAX_PHONE_COUNT_SINGLE_SIM) {
                TabWidget tab = (TabWidget) mViewRoot
                        .findViewById(android.R.id.tabs);
                tab.setVisibility(View.GONE);
            }
        }

        return mViewRoot;
    }

    public static class TabManager implements TabHost.OnTabChangeListener {
        private final RoamingTabsFragment mFragment;
        private RoamingSettingsFragmet mSettingsFragment;
        private final TabHost mTabHost;
        private final int mResContainerId;
        private final HashMap<String, TabInfo> mTabs = new HashMap<String, TabInfo>();

        private TabInfo mLastTab;

        public TabManager(RoamingTabsFragment fragment, TabHost tabHost,
                int resContainerId) {
            mFragment = fragment;
            mTabHost = tabHost;
            mResContainerId = resContainerId;

            // set the tab changed listener.
            mTabHost.setOnTabChangedListener(this);
        }

        @Override
        public void onTabChanged(String tabId) {
            TabInfo newTab = mTabs.get(tabId);
            if (mLastTab != newTab) {
                mSettingsFragment.onSubscriptionChanged(newTab.sub);
            }
            mLastTab = newTab;
        }

        public void addTab(TabSpec tabSpec, int sub) {
            tabSpec.setContent(new MyTabFactory(mFragment.getActivity()));
            String tag = tabSpec.getTag();

            TabInfo info = new TabInfo(tag, sub);
            if (mSettingsFragment == null) {
                FragmentTransaction ft = mFragment.getFragmentManager()
                        .beginTransaction();
                mSettingsFragment = (RoamingSettingsFragmet) Fragment
                        .instantiate(mFragment.getActivity(),
                                RoamingSettingsFragmet.class.getName(), null);
                ft.add(mResContainerId, mSettingsFragment, tag);
                ft.commit();
            }

            mTabs.put(tag, info);
            mTabHost.addTab(tabSpec);
        }

        static class MyTabFactory implements TabHost.TabContentFactory {
            private final Context mContext;

            public MyTabFactory(Context context) {
                mContext = context;
            }

            @Override
            public View createTabContent(String tag) {
                View v = new View(mContext);
                v.setMinimumWidth(0);
                v.setMinimumHeight(0);
                return v;
            }

        }

        static class TabInfo {
            private final String tag;
            private final int sub;

            TabInfo(String _tag, int _sub) {
                tag = _tag;
                sub = _sub;
            }
        }
    }

}
