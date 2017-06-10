/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.setupwizard.ui;

import com.android.setupwizard.R;
import com.android.setupwizard.ui.SetupPageFragment;
import com.android.setupwizard.util.SetupUtils;
import com.android.setupwizard.data.Page;
import com.android.setupwizard.data.SetupDataCallbacks;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class NetworkPage extends Page {

    public NetworkPage(Context context, SetupDataCallbacks callbacks, int titleResourceId) {
        super(context, callbacks, titleResourceId);
    }

    @Override
    public Fragment createFragment() {
        Bundle args = new Bundle();
        args.putString(Page.KEY_PAGE_ARGUMENT, getKey());

        NetworkFragment fragment = new NetworkFragment();
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public int getNextButtonResId() {
        return R.string.skip;
    }


    public static class NetworkFragment extends SetupPageFragment {

        @Override
        public void onActivityResult(int requestCode, int resultCode, Intent data) {
            if (requestCode == SetupUtils.REQUEST_CODE_SETUP_WIFI) {
                if (resultCode == Activity.RESULT_OK) {
                    ((Button)mRootView.findViewById(R.id.wifi_button)).setText(R.string.btn_disconnect_wifi);
                }
            }
        }

        @Override
        protected void setUpPage() {
            TextView summaryView = (TextView) mRootView.findViewById(android.R.id.summary);
            summaryView.setText(R.string.setup_network_summary);
            final Button wifiButton = (Button)mRootView.findViewById(R.id.wifi_button);
            refreshButtonText(wifiButton);
            wifiButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (SetupUtils.isWifiConnected(getActivity())) {
                        SetupUtils.tryDisenablingWifi(getActivity());
                        wifiButton.setText(R.string.btn_connect_wifi);
                    } else {
                        SetupUtils.launchWifiSetup(NetworkFragment.this);
                    }
                }
            });
            final Button dataButton = (Button)mRootView.findViewById(R.id.mobile_data_button);
            if (SetupUtils.isMobileDataConnected(getActivity())) {
                dataButton.setText(R.string.btn_disconnect_mobile_data);
            } else {
                dataButton.setText(R.string.btn_connect_mobile_data);
            }
            if (!SetupUtils.isSimReady()) {
                dataButton.setEnabled(false);
            }
            dataButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (SetupUtils.isMobileDataConnected(getActivity())) {
                        SetupUtils.tryDisEnablingMobileData(getActivity());
                        dataButton.setText(R.string.btn_connect_mobile_data);
                    } else {
                        SetupUtils.tryEnablingMobileData(getActivity());
                        dataButton.setText(R.string.btn_disconnect_mobile_data);
                    }
                }
            });
        }

        protected void refreshButtonText(Button button) {
            if (SetupUtils.isWifiConnected(getActivity())) {
                button.setText(R.string.btn_disconnect_wifi);
            } else {
                button.setText(R.string.btn_connect_wifi);
            }
        }
        @Override
        protected int getLayoutResource() {
            return R.layout.setup_network_page;
        }

        @Override
        protected int getTitleResource() {
            return R.string.setup_network;
        }
    }

}
