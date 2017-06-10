/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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
import android.app.TabActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TabHost;
import android.widget.TextView;
import android.content.Intent;
import android.content.ComponentName;
import com.android.firewall.R;
import android.provider.CallLog.Calls;
import android.util.DisplayMetrics;
import android.widget.TabWidget;
import android.util.Log;

public class FirewallActivity extends TabActivity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        TabHost tabHost = getTabHost();

        Intent settingIntent = new Intent(this, FirewallSettingPage.class);
        Intent blacklistIntent = new Intent(this, FirewallListPage.class);
        blacklistIntent.putExtra("mode", "blacklist");
        Intent whitelistIntent = new Intent(this, FirewallListPage.class);
        whitelistIntent.putExtra("mode", "whitelist");

        Intent intent = getIntent();
        String action = intent.getAction();
        Bundle bundle = intent.getExtras();
        if (action != null && action.equals(Intent.ACTION_INSERT)) {
            if (bundle.getString("mode").equals("blacklist")) {
                blacklistIntent.setAction(Intent.ACTION_INSERT);
            } else {
                whitelistIntent.setAction(Intent.ACTION_INSERT);
            }
        }

        tabHost.addTab(tabHost.newTabSpec("blacklist")
                .setIndicator(getText(R.string.firewall_blacklist_title))
                .setContent(blacklistIntent));
        tabHost.addTab(tabHost.newTabSpec("whitelist")
                .setIndicator(getText(R.string.firewall_whitelist_title))
                .setContent(whitelistIntent));
        tabHost.addTab(tabHost.newTabSpec("setting")
                .setIndicator(getText(R.string.firewall_setting))
                .setContent(settingIntent));
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);

        int screenWidth = dm.widthPixels;

        for (int i = 0; i < 3; i++) {
            tabHost.getTabWidget().getChildAt(i).getLayoutParams().width = (screenWidth / 3);

            TextView tv = (TextView) tabHost.getTabWidget().getChildAt(i)
                    .findViewById(android.R.id.title);

            tv.setTextSize(13);
            tv.setSingleLine(true);
        }

        if (action != null && action.equals(Intent.ACTION_INSERT)) {
            if (bundle.getString("mode").equals("blacklist")) {
                tabHost.setCurrentTab(1);
                tabHost.getTabWidget().getChildAt(0).setEnabled(false);
                tabHost.getTabWidget().getChildAt(2).setEnabled(false);
            } else {
                tabHost.setCurrentTab(2);
                tabHost.getTabWidget().getChildAt(0).setEnabled(false);
                tabHost.getTabWidget().getChildAt(1).setEnabled(false);
            }
        }
    }
}
