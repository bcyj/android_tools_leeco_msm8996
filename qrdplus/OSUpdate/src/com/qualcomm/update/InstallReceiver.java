/**
 * Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */

package com.qualcomm.update;

import java.io.File;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

public class InstallReceiver extends Activity implements OnCancelListener, OnClickListener {

    public static final String ACTION_REBOOT = "com.qualcomm.update.REBOOT";

    public static final String ACTION_REBOOT_DELTA = "com.qualcomm.update.REBOOT_DELTA";

    private static final int DIALOG_CONFIRM_REBOOT = 1;

    private static final String TAG = "InstallUpdateReceiver";

    private static final boolean DEBUG = true;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (ACTION_REBOOT.equals(getIntent().getAction())
                || ACTION_REBOOT_DELTA.equals(getIntent().getAction())) {
            showDialog(DIALOG_CONFIRM_REBOOT);
        } else {
            finish();
        }
    }

    private boolean isDelta() {
        return ACTION_REBOOT_DELTA.equals(getIntent().getAction());
    }

    private void rebootToInstall() {
        final String path = getIntent().getData().getSchemeSpecificPart();
        log("install file:" + path);
        Intent intent = new Intent("android.intent.action.MASTER_CLEAR");
        intent.putExtra("packagefile", new File(path));
        intent.putExtra("qrdupdate", true);
        sendBroadcast(intent);
    }

    private void installToInstallDelta() {
        final String path = getIntent().getData().getSchemeSpecificPart();
        log("delta install file:" + path);
        new AsyncTask<String, Object, Boolean>() {

            @Override
            protected void onPreExecute() {
                super.onPreExecute();
                Toast.makeText(InstallReceiver.this, R.string.toast_upgrade_reboot, Toast.LENGTH_LONG).show();
            }

            @Override
            protected void onPostExecute(Boolean result) {
                super.onPostExecute(result);
                if (!result) {
                    Toast.makeText(InstallReceiver.this, R.string.toast_upgrade_failed,
                            Toast.LENGTH_SHORT).show();
                }
            }

            @Override
            protected Boolean doInBackground(String... params) {
                return UpdateUtil.copyToDeltaFile(new File(path))
                        && UpdateUtil.writeDeltaCommand()
                        && UpdateUtil.rebootInstallDelta(InstallReceiver.this);
            }

        }.execute();
    }

    private void saveUpdate() {
        String path = getIntent().getData().getSchemeSpecificPart();
        log("save file:" + path);
        UpdateUtil.saveUpdate(this, path, isDelta());
    }

    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case DIALOG_CONFIRM_REBOOT: {
                return new AlertDialog.Builder(this).setOnCancelListener(this)
                        .setIcon(android.R.drawable.ic_dialog_alert)
                        .setTitle(getString(R.string.title_dialog_alert))
                        .setMessage(getString(R.string.msg_reboot))
                        .setNeutralButton(android.R.string.cancel, this)
                        .setPositiveButton(android.R.string.ok, this).create();
            }
        }
        return super.onCreateDialog(id);
    }

    public void onCancel(DialogInterface dialog) {
        finish();
    }

    public void onClick(DialogInterface dialog, int which) {
        if (ACTION_REBOOT.equals(getIntent().getAction())
                && which == DialogInterface.BUTTON_POSITIVE) {
            rebootToInstall();
        } else if (ACTION_REBOOT_DELTA.equals(getIntent().getAction())
                && which == DialogInterface.BUTTON_POSITIVE) {
            installToInstallDelta();
        } else if (which == DialogInterface.BUTTON_NEUTRAL) {
            saveUpdate();
        }
        finish();
    }

    private void log(String msg) {
        if (DEBUG) {
            Log.e(TAG, msg);
        }
    }

}
