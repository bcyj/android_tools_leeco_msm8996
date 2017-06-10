/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.ui;

import com.android.setupwizard.R;
import com.android.setupwizard.data.Page;
import com.android.setupwizard.data.SetupDataCallbacks;

import android.app.Fragment;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.Handler;
import android.widget.ArrayAdapter;
import android.widget.NumberPicker;
import android.widget.TextView;

import java.util.Locale;

public class WelcomePage extends Page {

    public WelcomePage(Context context, SetupDataCallbacks callbacks, int titleResourceId) {
        super(context, callbacks, titleResourceId);
    }

    @Override
    public Fragment createFragment() {
        Bundle args = new Bundle();
        args.putString(Page.KEY_PAGE_ARGUMENT, getKey());

        WelcomeFragment fragment = new WelcomeFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public int getNextButtonResId() {
        return R.string.next;
    }

    public static class WelcomeFragment extends SetupPageFragment {

        private ArrayAdapter<com.android.internal.app.LocalePicker.LocaleInfo> mLocaleAdapter;
        private Locale mInitialLocale;
        private Locale mCurrentLocale;
        private int[] mAdapterIndices;

        private LocalePicker mLanguagePicker;

        private final Handler mHandler = new Handler();

        private final Runnable mUpdateLocale = new Runnable() {
            public void run() {
                if (mCurrentLocale != null) {
                    com.android.internal.app.LocalePicker.updateLocale(mCurrentLocale);
                }
            }
        };

        @Override
        protected void setUpPage() {
            mLanguagePicker = (LocalePicker) mRootView.findViewById(R.id.locale_list);
            loadLanguages();
        }

        private void loadLanguages() {
            mLocaleAdapter = com.android.internal.app.LocalePicker.constructAdapter(getActivity(), R.layout.locale_picker_item, R.id.locale);
            mInitialLocale = Locale.getDefault();
            mCurrentLocale = mInitialLocale;
            mAdapterIndices = new int[mLocaleAdapter.getCount()];
            int currentLocaleIndex = 0;
            String [] labels = new String[mLocaleAdapter.getCount()];
            for (int i=0; i<mAdapterIndices.length; i++) {
                com.android.internal.app.LocalePicker.LocaleInfo localLocaleInfo = mLocaleAdapter.getItem(i);
                Locale localLocale = localLocaleInfo.getLocale();
                if (localLocale.equals(mCurrentLocale)) {
                    currentLocaleIndex = i;
                }
                mAdapterIndices[i] = i;
                labels[i] = localLocaleInfo.getLabel();
            }
            mLanguagePicker.setDisplayedValues(labels);
            mLanguagePicker.setMaxValue(labels.length - 1);
            mLanguagePicker.setValue(currentLocaleIndex);
            mLanguagePicker.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
            mLanguagePicker.setOnValueChangedListener(new LocalePicker.OnValueChangeListener() {
                public void onValueChange(LocalePicker picker, int oldVal, int newVal) {
                    setLocaleFromPicker();
                }
            });
        }

        private void setLocaleFromPicker() {
            int i = mAdapterIndices[mLanguagePicker.getValue()];
            final com.android.internal.app.LocalePicker.LocaleInfo localLocaleInfo = mLocaleAdapter.getItem(i);
            onLocaleChanged(localLocaleInfo.getLocale());
        }

        private void onLocaleChanged(Locale paramLocale) {
            Resources localResources = getActivity().getResources();
            Configuration localConfiguration1 = localResources.getConfiguration();
            Configuration localConfiguration2 = new Configuration();
            localConfiguration2.locale = paramLocale;
            localResources.updateConfiguration(localConfiguration2, null);
            localResources.updateConfiguration(localConfiguration1, null);
            TextView titleView = (TextView) mRootView.findViewById(android.R.id.title);
            titleView.setText(getTitleResource());
            mHandler.removeCallbacks(mUpdateLocale);
            mCurrentLocale = paramLocale;
            mHandler.postDelayed(mUpdateLocale, 1000);
        }

        @Override
        protected int getLayoutResource() {
            return R.layout.setup_welcome_page;
        }

        @Override
        protected int getTitleResource() {
            return R.string.setup_welcome;
        }
    }

}
