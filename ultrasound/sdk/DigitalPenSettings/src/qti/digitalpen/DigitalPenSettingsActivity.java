/******************************************************************************
 * @file    DigitalPenSettingsActivity.java
 * @brief   Digital Pen Settings
 *
 * ---------------------------------------------------------------------------
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 ******************************************************************************/

package qti.digitalpen;

import com.qti.snapdragon.digitalpen.DigitalPenGlobalControl;
import com.qti.snapdragon.digitalpen.DigitalPenGlobalControl.DigitalPenEventListener;
import com.qti.snapdragon.digitalpen.DigitalPenGlobalControl.DigitalPenGlobalSettings;
import com.qti.snapdragon.digitalpen.DigitalPenGlobalControl.DigitalPenGlobalSettings.EraseMode;
import com.qti.snapdragon.digitalpen.DigitalPenGlobalControl.DigitalPenGlobalSettings.OffScreenMode;
import com.qti.snapdragon.digitalpen.DigitalPenGlobalControl.DigitalPenGlobalSettings.PowerProfile;
import com.qti.snapdragon.digitalpen.DigitalPenGlobalControl.DigitalPenGlobalSettings.Side;
import com.qti.snapdragon.digitalpen.util.DigitalPenEvent;

import android.app.Activity;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.CompoundButton;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.widget.Toast;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnFocusChangeListener;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.util.Log;
import android.os.RemoteException;
import android.os.Bundle;

public class DigitalPenSettingsActivity extends Activity implements OnCheckedChangeListener {
    private static final String TAG = "DigitalPenSettingsActivity";

    private boolean isDigitalPenEnabled;
    private DigitalPenGlobalControl penControl;

