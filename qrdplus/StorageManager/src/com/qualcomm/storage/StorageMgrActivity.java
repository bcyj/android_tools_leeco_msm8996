/**
 * Copyright (c) 2011 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.qualcomm.storage;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.SystemProperties;

public class StorageMgrActivity extends Activity implements DialogInterface.OnCancelListener,
        DialogInterface.OnClickListener {

    private static final String KEY_STORAGE_PROP = "persist.sys.emmcsdcard.enabled";

    private static final int DIALOG_ALERT_CHANGE = 0;

    private static final int DIALOG_ALERT_REBOOT = 1;

    private static final int DIALOG_SHOW_CHOOSE = 3;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    protected void onResume() {
        super.onResume();
        showDialog(DIALOG_SHOW_CHOOSE);
    }

    private void doChange(String newValue) {
        if (!newValue.equals(SystemProperties.get(KEY_STORAGE_PROP))) {
            showDialog(DIALOG_ALERT_CHANGE);
        } else {
            finish();
        }
    }

    private class ChangeClickListener implements DialogInterface.OnClickListener {
        public void onClick(DialogInterface dialog, int which) {
            String storageValue = SystemProperties.get(KEY_STORAGE_PROP);
            SystemProperties.set(KEY_STORAGE_PROP, "1".equals(storageValue) ? "0" : "1");
            //showDialog(DIALOG_ALERT_REBOOT);
            reboot();
        }
    }

    private void reboot(){
        PowerManager pm = (PowerManager)this.getSystemService(this.POWER_SERVICE);
        pm.reboot(null);
    }

    private class RebootClickListener implements DialogInterface.OnClickListener {
        public void onClick(DialogInterface dialog, int which) {
            Intent reboot = new Intent(Intent.ACTION_REBOOT);
            reboot.putExtra("nowait", 1);
            reboot.putExtra("interval", 1);
            reboot.putExtra("window", 0);
            StorageMgrActivity.this.sendBroadcast(reboot);
        }
    }

    private class ChooseListener implements DialogInterface.OnClickListener {
        public void onClick(DialogInterface dialog, int which) {
            dialog.dismiss();
            doChange(String.valueOf(which));
        }
    }

    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case DIALOG_ALERT_CHANGE:
                return new AlertDialog.Builder(this).setMessage(R.string.storage_change_msg)
                        .setTitle(R.string.alert_title).setIcon(android.R.drawable.ic_dialog_alert)
                        .setOnCancelListener(this).setNegativeButton(android.R.string.cancel, this)
                        .setPositiveButton(android.R.string.ok, new ChangeClickListener()).create();
            case DIALOG_ALERT_REBOOT:
                return new AlertDialog.Builder(this).setMessage(R.string.reboot_msg)
                        .setTitle(R.string.alert_title).setIcon(android.R.drawable.ic_dialog_alert)
                        .setOnCancelListener(this).setNegativeButton(android.R.string.cancel, this)
                        .setPositiveButton(android.R.string.ok, new RebootClickListener()).create();
            case DIALOG_SHOW_CHOOSE:
                return new AlertDialog.Builder(this)
                        .setTitle(R.string.storage_option_title)
                        .setOnCancelListener(this)
                        .setNegativeButton(android.R.string.cancel, this)
                        .setSingleChoiceItems(R.array.storages_entries,
                                Integer.parseInt(SystemProperties.get(KEY_STORAGE_PROP,"0")),
                                new ChooseListener()).create();
        }
        return null;
    }

    public void onCancel(DialogInterface dialog) {
        finish();
    }

    public void onClick(DialogInterface dialog, int which) {
        finish();
    }
}
