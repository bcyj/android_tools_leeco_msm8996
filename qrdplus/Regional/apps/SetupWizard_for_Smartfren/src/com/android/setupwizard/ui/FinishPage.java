/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.ui;

import com.android.setupwizard.R;
import com.android.setupwizard.ui.SetupPageFragment;
import com.android.setupwizard.data.Page;
import com.android.setupwizard.data.SetupDataCallbacks;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;

public class FinishPage extends Page {

    public FinishPage(Context context, SetupDataCallbacks callbacks, int titleResourceId) {
        super(context, callbacks, titleResourceId);
    }

    @Override
    public Fragment createFragment() {
        Bundle args = new Bundle();
        args.putString(Page.KEY_PAGE_ARGUMENT, getKey());

        FinishFragment fragment = new FinishFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public int getNextButtonResId() {
        return R.string.finish;
    }


    public static class FinishFragment extends SetupPageFragment {

        @Override
        protected void setUpPage() {}

        @Override
        protected int getLayoutResource() {
            return R.layout.setup_finished_page;
        }

        @Override
        protected int getTitleResource() {
            return R.string.setup_complete;
        }
    }

}