    private void handleDigitalPenEvent(final DigitalPenEvent event) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                switch (event.getEventType()) {
                    case DigitalPenEvent.TYPE_POWER_STATE_CHANGED:
                        handlePowerStateChange(event);
                        break;
                    case DigitalPenEvent.TYPE_VERSION:
                        handleVersion(event);
                        break;
                    default:
                        break;
                }
            }
        });
    }

    private void handleVersion(DigitalPenEvent event) {
        int frameworkMajorVersion = event.getParameterValue(DigitalPenEvent.PARAM_VERSION_MAJOR);
        int frameworkMinorVersion = event.getParameterValue(DigitalPenEvent.PARAM_VERSION_MINOR);
        String version = "Service v" + frameworkMajorVersion + "." + frameworkMinorVersion;
        versionTextView.setText(version);
    }

    private void handlePowerStateChange(DigitalPenEvent event) {
        int powerState = event.getParameterValue(DigitalPenEvent.PARAM_CURRENT_POWER_STATE);
        if (needToHandlePowerStateEvent(powerState)) {
            boolean newEnableValue = powerState == DigitalPenEvent.POWER_STATE_ACTIVE;
            isDigitalPenEnabled = newEnableValue;
            updateUi();
        }
    }

    private boolean needToHandlePowerStateEvent(int powerState) {
        // other power states, such as idle, are informational
        return powerState == DigitalPenEvent.POWER_STATE_ACTIVE
                || powerState == DigitalPenEvent.POWER_STATE_OFF;
    }

    private EditText inRangeDistance;

    private DigitalPenGlobalSettings config;

    private EditText onScreenHoverMaxDistance;

    private EditText offScreenHoverMaxDistance;

    private EditText eraseButtonIndex;

    private CompoundButton eraseButtonEnabled;

    private boolean initializingWidgetValues;

    private TextView versionTextView;

    public void updateUi() {
        disableEnableControls(isDigitalPenEnabled, (ViewGroup) findViewById(R.id.topLayout));
        setCheckbox(R.id.toggleButtonEnable, isDigitalPenEnabled);
        findViewById(R.id.toggleButtonEnable).setEnabled(true);
    }

    private void disableEnableControls(boolean enable, ViewGroup vg) {
        for (int i = 0; i < vg.getChildCount(); i++) {
            View child = vg.getChildAt(i);
            child.setEnabled(enable);
            if (child instanceof ViewGroup) {
                disableEnableControls(enable, (ViewGroup) child);
            }
        }
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        versionTextView = (TextView) findViewById(R.id.textViewVersion);

        // initialize edit text callbacks
        inRangeDistance = (EditText) findViewById(R.id.editTextInRangeDistance);
        setEditDoneListeners(inRangeDistance);
        onScreenHoverMaxDistance = (EditText) findViewById(R.id.editTextOnScreenHoverMaxDistance);
        setEditDoneListeners(onScreenHoverMaxDistance);
        offScreenHoverMaxDistance = (EditText) findViewById(R.id.editTextOffScreenHoverMaxDistance);
        setEditDoneListeners(offScreenHoverMaxDistance);
        eraseButtonIndex = (EditText) findViewById(R.id.editTextEraserIndex);
        setEditDoneListeners(eraseButtonIndex);
        eraseButtonEnabled = (CompoundButton) findViewById(R.id.switchEraserEnable);

        // initialize switch callbacks; onClick doesn't capture finger slide
        final int[] switchIds = {
                R.id.switchSmarterStand,
                R.id.switchOnScreenHover,
                R.id.switchOffScreenHover,
                R.id.switchOnScreenHoverIcon,
                R.id.switchOffScreenHoverIcon,
                R.id.switchEraserEnable,
                R.id.switchStopPenOnScreenOff,
                R.id.switchStartPenOnBoot
        };
        for (int switchId : switchIds) {
            ((CompoundButton) findViewById(switchId)).setOnCheckedChangeListener(this);
        }

        penControl = new DigitalPenGlobalControl(this);
        try {
            penControl.registerEventListener(new DigitalPenEventListener() {

                @Override
                public void onDigitalPenPropEvent(DigitalPenEvent event) {
                    handleDigitalPenEvent(event);
                }
            });
            isDigitalPenEnabled = penControl.isPenFeatureEnabled();
        } catch (RemoteException e) {
            e.printStackTrace();
            Toast.makeText(this, "Problem intializing pen service", Toast.LENGTH_LONG).show();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        try {
            config = penControl.getCurrentSettings();
        } catch (RemoteException e) {
            Toast.makeText(this, "Failed to get config", Toast.LENGTH_SHORT).show();
            e.printStackTrace();
        }

        if (null == config) {
            Toast.makeText(this, "Failed to get config", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }

        initializeWidgetValues();
    }

    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        initializingWidgetValues = true;
        super.onRestoreInstanceState(savedInstanceState);
        initializingWidgetValues = false;
    };

    private void initializeWidgetValues() {
        initializingWidgetValues = true;
        isDigitalPenEnabled = false;
        if (penControl != null) {
            try {
                isDigitalPenEnabled = penControl.isPenFeatureEnabled();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
        setCheckbox(R.id.toggleButtonEnable, isDigitalPenEnabled);

        // set values read from config object
        if (config.getOffScreenMode() == OffScreenMode.DISABLED) {
            setCheckbox(R.id.radioOffScreenDisabled, true);
        } else {
            setCheckbox(R.id.radioOffScreenDuplicate, true);
        }
        setCheckbox(R.id.switchSmarterStand, config.isSmarterStandEnabled());
        inRangeDistance.setText(Integer.toString(config.getInRangeDistance()));

        setCheckbox(R.id.switchStopPenOnScreenOff, config.isStopPenOnScreenOffEnabled());

        setCheckbox(R.id.switchStartPenOnBoot, config.isStartPenOnBootEnabled());

        setCheckbox(R.id.switchOnScreenHover, config.isOnScreenHoverEnabled());
        onScreenHoverMaxDistance.setText(Integer.toString(config.getOnScreenHoverMaxRange()));
        setCheckbox(R.id.switchOnScreenHoverIcon, config.isShowingOnScreenHoverIcon());

        setCheckbox(R.id.switchOffScreenHover, config.isOffScreenHoverEnabled());
        offScreenHoverMaxDistance.setText(Integer.toString(config.getOffScreenHoverMaxRange()));
        setCheckbox(R.id.switchOffScreenHoverIcon, config.isShowingOffScreenHoverIcon());

        if (config.getOffScreenPortraitSide() == Side.LEFT) {
            setCheckbox(R.id.radioOffScreenLocationLeft, true);
        } else {
            setCheckbox(R.id.radioOffScreenLocationRight, true);
        }

        int buttonIndex = config.getEraseButtonIndex();
        setCheckbox(R.id.switchEraserEnable, buttonIndex != -1);
        eraseButtonIndex.setText(buttonIndex == -1 ? "0" : Integer
                .toString(buttonIndex));
        if (config.getEraseButtonMode() == EraseMode.HOLD) {
            setCheckbox(R.id.radioEraserBehaviorHold, true);
        } else {
            setCheckbox(R.id.radioEraserBehaviorToggle, true);
        }

        if (config.getPowerProfile() == PowerProfile.OPTIMIZE_ACCURACY) {
            setCheckbox(R.id.radioPowerModeAccuracy, true);
        } else {
            setCheckbox(R.id.radioPowerModePower, true);
        }
        updateUi();
        initializingWidgetValues = false;
    }

    private void setCheckbox(int id, boolean set) {
        ((CompoundButton) findViewById(id)).setChecked(set);
    }

    private void setEditDoneListeners(EditText editText) {
        editText.setOnEditorActionListener(new OnEditorActionListener() {

            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    return integerTextFieldDone(v);
                }
                return false;
            }
        });
        editText.setOnFocusChangeListener(new OnFocusChangeListener() {

            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                if (!hasFocus) {
                    integerTextFieldDone((TextView) v);
                }
            }
        });
    }

    private boolean integerTextFieldDone(TextView v) {
        try {
            final int value = Integer.parseInt(v.getText().toString());
            switch (v.getId()) {
                case R.id.editTextInRangeDistance:
                    config.setInRangeDistance(value);
                    break;
                case R.id.editTextOnScreenHoverMaxDistance:
                    config.setOnScreenHoverMaxRange(value);
                    break;
                case R.id.editTextOffScreenHoverMaxDistance:
                    config.setOffScreenHoverMaxRange(value);
                    break;
                case R.id.editTextEraserIndex:
                    if (eraseButtonEnabled.isChecked()) {
                        EraseMode currentMode = config.getEraseButtonMode();
                        config.enableErase(value, currentMode);
                    } else {
                        config.disableErase();
                    }
                    break;
                default:
                    throw new RuntimeException("Unknown text view in editTextDone: " + v);
            }
            setGlobalConfig();
        } catch (NumberFormatException x) {
            x.printStackTrace();
            Toast.makeText(this, "Illegal value '" + v.getText() + "', expected numeric string",
                    Toast.LENGTH_LONG).show();
        }
        return true;
    }

    public void onClickEnableButton(View v) {
        try {
            v.setEnabled(false);
            if (isDigitalPenEnabled) {
                penControl.disablePenFeature();
            } else {
                penControl.enablePenFeature();
            }
        } catch (RemoteException e) {
            Log.e(TAG, "Remote service error: " + e.getMessage());
        } catch (RuntimeException e) {
            Toast.makeText(this, "Cannot start digital pen service. Have you paired a pen?",
                    Toast.LENGTH_LONG).show();
            ((CompoundButton) v).setChecked(false);
        }
    }

    private void setGlobalConfig() {
        try {
            penControl.commitSettings(config);
        } catch (RemoteException e) {
            Log.e(TAG, "Remote exception trying to set global config");
            e.printStackTrace();
        }
    }

    public void onClickRadioOffScreenMode(View v) {
        switch (v.getId()) {
            case R.id.radioOffScreenDisabled:
                config.setDefaultOffScreenMode(OffScreenMode.DISABLED);
                break;
            case R.id.radioOffScreenDuplicate:
                config.setDefaultOffScreenMode(OffScreenMode.DUPLICATE);
                break;
            default:
                throw new RuntimeException("Unknown ID:" + v.getId());
        }
        setGlobalConfig();
        updateUi();
    }

    public void onClickRadioPowerMode(View v) {
        switch (v.getId()) {
            case R.id.radioPowerModeAccuracy:
                config.setPowerProfile(PowerProfile.OPTIMIZE_ACCURACY);
                break;
            case R.id.radioPowerModePower:
                config.setPowerProfile(PowerProfile.OPTIMIZE_POWER);
                break;
            default:
                throw new RuntimeException("Unknown ID:" + v.getId());
        }
        setGlobalConfig();
        updateUi();
    }

    public void onClickRadioOffScreenLocation(View v) {
        switch (v.getId()) {
            case R.id.radioOffScreenLocationLeft:
                config.setOffScreenPortraitSide(Side.LEFT);
                break;
            case R.id.radioOffScreenLocationRight:
                config.setOffScreenPortraitSide(Side.RIGHT);
                break;
            default:
                throw new RuntimeException("Unknown ID:" + v.getId());
        }
        setGlobalConfig();
        updateUi();
    }

    public void onClickRadioEraserBehavior(View v) {
        int currentIndex = config.getEraseButtonIndex();
        switch (v.getId()) {
            case R.id.radioEraserBehaviorHold:
                config.enableErase(currentIndex, EraseMode.HOLD);
                break;
            case R.id.radioEraserBehaviorToggle:
                config.enableErase(currentIndex, EraseMode.TOGGLE);
                break;
            default:
                throw new RuntimeException("Unknown ID:" + v.getId());
        }
        setGlobalConfig();
        updateUi();
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (initializingWidgetValues) {
            return; // button set due to resume or create, not user action
        }
        switch (buttonView.getId()) {

            case R.id.switchEraserEnable:
                // the edit text done handler checks this button state and
                // handles config change.
                integerTextFieldDone(eraseButtonIndex);
                return; // all other buttons fall-through to loadConfig
            case R.id.switchSmarterStand:
                config.enableSmarterStand(isChecked);
                break;
            case R.id.switchStopPenOnScreenOff:
                config.enableStopPenOnScreenOff(isChecked);
                break;
            case R.id.switchStartPenOnBoot:
                config.enableStartPenOnBoot(isChecked);
                break;
            case R.id.switchOnScreenHover:
                config.enableOnScreenHover(isChecked);
                break;
            case R.id.switchOffScreenHover:
                config.enableOffScreenHover(isChecked);
                break;
            case R.id.switchOnScreenHoverIcon:
                config.showOnScreenHoverIcon(isChecked);
                break;
            case R.id.switchOffScreenHoverIcon:
                config.showOffScreenHoverIcon(isChecked);
                break;
            default:
                throw new RuntimeException("Unknown switch, id: " + buttonView.getId());
        }
        setGlobalConfig();
        updateUi();
    }

    public void onClickEraserEnable(View v) {
        // the edit text done handler checks this button state
        integerTextFieldDone(eraseButtonIndex);
    }

}
