/*
 * Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.update;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ActivityNotFoundException;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.os.Bundle;
import android.os.RecoverySystem;
import java.io.File;
import java.io.IOException;
import java.security.GeneralSecurityException;
import android.widget.Toast;
import android.app.ActivityManager;

public class UpdateDialog extends Activity implements OnClickListener, OnCancelListener {

    private static final int DIALOG_CHOOSE = 1;

    private static final int INDEX_REMOTE = 0;

    private static final int INDEX_LOCAL = 1;

    private static final int REQUEST_CODE_FILE = 1;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(ActivityManager.isUserAMonkey())
            finish();
        else
            this.showDialog(DIALOG_CHOOSE);
    }

    private class ChooseFromListener implements OnClickListener {

        public void onClick(DialogInterface dialog, int which) {
            Intent intent = null;
            switch (which) {
                case INDEX_REMOTE:
                    intent = new Intent(UpdateDialog.this, RemoteActivity.class);
                    startActivity(intent);
                    finish();
                    break;
                case INDEX_LOCAL:
                    try {
                        intent = new Intent(Intent.ACTION_GET_CONTENT);
                        intent.setType("*/*");
                        startActivityForResult(intent, REQUEST_CODE_FILE);
                        Toast.makeText(UpdateDialog.this, R.string.msg_pick_update,
                                Toast.LENGTH_SHORT).show();
                    } catch (ActivityNotFoundException e) {
                        Toast.makeText(UpdateDialog.this, R.string.msg_no_file_explore,
                                Toast.LENGTH_SHORT).show();
                    }
                    break;
            }
        }

    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == RESULT_OK) {
            switch (requestCode) {
                case REQUEST_CODE_FILE:
                     Intent intent = new Intent(InstallReceiver.ACTION_REBOOT);
                     final String path = data.getData().getPath();
                     try {
                         File file = new File(path);
                         RecoverySystem.verifyPackage(file, null, null);
                         intent.setData(data.getData());
                         startActivity(intent);
                         finish();
                     } catch (IOException e) {
                         Toast.makeText(UpdateDialog.this, R.string.msg_not_right_file_type,
                                 Toast.LENGTH_SHORT).show();
                     } catch (GeneralSecurityException e) {
                         Toast.makeText(UpdateDialog.this, R.string.msg_not_right_file_type,
                                 Toast.LENGTH_SHORT).show();
                     }
                     break;
            }
        }
    }

    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case DIALOG_CHOOSE:
                return new AlertDialog.Builder(this)
                        .setSingleChoiceItems(
                                new MenuAdapter(this, getResources().getStringArray(
                                        R.array.remote_entries), R.drawable.remote_update,
                                        R.drawable.local_update), 0, new ChooseFromListener())
                        .setTitle(R.string.title_choose_update_from).setOnCancelListener(this)
                        .setNegativeButton(android.R.string.cancel, this).create();
        }
        return null;
    }

    public void onClick(DialogInterface dialog, int which) {
        finish();
    }

    public void onCancel(DialogInterface dialog) {
        finish();
    }
}
