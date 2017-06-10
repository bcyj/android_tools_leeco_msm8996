/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.firewall;
import android.os.Bundle;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.TabActivity;
import android.content.Context;
import android.content.Intent;
import android.text.TextUtils;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.Toast;
import android.util.DisplayMetrics;

@SuppressWarnings("deprecation")
public class BlockRecordTabActivity extends TabActivity implements OnTabChangeListener{
    public static final String NUMBER_KEY = "number_key";

    private TabHost mTabHost;
    private String number;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.block_record_tab);
        number = getIntent().getStringExtra(NUMBER_KEY);
        if (TextUtils.isEmpty(number)) {
            Toast.makeText(this, R.string.number_is_empty, Toast.LENGTH_LONG).show();
            finish();
        }
        initActionBar();
        mTabHost = getTabHost();
        setupBlockCallTab();
        setupBlockMmsTab();
        mTabHost.setOnTabChangedListener(this);
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);

        int screenWidth = dm.widthPixels;

        for (int i = 0; i < 2; i++) {
            mTabHost.getTabWidget().getChildAt(i).getLayoutParams().width = (screenWidth / 2);

            TextView tv = (TextView) mTabHost.getTabWidget().getChildAt(i)
                    .findViewById(android.R.id.title);

            tv.setTextSize(13);
            tv.setSingleLine(true);
        }
        setCurrentTab();
    }

    @SuppressLint("NewApi")
    private void initActionBar() {
        getActionBar().setDisplayShowHomeEnabled(false);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        getActionBar().setTitle(number);
    }

    private void setupBlockCallTab(){
        // Force the class since overriding tab entries doesn't work
        Intent intent = new Intent("com.android.firewall.action.BLOCK_CALLS");
        intent.putExtra(NUMBER_KEY,number);
        intent.setClass(this, BlockCalllogListActivity.class);
        mTabHost.addTab(mTabHost.newTabSpec("call_log")
                .setIndicator(getString(R.string.call_block))
                .setContent(intent));
    }

    private void setupBlockMmsTab(){
        Intent intent = new Intent("com.android.firewall.action.BLOCK_MESSAGES");
        intent.putExtra(NUMBER_KEY,number);
        intent.setClass(this, BlockMessageListActivity.class);
        mTabHost.addTab(mTabHost.newTabSpec("message")
                .setIndicator(getString(R.string.message_block))
                .setContent(intent));
    }

    /**
     * Sets the current tab based on the intent's request type
     *
     * @param intent Intent that contains information about which tab should be selected
     */
    private void setCurrentTab() {
        // Dismiss menu provided by any children activities
        Activity activity = getLocalActivityManager().
                getActivity(mTabHost.getCurrentTabTag());
        if (activity != null) {
            activity.closeOptionsMenu();
        }
        mTabHost.setCurrentTab(0);

    }

    @Override
    public void onTabChanged(String tabId) {
        // TODO Auto-generated method stub
        Activity activity = getLocalActivityManager().getActivity(tabId);
        if (activity != null) {
            activity.onWindowFocusChanged(true);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                break;
        }
        return super.onOptionsItemSelected(item);
    }
}
