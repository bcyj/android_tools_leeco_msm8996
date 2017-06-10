/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.carrierconfigure;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.Window;

import java.util.ArrayList;

public class ConfigurationActivity extends Activity {
    private static final String KEY_NO_TITILE = "no_title";
    private static final String KEY_VISIBLE_CONTAINER_ID = "visible_container_id";

    private boolean mNoTitle = false;
    private int mVisibleContainerId = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (savedInstanceState == null) {
            initFragment();
        } else {
            mNoTitle = savedInstanceState.getBoolean(KEY_NO_TITILE, false);
            mVisibleContainerId = savedInstanceState.getInt(KEY_VISIBLE_CONTAINER_ID, -1);
        }

        if (mVisibleContainerId < 0) {
            finish();
            return;
        }

        if (mNoTitle) {
            requestWindowFeature(Window.FEATURE_NO_TITLE);
        }

        setContentView(R.layout.configuration_activity);
        findViewById(mVisibleContainerId).setVisibility(View.VISIBLE);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putBoolean(KEY_NO_TITILE, mNoTitle);
        outState.putInt(KEY_VISIBLE_CONTAINER_ID, mVisibleContainerId);
        super.onSaveInstanceState(outState);
    }

    private void initFragment() {
        String action = getIntent().getAction();
        if (Intent.ACTION_MAIN.equals(action)) {
            if (getIntent().hasCategory(StartActivityReceiver.CODE_SPEC_SWITCH_L)) {
                getFragmentManager().beginTransaction()
                        .add(R.id.first, new MultiSimConfigFragmentL())
                        .commit();
                getFragmentManager().beginTransaction()
                        .add(R.id.second, new CarrierConfigFragmentL())
                        .commit();
            } else {
                getFragmentManager().beginTransaction()
                        .add(R.id.first, new MultiSimConfigFragment())
                        .commit();
                getFragmentManager().beginTransaction()
                        .add(R.id.second, new CarrierConfigFragment())
                        .commit();
            }

            mNoTitle = false;
            mVisibleContainerId = R.id.two_fragment;
        } else {
            ArrayList<Carrier> list =
                    getIntent().getParcelableArrayListExtra(Utils.EXTRA_CARRIER_LIST);
            if (list == null || list.size() < 1) return;

            if (Utils.ACTION_TRIGGER_WELCOME.equals(action)) {
                getFragmentManager().beginTransaction()
                        .add(R.id.one_fragment, TriggerWelcomeFragment.newInstance(list))
                        .commit();
            } else if (Utils.ACTION_TRIGGER_START.equals(action)) {
                getFragmentManager().beginTransaction()
                        .add(R.id.one_fragment, TriggerStartFragment.newInstance(list))
                        .commit();
            }

            mNoTitle = true;
            mVisibleContainerId = R.id.one_fragment;
        }
    }

}
