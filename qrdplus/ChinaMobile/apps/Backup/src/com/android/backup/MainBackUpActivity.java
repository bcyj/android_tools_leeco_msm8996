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

package com.android.backup;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import com.android.backup.BackupUtils;
import com.android.backup.R;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.TabActivity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.GestureDetector;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.Window;
import android.widget.TabHost;
import android.util.Log;

public class MainBackUpActivity extends TabActivity implements TabHost.OnTabChangeListener {

    private static final String TAG = "MainBackUpActivity";
    private static final boolean LOCAL_DEBUG = true;
    private TabHost mTabHost;
    long startMillis;

    private static final int TAB_INDEX_BACKUP = 0;
    private static final int TAB_INDEX_RECOVER = 1;

    private static final String BACKUP_PREFERENCE_NAME = "Backup_Preference";
    private static final String BACKUP_FIRST_TIME_KEY = "BackupFirstTimeKey";
    private static final String BACKUP_TAG_PATH = "/backup/App/backuptag";
    private static final String RESTORE_TAG_PATH = "/backup/App/restoretag";

    private static final int PATH_INTERNAL = 0;
    private static final int PATH_EXTERNAL = 1;
    private static final int PATH_FAKE = -1;

    private static final int SWIPE_MIN_DISTANCE = 120;
    private static final int SWIPE_MAX_OFF_PATH = 250;
    private static final int SWIPE_THRESHOLD_VELOCITY = 200;
    private static final int BACKUP_TAB = 0;
    private static final int RESTORE_TAB = 1;
    private int currentView = BACKUP_TAB;
    private static int maxTabIndex = 2;
    private GestureDetector mGestureDetector;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setTitle(R.string.backup_label);

        final Intent intent = getIntent();

        mTabHost = getTabHost();
        mTabHost.setOnTabChangedListener(this);

        // Set actionbar style
        this.getActionBar().setDisplayShowHomeEnabled(false);
        this.getActionBar().setTitle(R.string.backup_label);

        // Setup the tabs
        setupBackupTab();
        setupRecoverTab();
        installTool("busybox");
        installTool("backupAppData.sh");
        installTool("recoverAppData.sh");

