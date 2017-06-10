/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.encryptdata;

import static com.qapp.secprotect.utils.UtilsLog.logd;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.storage.StorageManager;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.qapp.secprotect.Configs;
import com.qapp.secprotect.R;
import com.qapp.secprotect.utils.UtilsLog;
public class PasswordActivity extends Activity implements
        android.view.View.OnClickListener {
    private Context mContext;
    private EditText setPasswordEditText, confirmPasswordEditText;
    private Button okButton, cancelButton;

    private void init() {
        mContext = getApplicationContext();
        getActionBar().setDisplayShowHomeEnabled(false);
        Intent intent = getIntent();
        if (intent.hasExtra("mode")) {
            String mode = intent.getStringExtra("mode");
            if (Configs.INTENT_CREATE_PASSWORD.equals(mode)) {
                setContentView(R.layout.create_password_layout);
                getActionBar().setTitle(getString(R.string.create_password));

                setPasswordEditText = (EditText) findViewById(R.id.set_password);
                confirmPasswordEditText = (EditText) findViewById(R.id.confirm_password);
                okButton = (Button) findViewById(R.id.create_password_ok);
                cancelButton = (Button) findViewById(R.id.create_password_cancel);

                okButton.setOnClickListener(this);
                cancelButton.setOnClickListener(this);
            }
        } else {
            finish();
        }

    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        init();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        logd("");
        super.onDestroy();
    }

    @Override
    public void onClick(View view) {
        logd("");
        StorageManager storageManager = (StorageManager) getSystemService(Context.STORAGE_SERVICE);
        String[] s = storageManager.getVolumePaths();
        logd(s);
        if (view.getId() == R.id.create_password_ok) {
            String setString = setPasswordEditText.getText().toString();
            String confirmString = confirmPasswordEditText.getText().toString();
            if (setString.length() == 0) {
                UtilsLog.toast(mContext,
                        getString(R.string.empty_password_hint));
                return;
            }
            if (confirmString.length() == 0) {
                UtilsLog.toast(mContext,
                        getString(R.string.empty_confirm_password_hint));
                return;
            }
            if (!setString.equals(confirmString)) {
                UtilsLog.toast(mContext,
                        getString(R.string.inconsistent_password_hint));
                return;
            }

            UtilsPassword.writePasswordToFile(setString,
                    UtilsPassword.INTERNAL_INDEX);
            UtilsPassword.writePasswordToFile(setString,
                    UtilsPassword.SDCARD_INDEX);
            finish();

        } else if (view.getId() == R.id.create_password_cancel) {
            finish();
        }
    }
}
