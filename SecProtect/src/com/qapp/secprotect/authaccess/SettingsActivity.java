/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess;

import static com.qapp.secprotect.utils.UtilsLog.logd;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTransaction;

import android.support.v4.app.FragmentTabHost;
import android.view.KeyEvent;
import android.view.View;

import com.qapp.secprotect.R;
import com.qapp.secprotect.authaccess.appsettings.AppConfigFragment;
import com.qapp.secprotect.authaccess.protectops.ProtectOpsFragment;
import com.qapp.secprotect.explorer.FileExplorerFragment;

public class SettingsActivity extends FragmentActivity {
    private FragmentTabHost mTabHost;
    FileExplorerFragment mFileExplorerFragment;
    ProtectOpsFragment mSettingsFragment;

    void init() {

        logd("");
        getActionBar().setDisplayShowHomeEnabled(false);
        setContentView(R.layout.authaccess_settings_activity);

        mTabHost = (FragmentTabHost) findViewById(android.R.id.tabhost);
        mTabHost.setup(this, getSupportFragmentManager(), R.id.realtabcontent);

        mTabHost.addTab(
                mTabHost.newTabSpec("protect").setIndicator(
                        getString(R.string.protect)), ProtectOpsFragment.class,
                null);
        mTabHost.addTab(
                mTabHost.newTabSpec("app_config").setIndicator(
                        getString(R.string.app_config)),
                AppConfigFragment.class, null);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        init();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        logd("");
        boolean isRootPath = false;
        FileExplorerFragment fileExplorerFragment = (FileExplorerFragment) getSupportFragmentManager()
                .findFragmentByTag("mFileExplorerFragment");
        if (fileExplorerFragment != null) {
            // FileExplorer is on root directory
            isRootPath = fileExplorerFragment.onKeyDown(keyCode, event);
            if (!isRootPath) {
                return true;
            }
        }
        if (isRootPath) {
            FragmentTransaction transaction = getSupportFragmentManager()
                    .beginTransaction();
            transaction.remove(fileExplorerFragment);
            transaction.commit();
            findViewById(R.id.configs_tabs).setVisibility(View.VISIBLE);
            logd("Go back to TabHosts");
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onAttachFragment(Fragment fragment) {
        logd("");
        super.onAttachFragment(fragment);
        if (fragment instanceof FileExplorerFragment) {
            mFileExplorerFragment = (FileExplorerFragment) fragment;
        } else if (fragment instanceof ProtectOpsFragment) {
            mSettingsFragment = (ProtectOpsFragment) fragment;
        }
    }
}
