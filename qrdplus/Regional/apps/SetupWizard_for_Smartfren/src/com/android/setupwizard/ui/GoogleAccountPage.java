/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.ui;

import com.android.setupwizard.R;
import com.android.setupwizard.activity.SetupWizardActivity;
import com.android.setupwizard.data.Page;
import com.android.setupwizard.data.SetupDataCallbacks;
import com.android.setupwizard.util.SetupUtils;

import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;


public class GoogleAccountPage extends Page {

    public GoogleAccountPage(Context context, SetupDataCallbacks callbacks, int titleResourceId) {
        super(context, callbacks, titleResourceId);
    }

    @Override
    public Fragment createFragment() {
        Bundle args = new Bundle();
        args.putString(Page.KEY_PAGE_ARGUMENT, getKey());

        GoogleAccountFragment fragment = new GoogleAccountFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public int getNextButtonResId() {
        return R.string.skip;
    }

    public static class GoogleAccountFragment extends SetupPageFragment {

        @Override
        protected void setUpPage() {
            TextView summaryView = (TextView) mRootView.findViewById(android.R.id.summary);
            summaryView.setText(R.string.google_account_summary);
            Button mButton = (Button) mRootView.findViewById(R.id.google_button);
            mButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    ((SetupWizardActivity) getActivity()).launchGoogleAccountSetup();
                }
            });
        }

        @Override
        protected int getLayoutResource() {
            return R.layout.setup_google_account_page;
        }

        @Override
        protected int getTitleResource() {
            return R.string.setup_google_account;
        }
    }
}
