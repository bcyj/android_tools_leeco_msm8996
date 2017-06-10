/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.authaccess;

import static com.qapp.secprotect.utils.UtilsLog.logd;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ImageView;
import android.widget.TextView;

import com.qapp.secprotect.R;
import com.qapp.secprotect.data.AuthDBManager;
import com.qapp.secprotect.data.AuthInfo;
import com.qapp.secprotect.utils.UtilsStrings;
import com.qapp.secprotect.utils.UtilsSystem;

public class PromptActivity extends Activity {

    private static final int DIALOG_USER_CONFIRM = 0;
    CountdownRunnable mCountdownRunnable;
    Context mContext;
    private final int TIMEOUT = 10 * 1000;
    private AuthInfo mAuthInfo;

    boolean parseIntent() {
        Intent intent = getIntent();
        if (intent == null)
            return false;
        int uid = intent.getIntExtra("uid", -1);
        int gid = intent.getIntExtra("gid", -1);
        int pid = intent.getIntExtra("pid", -1);
        String packageName = UtilsSystem.getPackageNameByPid(
                getApplicationContext(), uid);
        String path = intent.getStringExtra("path");

        mAuthInfo = new AuthInfo();
        mAuthInfo.uid = uid;
        mAuthInfo.gid = gid;
        mAuthInfo.pid = pid;
        mAuthInfo.packageName = packageName;
        mAuthInfo.lastPath = path;

        logd(mAuthInfo);
        return true;
    }

    private void init() {
        mContext = getApplicationContext();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        init();
        if (!parseIntent()) {
            finish();
            return;
        }
        AlertDialog confirmDialog = (AlertDialog) createDialog(
                DIALOG_USER_CONFIRM, null);
        confirmDialog.show();
        mCountdownRunnable = new CountdownRunnable(confirmDialog, TIMEOUT);
        mHandler.post(mCountdownRunnable);
    }

    class CountdownRunnable implements Runnable {
        TextView timerTextView;
        int countdownTime;
        int counter;

        public CountdownRunnable(AlertDialog alertDialog, int countdownTime) {
            timerTextView = (TextView) alertDialog
                    .findViewById(R.id.countdown_timer);
            this.countdownTime = countdownTime;
            counter = 0;
        }

        @Override
        public void run() {
            // logd(counter);
            timerTextView.setText("" + (countdownTime - counter) / 1000);
            if (counter < countdownTime) {
                mHandler.postDelayed(mCountdownRunnable, 1000);
            } else {
                allow(false);
            }
            counter += 1000;
        }

    };

    @Override
    public void finish() {
        logd("");
        super.finish();
    }

    protected Dialog createDialog(int id, Bundle args) {

        switch (id) {
        case DIALOG_USER_CONFIRM:
            final View view = LayoutInflater.from(this).inflate(
                    R.layout.prompt_user_confirm, null);
            TextView appnameText = (TextView) view.findViewById(R.id.app_name);
            ImageView appIcon = (ImageView) view.findViewById(R.id.app_icon);
            TextView pathText = (TextView) view
                    .findViewById(R.id.authaccess_path);

            appnameText.setText(UtilsSystem.getAppName(mContext, mAuthInfo.uid,
                    mAuthInfo.packageName));
            appIcon.setImageDrawable(UtilsSystem.getAppIcon(mContext,
                    mAuthInfo.uid, mAuthInfo.packageName));
            pathText.setText(UtilsStrings
                    .getProtectedFilePath(mAuthInfo.lastPath));

            CheckBox rememberCheckbox = (CheckBox) view
                    .findViewById(R.id.remember_checkbox);
            rememberCheckbox
                    .setOnCheckedChangeListener(new OnCheckedChangeListener() {

                        @Override
                        public void onCheckedChanged(CompoundButton arg0,
                                boolean checked) {
                            logd(checked);
                            if (checked) {
                                mAuthInfo.remember = 1;
                            }
                        }
                    });
            AlertDialog confirmDialog = new AlertDialog.Builder(this)
                    .setTitle(R.string.authaccess)
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .setView(view)
                    .setCancelable(false)
                    .setOnCancelListener(new OnCancelListener() {

                        @Override
                        public void onCancel(DialogInterface dialog) {
                            finish();
                        }
                    })
                    .setPositiveButton(R.string.grant,
                            new DialogInterface.OnClickListener() {

                                public void onClick(DialogInterface dialog,
                                        int whichButton) {
                                    allow(true);
                                }
                            })
                    .setNegativeButton(R.string.deny,
                            new DialogInterface.OnClickListener() {

                                public void onClick(DialogInterface dialog,
                                        int whichButton) {
                                    allow(false);
                                }
                            }).create();
            return confirmDialog;

        default:
            break;

        }
        return null;
    }

    Handler mHandler = new Handler() {
        public void handleMessage(android.os.Message msg) {

        };
    };

    private void allow(boolean isAllowed) {

        mHandler.removeCallbacks(mCountdownRunnable);
        AuthDBManager mDbManager = new AuthDBManager(mContext);
        mDbManager.openDataBase();
        Cursor cursor = mDbManager.query(mAuthInfo.uid);
        if (cursor != null) {
            mAuthInfo.time = System.currentTimeMillis();
            mAuthInfo.mode = isAllowed ? 1 : -1;
            if (cursor.getCount() == 0) {
                mDbManager.insert(mAuthInfo);
            } else {
                mDbManager.update(mAuthInfo);
            }
            cursor.close();
        }
        mDbManager.closeDataBase();
        this.finish();
    }

    @Override
    protected void onResume() {
        logd("");
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("");
        registerReceiver(mBroadcastReceiver, intentFilter);
        super.onResume();
    }

    @Override
    protected void onPause() {
        logd();
        unregisterReceiver(mBroadcastReceiver);
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        logd();
        super.onDestroy();
    };

    BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logd(action);
        };
    };
}
