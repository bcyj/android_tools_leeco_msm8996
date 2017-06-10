/******************************************************************************
 * @file    ConfigurationActivity.java
 * ---------------------------------------------------------------------------
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 ******************************************************************************/
package com.qti.antitheftdemo;

import android.app.Activity;
import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.widget.Toast;
import android.widget.ToggleButton;

public class ConfigurationActivity extends Activity implements OnCheckedChangeListener, OnEditorActionListener {

    private static DevicePolicyManager devicePolicyManager;
    private static ComponentName deviceAdmin;
    private ToggleButton toggleButton;

    static final int ACTIVATION_REQUEST = 47;

    @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            setContentView(R.layout.activity_configuration);

            configureEditTextView();

            devicePolicyManager = (DevicePolicyManager) getSystemService(Context.DEVICE_POLICY_SERVICE);
            deviceAdmin = new ComponentName(this, DeviceManagerAdminReceiver.class);
            toggleButton = (ToggleButton) super
                .findViewById(R.id.toggle_device_admin);
            toggleButton.setOnCheckedChangeListener(this);
            setToggleButton();

            Switch permissionSwitch = (Switch) findViewById(R.id.switch_permission);
            switchConfig(permissionSwitch);

        }

    @Override
        public void onCheckedChanged(CompoundButton button, boolean isChecked) {
            if (button.getId() == R.id.toggle_device_admin) {
                if (isChecked) {
                    // Activate device administration
                    activateDeviceManager();
                } else {
                    if(isDeviceAdminActive()) {
                        devicePolicyManager.removeActiveAdmin(deviceAdmin);
                    }
                }
                if(Debug.DEBUG) Log.d(Debug.TAG, "onCheckedChanged to: " + isChecked);
            } else if (button.getId() == R.id.switch_permission) {
                if(Debug.DEBUG) Log.d(Debug.TAG, "Switch onCheckedChanged to: " + isChecked);
                if (!isChecked) {
                    SettingsEditor.setPermission(this, false);
                    startMainActivity();
                }
            }
        }

    @Override
        public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
            if (actionId == EditorInfo.IME_ACTION_DONE) {
                hideKeyboard((EditText) v);
                String value = v.getText().toString();
                switch(v.getId()) {
                    case R.id.confirmationCode:
                        if (Debug.DEBUG) Log.d(Debug.TAG, "Confirmation code set button clicked.");
                        if(checkPasswordStrength(value) == false) {
                            Toast.makeText(this, R.string.text_wrong_confirmationCode, Toast.LENGTH_SHORT).show();
                            value = SettingsEditor.getConfirmationCode(this);
                            v.setText(value);
                        } else {
                            SettingsEditor.setConfirmationCode(this, value);
                        }
                        break;
                    case R.id.friendNumber:
                        if (Debug.DEBUG) Log.d(Debug.TAG, "Friend number set button clicked.");
                        SettingsEditor.setFriendNumber(this, value);
                        TelephonyManager telephonyMgr = (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
                        String newSubscriberId = telephonyMgr.getSubscriberId();
                        SettingsEditor.setSubscriberId(this, newSubscriberId);
                        break;
                    default:
                        Log.e(Debug.TAG, "Unknow button clicked. Ignoring!");
                }

                return true;
            }
            return false;
        }

    @Override
        protected void onResume() {
            super.onResume();
            setToggleButton();
        }

    private void hideKeyboard(EditText editText) {
        InputMethodManager imm= (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
    }

    private void configureEditTextView() {
        EditText confrimationCodeEditText = (EditText) this.findViewById(R.id.confirmationCode);
        String confirmationCode = SettingsEditor.getConfirmationCode(this);
        confrimationCodeEditText.setOnEditorActionListener(this);
        if(confirmationCode != null) {
            confrimationCodeEditText.setText(confirmationCode);
        }

        EditText friendNumberEditText = (EditText) this.findViewById(R.id.friendNumber);
        String friendNumber = SettingsEditor.getFriendNumber(this);
        friendNumberEditText.setOnEditorActionListener(this);
        if(friendNumber != null) {
            friendNumberEditText.setText(friendNumber);
        }
    }

    private void activateDeviceManager() {
        Intent intent = new Intent(
                DevicePolicyManager.ACTION_ADD_DEVICE_ADMIN);
        intent.putExtra(DevicePolicyManager.EXTRA_DEVICE_ADMIN,
                deviceAdmin);
        intent.putExtra(DevicePolicyManager.EXTRA_ADD_EXPLANATION,
                "Enable Anti Theft Demo.");
        startActivityForResult(intent, ACTIVATION_REQUEST);
    }

    private boolean isDeviceAdminActive() {
        if(devicePolicyManager != null && devicePolicyManager.isAdminActive(deviceAdmin)) {
            return true;
        }
        return false;
    }

    private void setToggleButton() {
        toggleButton.setChecked(isDeviceAdminActive());
    }

    private boolean checkPasswordStrength(String password) {
        int length = password.length();
        if (length >= 6 && length <= 12) {
            return true;
        }
        return false;
    }

    private void startMainActivity() {
        Intent i = new Intent(this, MainActivity.class);
        startActivity(i);
        finish();
    }

    private void switchConfig(Switch s) {
        if (s != null) {
            s.setChecked(SettingsEditor.getPermission(this));
            s.setOnCheckedChangeListener(this);
        }
    }

}
