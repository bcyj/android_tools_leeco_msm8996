/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.loadcarrier;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.Switch;

public class EditTriggerValuesActivity extends Activity implements OnCheckedChangeListener,
        OnClickListener {

    private Switch mTriggerOnOff;
    private View mEditContainer;
    private EditText mEditPath;
    private EditText mEditBrand;
    private EditText mEditTarget;
    private Button mBtnStart;
    private Button mBtnExit;

    private boolean mOldTriggerEnabled;
    private String mOldTriggerPath;
    private String mOldTriggerBrand;
    private String mOldTriggerTarget;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.edit_values_activity);

        mTriggerOnOff = (Switch) findViewById(R.id.trigger_on_off);
        mTriggerOnOff.setOnCheckedChangeListener(this);

        mEditContainer = findViewById(R.id.edit_container);
        mEditPath = (EditText) findViewById(R.id.edit_path);
        mEditBrand = (EditText) findViewById(R.id.edit_brand);
        mEditTarget = (EditText) findViewById(R.id.edit_target);

        mBtnStart = (Button) findViewById(R.id.btn_start);
        mBtnStart.setOnClickListener(this);
        mBtnExit = (Button) findViewById(R.id.btn_exit);
        mBtnExit.setOnClickListener(this);

        initValues();
        updateViews(mOldTriggerEnabled);
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        updateViews(isChecked);
    }

    @Override
    public void onClick(View v) {

        switch (v.getId()) {
            case R.id.btn_start:
                // Commit the values first.
                commitValues();
                startService(new Intent(this, TriggerService.class));
                finish();
                break;
            case R.id.btn_exit:
                // Commit the values first.
                commitValues();
                finish();
                break;
        }
    }

    private void initValues() {
        boolean defTriggerEnabled = getResources().getBoolean(R.bool.trigger_enabled);
        mOldTriggerEnabled = Utils.getValue(Utils.PROP_KEY_TRIGGER, defTriggerEnabled);

        String defPath = getString(R.string.trigger_path);
        mOldTriggerPath = Utils.getValue(Utils.PROP_KEY_TRIGGER_PATH, defPath);
        mEditPath.setText(mOldTriggerPath);

        String defBrand = getString(R.string.trigger_brand);
        mOldTriggerBrand = Utils.getValue(Utils.PROP_KEY_TRIGGER_BRAND, defBrand);
        mEditBrand.setText(mOldTriggerBrand);

        String defTarget = getString(R.string.trigger_target);
        mOldTriggerTarget = Utils.getValue(Utils.PROP_KEY_TRIGGER_TARGET, defTarget);
        mEditTarget.setText(mOldTriggerTarget);
    }

    private void updateViews(boolean triggerEnabled) {
        // Update the trigger on/off view.
        mTriggerOnOff.setChecked(triggerEnabled);
        // Update the edit container view visibility.
        mEditContainer.setVisibility(triggerEnabled ? View.VISIBLE : View.GONE);
        // Update the button visibility.
        mBtnStart.setVisibility(triggerEnabled ? View.VISIBLE : View.GONE);
    }

    private void commitValues() {
        boolean newTriggerValue = mTriggerOnOff.isChecked();
        if (newTriggerValue != mOldTriggerEnabled) {
            Utils.setValue(Utils.PROP_KEY_TRIGGER, String.valueOf(newTriggerValue));
        }

        String newPath = mEditPath.getText().toString();
        if (newPath != null && !newPath.equals(mOldTriggerPath)) {
            Utils.setValue(Utils.PROP_KEY_TRIGGER_PATH, newPath);
        }

        String newBrand = mEditBrand.getText().toString();
        if (newBrand != null && !newBrand.equals(mOldTriggerBrand)) {
            Utils.setValue(Utils.PROP_KEY_TRIGGER_BRAND, newBrand);
        }

        String newTarget = mEditTarget.getText().toString();
        if (newTarget != null && !newTarget.equals(mOldTriggerTarget)) {
            Utils.setValue(Utils.PROP_KEY_TRIGGER_TARGET, newTarget);
        }
    }
}