        firstTimeDialog();
        initLocation();
        mGestureDetector = new GestureDetector(new MainGestureDetector());
    }

    class MainGestureDetector extends SimpleOnGestureListener {
        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                float velocityY) {
            TabHost tabHost = getTabHost();
            try {
                if (Math.abs(e1.getY() - e2.getY()) > SWIPE_MAX_OFF_PATH)
                    return false;
                // right to left swipe
                if (e1.getX() - e2.getX() > SWIPE_MIN_DISTANCE
                        && Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
                    Log.i(TAG, "to left, currentView: " + currentView);
                    if (currentView == RESTORE_TAB) {
                        mTabHost.setCurrentTab(BACKUP_TAB);
                        currentView = BACKUP_TAB;
                    }
                } else if (e2.getX() - e1.getX() > SWIPE_MIN_DISTANCE
                        && Math.abs(velocityX) > SWIPE_THRESHOLD_VELOCITY) {
                    Log.i(TAG, "to right, currentView: " + currentView);
                    if(currentView == BACKUP_TAB) {
                        mTabHost.setCurrentTab(RESTORE_TAB);
                        currentView = RESTORE_TAB;
                    }
                }
            } catch (Exception e) {
            }
            return false;
        }
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        Log.d(TAG, "dispatchTouchEvent()");
        if (mGestureDetector.onTouchEvent(event)) {
            event.setAction(MotionEvent.ACTION_CANCEL);
        }
        return super.dispatchTouchEvent(event);
    }

    private void initLocation() {
        SharedPreferences sharedPreferences = getSharedPreferences(
                BackupUtils.LOCATION_PREFERENCE_NAME, Context.MODE_PRIVATE);
        if (PATH_FAKE == sharedPreferences.getInt(BackupUtils.KEY_BACKUP_LOCATION, PATH_FAKE)) {
            Log.d("SettingsActivity", "set backup location: " +
            sharedPreferences.getInt(BackupUtils.KEY_BACKUP_LOCATION, PATH_FAKE));
            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putInt(BackupUtils.KEY_BACKUP_LOCATION, PATH_INTERNAL);
            editor.commit();
            String dirString = System.getenv("SECONDARY_STORAGE") + BACKUP_TAG_PATH;
            File path=new File(dirString);
            if(path.exists())
               path.delete();
        }
        if (PATH_FAKE == sharedPreferences.getInt(BackupUtils.KEY_RESTORE_LOCATION, PATH_FAKE)) {
            Log.d("SettingsActivity", "set restore location: " +
            sharedPreferences.getInt(BackupUtils.KEY_RESTORE_LOCATION, PATH_FAKE));
            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putInt(BackupUtils.KEY_RESTORE_LOCATION, PATH_INTERNAL);
            editor.commit();
            String dirString = System.getenv("SECONDARY_STORAGE") + RESTORE_TAG_PATH;
            File path=new File(dirString);
            if(path.exists())
               path.delete();
        }
    }

    private void firstTimeDialog() {
        final SharedPreferences sharedPreferences = getSharedPreferences(BACKUP_PREFERENCE_NAME,
                Context.MODE_PRIVATE);
        if (sharedPreferences.getBoolean(BACKUP_FIRST_TIME_KEY, true)) {
            new AlertDialog.Builder(MainBackUpActivity.this,
                    android.R.style.Theme_DeviceDefault_Dialog_NoActionBar)
                    .setTitle(R.string.backup_first_time_title)
                    .setMessage(R.string.backup_first_time_msg)
                    .setOnDismissListener(new DialogInterface.OnDismissListener() {
                        @Override
                        public void onDismiss(DialogInterface dialog) {
                            SharedPreferences.Editor editor = sharedPreferences.edit();
                            editor.putBoolean(BACKUP_FIRST_TIME_KEY, false);
                            editor.commit();
                        }
                    }).setPositiveButton(R.string.button_ok, null).show();
        }
    }

    private void installTool(String tool)
    {
        InputStream is = null;
        FileOutputStream os = null;
        try {
            is = getAssets().open(tool);
            File out = new File(getCacheDir(), tool);
            if (!out.exists()) {
                File fd = new File(getCacheDir(), tool + "_bak");
                if (fd.exists())
                    fd.delete();
                if (fd.createNewFile()) {
                    os = new FileOutputStream(fd);
                    byte[] buffer = new byte[1024 * 256];
                    while (true) {
                        int rn = is.read(buffer);
                        if (-1 == rn)
                            break;
                        os.write(buffer, 0, rn);
                        os.flush();
                    }
                    os.close();
                    os = null;
                }
                fd.renameTo(out);
            }
            is.close();
        } catch (Exception e) {
            e.printStackTrace();
            if (is != null) {
                try {
                    is.close();
                } catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
            if (os != null) {
                try {
                    os.close();
                } catch (Exception ex) {
                    ex.printStackTrace();
                }
            }
        }
    }

    private void setupBackupTab()
    {
        Intent intent = new Intent("android.intent.action.BACKUP");
        intent.setClass(this, BackupActivity.class);

        mTabHost.addTab(mTabHost
                .newTabSpec("Backup")
                .setIndicator(getResources().getString(R.string.backup_label),
                        getResources().getDrawable(R.drawable.ic_tab_memo))
                .setContent(intent));

    }

    private void setupRecoverTab()
    {
        Intent intent = new Intent("android.intent.action.RECOVER");
        intent.setClass(this, RestoreActivity.class);

        mTabHost.addTab(mTabHost.newTabSpec("Recover")
                .setIndicator(getResources().getString(R.string.recvoer_label),
                        getResources().getDrawable(R.drawable.ic_tab_appointment))
                .setContent(intent));
    }

    private void setCurrentTab(Intent intent)
    {
        Activity activity = getLocalActivityManager().
                getActivity(mTabHost.getCurrentTabTag());

        if (activity != null) {
            activity.closeOptionsMenu();
        }

        mTabHost.setCurrentTab(TAB_INDEX_BACKUP);
    }

    public void onTabChanged(String tabId) {

        Activity activity = getLocalActivityManager().getActivity(tabId);
        if (activity != null) {
            activity.onWindowFocusChanged(true);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.settings_options, menu);

        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {

        super.onPrepareOptionsMenu(menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        switch (item.getItemId()) {
            case R.id.menu_settings: {
                // settings on the action bar is pressed
                Intent intent = new Intent();
                intent.setClass(MainBackUpActivity.this, SettingsActivity.class);
                startActivity(intent);
            }

        }
        return super.onOptionsItemSelected(item);
    }

}
